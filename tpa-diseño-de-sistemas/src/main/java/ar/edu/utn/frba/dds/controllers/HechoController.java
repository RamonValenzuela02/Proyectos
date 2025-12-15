package ar.edu.utn.frba.dds.controllers;

import ar.edu.utn.frba.dds.model.Hecho.Hecho;
import ar.edu.utn.frba.dds.model.Multimedia.ArchivoMultimedia;
import ar.edu.utn.frba.dds.model.criterio.Categoria;
import ar.edu.utn.frba.dds.model.exportarCSV.ImportarCSV;
import ar.edu.utn.frba.dds.model.fuente.Fuente;
import ar.edu.utn.frba.dds.repositorios.RepoArchivosMultimedia;
import ar.edu.utn.frba.dds.repositorios.RepoFuentesDelSistema;
import ar.edu.utn.frba.dds.repositorios.RepoHechosDinamicos;
import ar.edu.utn.frba.dds.repositorios.RepoUsuarios;
import io.javalin.http.Context;
import io.javalin.http.UploadedFile;

import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.time.LocalDateTime;
import java.util.*;
import org.jetbrains.annotations.NotNull;

public class HechoController {

    public void formHechoNuevo(Context ctx){
        Long user_id = ctx.sessionAttribute("user_id");

        Map<String, Object> model = new HashMap<>();
        model.put("categorias", Categoria.values());
        model.put("values", Collections.emptyMap());
        model.put("errors", Collections.emptyList());
        model.put("usuarioLogueado", user_id != null);

        ctx.render("hecho/hecho.nuevo.hbs", model);
    }

    public void crearHecho(Context ctx){
        try {

            List<String> errors = new ArrayList<>();

            String titulo = ctx.formParam("titulo");
            String descripcion = ctx.formParam("descripcion");
            String categoriaStr = ctx.formParam("categoria");
            String latitud = ctx.formParam("latitud");
            String longitud = ctx.formParam("longitud");
            String fechaStr = ctx.formParam("fecha");
            List<UploadedFile> archivos = ctx.uploadedFiles("multimedia[]");

            if (titulo.isBlank()) {
                errors.add("El título es obligatorio.");
            }
            if (descripcion.isBlank()) {
                errors.add("La descripción es obligatoria.");
            }
            if (categoriaStr.isBlank()){
                errors.add("La categoría es obligatoria.");
            }
            if (fechaStr.isBlank()) {
                errors.add("La fecha del hecho es obligatoria.");
            }

            Categoria categoria = null;
            if(!categoriaStr.isBlank()){
                try {
                    categoria = Categoria.valueOf(categoriaStr.toUpperCase());
                } catch (IllegalArgumentException e) {
                    errors.add("Categoría inválida: " + categoriaStr);
                }
            }

            LocalDateTime fechaHecho = null;
            try {
                fechaHecho = LocalDateTime.parse(fechaStr);
            } catch (Exception e) {
                    errors.add("Fecha del hecho inválida.");
            }

            if (!errors.isEmpty()) {
                Map<String,Object> model = new HashMap<>();
                Map<String,String> values = new HashMap<>();
                values.put("titulo", titulo);
                values.put("descripcion", descripcion);
                values.put("categoria", categoriaStr);
                values.put("latitud", latitud);
                values.put("longitud", longitud);
                values.put("fecha", fechaStr);

                model.put("categorias", Categoria.values());
                model.put("values", values);
                model.put("errors", errors);

                ctx.status(400).render("hecho/hecho.nuevo.hbs", model);
                return;
            }

            Hecho hecho = new Hecho(
                    titulo,
                    descripcion,
                    categoria,
                    latitud,
                    longitud,
                    fechaHecho
            );

            RepoHechosDinamicos repoHechosDinamicos = RepoHechosDinamicos.getInstance();
            repoHechosDinamicos.withTransaction(() -> repoHechosDinamicos.agregarHecho(hecho));

            for (UploadedFile archivo : archivos) {
                String nombreArchivo = archivo.filename();
                String uploads = "uploads/";
                String rutaDestino = uploads + nombreArchivo;

                // Guardar el archivo físico
                try (InputStream in = archivo.content()) {
                    Files.copy(in, Paths.get(rutaDestino), StandardCopyOption.REPLACE_EXISTING);
                }
                String rutaVista = "/uploads/" + nombreArchivo;

                // Crear entidad y asociar
                ArchivoMultimedia nuevoArchivo = new ArchivoMultimedia(
                        nombreArchivo,
                        rutaVista,
                        hecho
                );

                hecho.agregarArchivo(nuevoArchivo);
                RepoArchivosMultimedia repoArchivosMultimedia = RepoArchivosMultimedia.getInstance();
                repoArchivosMultimedia.withTransaction(() -> repoArchivosMultimedia.agregarArchivo(nuevoArchivo));
            }

            ctx.redirect("/hechos/" + hecho.getId());

        } catch (Exception e) {
            e.printStackTrace();
            ctx.status(500).result("Ocurrió un error al crear el hecho: " + e.getMessage());
        }
    }

