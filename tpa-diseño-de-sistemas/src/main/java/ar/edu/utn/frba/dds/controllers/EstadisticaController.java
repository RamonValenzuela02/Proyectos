package ar.edu.utn.frba.dds.controllers;

import ar.edu.utn.frba.dds.model.Estadisticas;
import ar.edu.utn.frba.dds.model.coleccion.Coleccion;
import ar.edu.utn.frba.dds.model.criterio.Categoria;
import ar.edu.utn.frba.dds.model.ubicacion.Provincia;
import ar.edu.utn.frba.dds.repositorios.RepoDeColecciones;
import ar.edu.utn.frba.dds.repositorios.RepoFuentesDelSistema;
import io.javalin.http.Context;

import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

public class EstadisticaController {

    public void mostrarEstadisticaColeccion(Context ctx){
        long id = Long.parseLong(Objects.requireNonNull(ctx.queryParam("id")));
        RepoDeColecciones repo = RepoDeColecciones.getInstance();
        Coleccion coleccion = repo.getColeccionPorId(id);

        Estadisticas estadisticas = Estadisticas.getInstance();
        Provincia provinciaConMasHechos = estadisticas.provinciaConMayorCantidadDeHechosEn(coleccion);

        Map<String, Object> modelo = new HashMap<>();
        modelo.put("coleccion", coleccion);
        modelo.put("cantidadHechos", coleccion.getHechos().size());
        modelo.put("provincia", provinciaConMasHechos);
        modelo.put("cantidadHechosProvincia", coleccion.getHechosPorProvincia(provinciaConMasHechos).size());

        ctx.render("partials/estadisticas/estadistica.coleccion.hbs", modelo);
    }

    public void mostrarEstadisticaCategoria(Context ctx){
        String categoriaStr =  Objects.requireNonNull(ctx.queryParam("categoria"));

        Categoria categoria = Categoria.valueOf(categoriaStr.toUpperCase());

        Estadisticas estadisticas = Estadisticas.getInstance();
        Provincia provincia = estadisticas.provinciaConMayorCantidadDeHechosDe(categoria);
        Integer cantidadHechos = estadisticas.cantidadHechosDeCategoriaEnProvincia(categoria, provincia);

        Integer hora = estadisticas.horaDelDiaQueOcurrenMasHechosDe(categoria);

        Map<String, Object> modelo = new HashMap<>();
        modelo.put("categoria", categoria);
        modelo.put("provincia", provincia);
        modelo.put("cantidadHechos", cantidadHechos);
        modelo.put("hora", hora);

        ctx.render("partials/estadisticas/estadistica.categoria.hbs", modelo);
    }
}

