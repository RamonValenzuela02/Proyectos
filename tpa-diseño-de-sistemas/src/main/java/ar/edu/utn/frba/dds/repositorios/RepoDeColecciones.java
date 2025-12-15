package ar.edu.utn.frba.dds.repositorios;

import ar.edu.utn.frba.dds.model.coleccion.Coleccion;
import io.github.flbulgarelli.jpa.extras.simple.WithSimplePersistenceUnit;
import java.util.List;

public class RepoDeColecciones implements WithSimplePersistenceUnit {
    private static final RepoDeColecciones INSTANCE = new RepoDeColecciones();

    private RepoDeColecciones() {}

    public static RepoDeColecciones getInstance() {
      return INSTANCE;
    }

    public void agregarColeccion(Coleccion coleccion) {
      entityManager().persist(coleccion);
    }

    public List<Coleccion> getColecciones() {
      return entityManager()
        .createQuery("SELECT c FROM Coleccion c", Coleccion.class)
        .getResultList();
    }

    public Coleccion getColeccionPorId(Long id) {
        return entityManager().find(Coleccion.class, id);
    }
}
