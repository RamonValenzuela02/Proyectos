package ar.edu.utn.frba.dds.model;

import ar.edu.utn.frba.dds.model.Hecho.Hecho;
import ar.edu.utn.frba.dds.model.coleccion.Coleccion;
import ar.edu.utn.frba.dds.model.criterio.Categoria;
import ar.edu.utn.frba.dds.model.fuente.Fuente;
import ar.edu.utn.frba.dds.model.ubicacion.Provincia;
import ar.edu.utn.frba.dds.repositorios.RepoFuentesDelSistema;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

public class Estadisticas {
    private static Estadisticas INSTANCE;
    private final RepoFuentesDelSistema repoFuentesDelSistema;

    public Estadisticas() {
        this.repoFuentesDelSistema = RepoFuentesDelSistema.getInstance();
    };

    public static Estadisticas getInstance() {
        if (INSTANCE == null) {
            INSTANCE = new Estadisticas();
        }
        return INSTANCE;
    }

    public List<Hecho> obtenerHechosSimilares(String texto) {
        return repoFuentesDelSistema.obtenerFuentes().stream()
                .flatMap(fuente -> fuente.obtenerHechos().stream())
                .filter(hecho -> hecho.cumpleConBusqueda(texto))
                .collect(Collectors.toList());
    }

    public Provincia provinciaConMayorCantidadDeHechosDe(Categoria categoria) {
        return repoFuentesDelSistema.obtenerFuentes().stream()
                .flatMap(fuente -> fuente.obtenerHechos().stream())
                .filter(hecho -> hecho.getCategoria()==categoria)
                .collect(Collectors.groupingBy(Hecho::getProvincia,Collectors.counting()))
                .entrySet().stream()
                .max(Map.Entry.comparingByValue())
                .map(Map.Entry::getKey)
                .orElse(null);
    }

    public Integer horaDelDiaQueOcurrenMasHechosDe(Categoria categoria) {
        return repoFuentesDelSistema.obtenerFuentes().stream()
                .flatMap(fuente -> fuente.obtenerHechos().stream())
                .filter(hecho -> hecho.getCategoria()==categoria)
                .collect(Collectors.groupingBy(Hecho::getHoraDelHecho,Collectors.counting()))
                .entrySet().stream()
                .max(Map.Entry.comparingByValue())
                .map(Map.Entry::getKey)
                .orElse(null);
    }

    public Categoria categoriaConMayorCantidadDeHechos(){

        RepoFuentesDelSistema repoFuentes =  RepoFuentesDelSistema.getInstance();
        List<Fuente> fuentes = repoFuentes.obtenerFuentes();
        Map<Categoria, Integer> cantidadHechosPorCategoria = new HashMap<>();
        List<Hecho> hechos = new ArrayList<>();

        for(Fuente fuente : fuentes){
            hechos.addAll(fuente.obtenerHechos());
        }

        for(Hecho hecho : hechos){
            Categoria categoria = hecho.getCategoria();
            cantidadHechosPorCategoria.merge(categoria, 1, Integer::sum);
        }

        Categoria categoria = null;
        int max = 0;
        for(Map.Entry<Categoria, Integer> m : cantidadHechosPorCategoria.entrySet()){
            if(categoria == null || m.getValue() > max){
                categoria = m.getKey();
                max = m.getValue();
            }
        }
        return categoria;
    }

    public int cantidadDeHechosDeCategoria(Categoria categoria) {
        RepoFuentesDelSistema repoFuentesDelSistema = RepoFuentesDelSistema.getInstance();
        List<Fuente> fuentes = repoFuentesDelSistema.obtenerFuentes();

        int contador = 0;

        for (Fuente fuente : fuentes) {
            for (Hecho hecho : fuente.obtenerHechos()) {
                if (categoria.equals(hecho.getCategoria())) {
                    contador++;
                }
            }
        }

        return contador;
    }

    public int cantidadHechosDeCategoriaEnProvincia(Categoria categoria, Provincia provincia) {
        RepoFuentesDelSistema repoFuentesDelSistema = RepoFuentesDelSistema.getInstance();
        List<Fuente> fuentes = repoFuentesDelSistema.obtenerFuentes();

        int contador = 0;
        for (Fuente fuente : fuentes) {
            for (Hecho hecho : fuente.obtenerHechos()) {
                if (provincia.equals(hecho.getProvincia()) &&  categoria.equals(hecho.getCategoria())) {
                    contador++;
                }
            }
        }
        return contador;
    };

    public Provincia provinciaConMayorCantidadDeHechosEn(Coleccion coleccion){

        Map<Provincia, Integer> cantidadHechosPorProvincia = new HashMap<>();
        List<Hecho> hechos = coleccion.getHechos();

        for(Hecho hecho : hechos){
            Provincia provincia = hecho.getProvincia();
            cantidadHechosPorProvincia.merge(provincia, 1, Integer::sum);
        }

        Provincia provincia = null;
        int max = 0;
        for(Map.Entry<Provincia, Integer> m : cantidadHechosPorProvincia.entrySet()){
            if(provincia == null || m.getValue() > max){
                provincia = m.getKey();
                max = m.getValue();
            }
        }
        return provincia;
    };

    public Provincia provinciaConMayorCantidadDeHechosReportados(){
        RepoFuentesDelSistema repoFuentesDelSistema = RepoFuentesDelSistema.getInstance();

        List<Fuente> fuentes = repoFuentesDelSistema.obtenerFuentes();
        List<Hecho> hechos = new ArrayList<>();

        for(Fuente fuente : fuentes){
            hechos.addAll(fuente.obtenerHechos());
        }

        Map<Provincia, Integer> cantidadHechosPorProvincia = new HashMap<>();

        for(Hecho hecho : hechos){
            Provincia provincia = hecho.getProvincia();
            cantidadHechosPorProvincia.merge(provincia, 1, Integer::sum);
        }

        Provincia provincia = null;
        int max = 0;
        for(Map.Entry<Provincia, Integer> m : cantidadHechosPorProvincia.entrySet()){
            if(provincia == null || m.getValue() > max){
                provincia = m.getKey();
                max = m.getValue();
            }
        }
        return provincia;
    };

}
