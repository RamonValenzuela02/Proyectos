package ar.edu.utn.frba.dds.repositorios;

import ar.edu.utn.frba.dds.model.Multimedia.ArchivoMultimedia;
import io.github.flbulgarelli.jpa.extras.simple.WithSimplePersistenceUnit;

public class RepoArchivosMultimedia implements WithSimplePersistenceUnit {
  private static final RepoArchivosMultimedia INSTANCE = new RepoArchivosMultimedia();

  public static RepoArchivosMultimedia getInstance() {
    return INSTANCE;
  }

  public void agregarArchivo(ArchivoMultimedia archivo) {
    withTransaction(() -> entityManager().persist(archivo));
  }
}
