package ar.edu.utn.frba.dds.model.fuente;

import ar.edu.utn.frba.dds.model.criterio.Categoria;
import java.time.LocalDateTime;
import java.util.List;
import ar.edu.utn.frba.dds.model.Hecho.Hecho;
import ar.edu.utn.frba.dds.model.solicitud.SolicitudDinamica;
import ar.edu.utn.frba.dds.repositorios.RepoHechosDinamicos;
import ar.edu.utn.frba.dds.repositorios.RepoSolicitudesDinamicas;
import javax.persistence.Entity;

@Entity
public class FuenteDinamica extends Fuente{
  public FuenteDinamica() {
    //RepoFuentesDelSistema.getInstance().agregarFuente(this);
  }

  public void agregarHecho(String titulo,
                           String descripcion,
                           Categoria categoria,
                           String latitud,
                           String longitud,
                           LocalDateTime fechaHecho,
                           LocalDateTime fechaCarga) {
    SolicitudDinamica solicitud = new SolicitudDinamica(titulo,descripcion,categoria,latitud,longitud,fechaHecho,fechaCarga);
    RepoSolicitudesDinamicas.getInstance().agregarSolicitud(solicitud);
  }

  public List<Hecho> obtenerHechos() {
    return RepoHechosDinamicos.getInstance().obtenerHechos();
  }
}