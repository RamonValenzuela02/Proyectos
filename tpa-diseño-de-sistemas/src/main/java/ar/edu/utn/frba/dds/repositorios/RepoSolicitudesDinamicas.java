package ar.edu.utn.frba.dds.repositorios;

import ar.edu.utn.frba.dds.model.solicitud.SolicitudDinamica;
import io.github.flbulgarelli.jpa.extras.simple.WithSimplePersistenceUnit;
import java.util.List;


public class RepoSolicitudesDinamicas implements WithSimplePersistenceUnit {
  private static final RepoSolicitudesDinamicas INSTANCE = new RepoSolicitudesDinamicas();

  private RepoSolicitudesDinamicas() {
  }

  public static RepoSolicitudesDinamicas getInstance() {
    return INSTANCE;
  }

  public void agregarSolicitud(SolicitudDinamica solicitud) {
    entityManager().persist(solicitud);
  }

  public List<SolicitudDinamica> getSolicitudes() {
    return entityManager()
      .createQuery("SELECT s FROM SolicitudDinamica s", SolicitudDinamica.class)
      .getResultList();
  }

}
