package ar.edu.utn.frba.dds.repositorios;

import io.github.flbulgarelli.jpa.extras.simple.WithSimplePersistenceUnit;
import java.util.List;
import ar.edu.utn.frba.dds.model.Hecho.Hecho;



public class RepoHechosDinamicos implements WithSimplePersistenceUnit {
  private static final RepoHechosDinamicos INSTANCE = new RepoHechosDinamicos();

  private RepoHechosDinamicos() {
  }

  public static RepoHechosDinamicos getInstance() {
    return INSTANCE;
  }

  public void agregarHecho(Hecho hecho) {
    entityManager().persist(hecho);
  }

  public List<Hecho> obtenerHechos() {
    return entityManager()
      .createQuery("SELECT h FROM Hecho h", Hecho.class)
      .getResultList();
  }

  public Hecho obtenerHechoPorId(Long id) {
    return entityManager().find(Hecho.class, id);
  }
}