    public void ver(Context ctx) {
        long id = Long.parseLong(ctx.pathParam("id"));
        Long user_id = ctx.sessionAttribute("user_id");

        RepoHechosDinamicos repo = RepoHechosDinamicos.getInstance();
        Hecho hecho = repo.obtenerHechoPorId(id);

        boolean solicitudEnviada = "1".equals(ctx.queryParam("ok"));

        if (hecho == null) {
            ctx.status(404).render("errors/404.hbs", Map.of(
                    "mensaje", "No se encontró el hecho con ID " + id
            ));
            return;
        }

        if (hecho.estaEliminado()) {
            ctx.status(410).render("errors/410.hbs", Map.of(
                    "mensaje", "El hecho fue eliminado."
            ));
            return;
        }

        Map<String, Object> model = new HashMap<>();
        model.put("hecho", hecho);
        model.put("solicitudEnviada", solicitudEnviada);
        model.put("usuarioLogueado", user_id != null);

        ctx.render("hecho/hecho.detalle.hbs", model);
    }

    public void mostrarMapa(Context ctx) {
        RepoHechosDinamicos repoHechosDinamicos = RepoHechosDinamicos.getInstance();
        List<Hecho> hechos = repoHechosDinamicos.obtenerHechos();
        List<Hecho> hechosConUbicacion = hechos.stream()
            .filter(h -> h.getUbicacion() != null &&
                h.getUbicacion().getLatitud() != null &&
                h.getUbicacion().getLongitud() != null)
            .toList();
        Map<String, Object> modelo = new HashMap<>();
        modelo.put("hechos", hechosConUbicacion);

        ctx.render("hecho/mapa.hbs", modelo);
    }

    public void mostrarImportarHechos(Context ctx) {
        Map<String, Object> modelo = new HashMap<>();
        ctx.render("hecho/hecho.desde.csv.hbs", modelo);
    }

    public void importarHechosDesdeCsv(Context ctx) {

        UploadedFile archivo = ctx.uploadedFile("archivoCsv");

        try (InputStream inputStream = archivo.content()) {

            ImportarCSV csv = new ImportarCSV();
            List<Hecho> hechosImportados = csv.importarDesdeStream(inputStream);

            RepoHechosDinamicos repo = RepoHechosDinamicos.getInstance();
            repo.withTransaction(() -> {
                for (Hecho hecho : hechosImportados) {
                    repo.agregarHecho(hecho);
                }
            });

            Map<String, Object> modelo = new HashMap<>();
            modelo.put("hechos", hechosImportados);
            modelo.put("cantidad", hechosImportados.size());
            ctx.render("partials/hechos-importados.hbs", modelo);

        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}