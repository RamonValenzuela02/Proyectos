package ar.edu.utn.frba.dds.controllers;

import ar.edu.utn.frba.dds.model.coleccion.Coleccion;
import ar.edu.utn.frba.dds.model.coleccion.ColeccionBuilder;
import ar.edu.utn.frba.dds.model.consenso.AlgoritmoConsenso;
import ar.edu.utn.frba.dds.model.criterio.*;
import ar.edu.utn.frba.dds.model.fuente.Fuente;
import ar.edu.utn.frba.dds.model.ubicacion.Provincia;
import ar.edu.utn.frba.dds.repositorios.RepoDeColecciones;
import ar.edu.utn.frba.dds.repositorios.RepoFuentesDelSistema;
import io.javalin.http.Context;
import org.jetbrains.annotations.NotNull;

import java.util.*;

public class ColeccionController {

    public void formNuevaColeccion(Context ctx) {
        List<Fuente> fuentes = RepoFuentesDelSistema.getInstance().obtenerFuentes();

        Map<String,Object> model = new HashMap<>();
        model.put("algoritmos",AlgoritmoConsenso.values());
        model.put("fuentes",fuentes);
        model.put("categorias", Categoria.values());
        model.put("provincias", Provincia.values());
        ctx.render("coleccion/coleccion.nueva.hbs", model);
    }

    public void crearColeccion(@NotNull Context context) {

        try {
            String handle = context.formParam("handle");
            String titulo = context.formParam("titulo");
            String descripcion = context.formParam("descripcion");
            String fuenteStr = context.formParam("fuente");
            String algoritmoStr = context.formParam("algoritmo");
            String categoriaStr = context.formParam("categoria");
            String fechaDesde = context.formParam("fechaDesde");
            String fechaHasta = context.formParam("fechaHasta");
            String provinciaStr = context.formParam("provincia");
            List<Criterio> criterios = new ArrayList<>(List.of(new CriterioCumplidorSiempre()));

            if (titulo == null || titulo.isBlank() ||
                    descripcion == null || descripcion.isBlank()) {
                context.status(400).result("Faltan campos obligatorios");
                return;
            }

            AlgoritmoConsenso algoritmoConsenso = null;
            if(algoritmoStr != null && !algoritmoStr.isBlank()) {
                algoritmoConsenso = AlgoritmoConsenso.valueOf(algoritmoStr);
            }

            long fuenteId = Long.parseLong(fuenteStr);
            Fuente fuente = RepoFuentesDelSistema.getInstance().obtenerFuenteConId(fuenteId);

            Categoria categoria = null;
            if(categoriaStr != null && !categoriaStr.isBlank()) {
                categoria = Categoria.valueOf(categoriaStr.toUpperCase());
                criterios.add(new CriterioPorCategoria(categoria));
            }

            if (provinciaStr != null && !provinciaStr.isBlank()) {
                Provincia provincia = Provincia.desdeTexto(provinciaStr);
                CriterioPorProvincia criterioProvincia = new CriterioPorProvincia(provincia);
                criterios.add(criterioProvincia);
            }

            CriterioPorRangoFecha criterioFecha = new CriterioPorRangoFecha(fechaDesde,fechaHasta);
            criterios.add(criterioFecha);


            Coleccion coleccion = new ColeccionBuilder()
                    .conTitulo(titulo)
                    .conHandle(handle)
                    .conDescripcion(descripcion)
                    .conCriterios(criterios)
                    .conAlgoritmoConsenso(algoritmoConsenso)
                    .conFuente(fuente)
                    .crear();
            RepoDeColecciones repo = RepoDeColecciones.getInstance();
            repo.withTransaction(() -> repo.agregarColeccion(coleccion));

            context.redirect("/admin/colecciones");

        } catch (Exception e) {
            e.printStackTrace();
            context.status(500).result("Ocurrió un error al crear la coleccion: " + e.getMessage());
        }
    }

    public void listarColecciones(Context ctx) {

        List<Coleccion> colecciones = RepoDeColecciones.getInstance().getColecciones();
        Map<String, Object> model = new HashMap<>();
        model.put("colecciones", colecciones);
        ctx.render("coleccion/colecciones.hbs", model);
    }

    public void actualizarColeccionForm(Context ctx) {

        Long id = Long.parseLong(ctx.pathParam("id"));
        Coleccion coleccion = RepoDeColecciones.getInstance().getColeccionPorId(id);
        List<Fuente> fuentes = RepoFuentesDelSistema.getInstance().obtenerFuentes();

        Map<String,Object> model = new HashMap<>();
        model.put("coleccion",coleccion);
        model.put("algoritmos",AlgoritmoConsenso.values());
        model.put("fuentes",fuentes);
        model.put("categorias", Categoria.values());
        model.put("provincias", Provincia.values());
        ctx.render("coleccion/colecciones.configuracion.hbs", model);
    }

    public void actualizarColeccion(Context ctx) {

        String id = ctx.pathParam("id");
        String titulo = ctx.formParam("titulo");
        String descripcion = ctx.formParam("descripcion");
        String fuenteStr = ctx.formParam("fuente");
        String categoriaStr = ctx.formParam("categoria");
        String provinciaStr = ctx.formParam("provincia");
        String fechaDesdeStr = ctx.formParam("fechaDesde");
        String fechaHastaStr = ctx.formParam("fechaHasta");
        String algoritmoStr = ctx.formParam("algoritmo");

        RepoDeColecciones repo = RepoDeColecciones.getInstance();
        Coleccion coleccion = repo.getColeccionPorId(Long.parseLong(id));
        repo.withTransaction(() -> {
            coleccion.borrarCriterios();
            if(titulo != null && !titulo.isBlank()){
                coleccion.setTitulo(titulo);
            }

            if(descripcion != null && !descripcion.isBlank()){
                coleccion.setDescripcion(descripcion);
            }

            if (fuenteStr != null && !fuenteStr.isBlank()) {
                Long fuenteId = Long.parseLong(fuenteStr);
                Fuente fuente = RepoFuentesDelSistema.getInstance().obtenerFuenteConId(fuenteId);
                coleccion.setFuente(fuente);
            }

            if (categoriaStr != null && !categoriaStr.isBlank()) {
                Categoria categoria = Categoria.valueOf(categoriaStr);
                Criterio criterioPorCategoria = new CriterioPorCategoria(categoria);
                coleccion.agregarCriterio(criterioPorCategoria);
            }

            if (provinciaStr != null && !provinciaStr.isBlank()) {
                Provincia provincia = Provincia.desdeTexto(provinciaStr);
                Criterio criterioPorProvincia = new CriterioPorProvincia(provincia);
                coleccion.agregarCriterio(criterioPorProvincia);
            }

            if ( (fechaDesdeStr != null && !fechaDesdeStr.isBlank())
                    || (fechaHastaStr != null && !fechaHastaStr.isBlank()) ) {

                Criterio criterioPorFecha = new CriterioPorRangoFecha(fechaDesdeStr, fechaHastaStr);
                coleccion.agregarCriterio(criterioPorFecha);
            }

            if (algoritmoStr == null || algoritmoStr.isBlank()) {
                coleccion.setAlgoritmoConsenso(null);
            } else {
                AlgoritmoConsenso algoritmo = AlgoritmoConsenso.valueOf(algoritmoStr);
                coleccion.setAlgoritmoConsenso(algoritmo);
            }
            repo.entityManager().merge(coleccion);
        });

        ctx.redirect("/admin/colecciones");
    }
}
