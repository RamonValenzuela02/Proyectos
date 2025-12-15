package ar.edu.utn.frba.dds.model.coleccion;

import java.util.ArrayList;
import java.util.List;

import static java.util.Objects.requireNonNull;

import ar.edu.utn.frba.dds.model.consenso.AlgoritmoConsenso;
import ar.edu.utn.frba.dds.model.criterio.Criterio;
import ar.edu.utn.frba.dds.model.fuente.Fuente;

public class ColeccionBuilder {
    private String handle;
    private String titulo;
    private String descripcion;
    private Fuente fuente;
    private List<Criterio> criterios = new ArrayList<>();
    private AlgoritmoConsenso algoritmoConsenso;

    public ColeccionBuilder conHandle(String handle) {
        this.handle = handle;
        return this;
    }

    public ColeccionBuilder conTitulo(String titulo) {
        this.titulo = titulo;
        return this;
    }

    public ColeccionBuilder conDescripcion(String descripcion) {
        this.descripcion = descripcion;
        return this;
    }

    public ColeccionBuilder conFuente(Fuente fuente) {
        this.fuente = fuente;
        return this;
    }

    public ColeccionBuilder conCriterios(List<Criterio> criterios) {
        this.criterios = criterios;
        return this;
    }

    public ColeccionBuilder conAlgoritmoConsenso(AlgoritmoConsenso algoritmo) {
        this.algoritmoConsenso = algoritmo;
        return this;
    }

    public Coleccion crear() {
        validarColeccion(handle,titulo,descripcion,fuente,criterios);
        Coleccion coleccion = new Coleccion(handle, titulo, descripcion, fuente, criterios, algoritmoConsenso);
        return coleccion;
    }

    private void validarColeccion(String handle, String titulo, String descripcion, Fuente fuente, List<Criterio> criterios) {
        requireNonNull(handle);
        requireNonNull(titulo);
        requireNonNull(descripcion);
        requireNonNull(fuente);
        //requireNonNull(criterios);

        if (!handle.matches("^[a-zA-Z0-9_-]+$")) {
            throw new IllegalArgumentException("El handle debe ser alfanumérico, sin espacios.");
        }
    }
}
