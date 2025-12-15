package ar.edu.utn.frba.dds.repositorios;

import ar.edu.utn.frba.dds.model.Hecho.Hecho;
import ar.edu.utn.frba.dds.model.coleccion.Coleccion;
import ar.edu.utn.frba.dds.model.criterio.Categoria;
import ar.edu.utn.frba.dds.model.fuente.Fuente;
import ar.edu.utn.frba.dds.model.ubicacion.Provincia;
import io.github.flbulgarelli.jpa.extras.simple.WithSimplePersistenceUnit;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;


public class RepoFuentesDelSistema implements WithSimplePersistenceUnit {
  private static final RepoFuentesDelSistema INSTANCE = new RepoFuentesDelSistema();

  private RepoFuentesDelSistema() {}

  public static RepoFuentesDelSistema getInstance() {
    return INSTANCE;
  }

  public void agregarFuente(Fuente fuente) {
    entityManager().persist(fuente);
  }

  public void agregarFuentes(List<Fuente> fuentesNuevas ) {
    fuentesNuevas.forEach(this::agregarFuente);
  }

  public List<Fuente> obtenerFuentes(){
    return entityManager()
      .createQuery("SELECT f FROM Fuente f", Fuente.class)
      .getResultList();
  }

  public Fuente obtenerFuenteConId(Long fuenteId) {
    return entityManager()
      .createQuery("SELECT f FROM Fuente f WHERE f.id = :fuenteId", Fuente.class)
      .setParameter("fuenteId", fuenteId)
      .getSingleResult();
  }
}
