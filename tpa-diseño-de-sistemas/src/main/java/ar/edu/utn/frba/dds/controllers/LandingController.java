package ar.edu.utn.frba.dds.controllers;

import ar.edu.utn.frba.dds.model.Hecho.Hecho;
import ar.edu.utn.frba.dds.model.coleccion.Coleccion;
import ar.edu.utn.frba.dds.model.fuente.Fuente;
import ar.edu.utn.frba.dds.repositorios.RepoDeColecciones;
import ar.edu.utn.frba.dds.repositorios.RepoFuentesDelSistema;
import ar.edu.utn.frba.dds.repositorios.RepoHechosDinamicos;
import io.javalin.http.Context;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class LandingController {

    public Map<String, Object> modeloLanding(Context ctx) {

        RepoDeColecciones repoDeColecciones = RepoDeColecciones.getInstance();
        List<Coleccion> colecciones = repoDeColecciones.getColecciones();

        RepoHechosDinamicos repoDeHechosDinamicos = RepoHechosDinamicos.getInstance();
        List<Hecho> hechos = repoDeHechosDinamicos.obtenerHechos();

        RepoFuentesDelSistema repoFuentesDelSistema = RepoFuentesDelSistema.getInstance();
        List<Fuente> fuentes = repoFuentesDelSistema.obtenerFuentes();

        Map<String, Object> modelo = new HashMap<>();
        modelo.put("cantidad-fuentes", fuentes.size());
        modelo.put("cantidad-colecciones", colecciones.size());
        modelo.put("cantidad-hechos", hechos.size());
        modelo.put("hechos", hechos);
        return modelo;
    };

    public void showLanding(Context ctx) {
        ctx.render("landing/landing.hbs", modeloLanding(ctx));
    }
}
