package ar.edu.utn.frba.dds.model.solicitud;

import ar.edu.utn.frba.dds.repositorios.RepoSolicitudesDeEliminacion;


public class ServicioDeSolicitudesEliminacion {
  private Long id;
  private  DetectorDeSpam detector;

  public ServicioDeSolicitudesEliminacion(DetectorDeSpam detector) {
    this.detector = detector;
  }

  public void registrarSolicituDeEliminacion(SolicitudDeEliminacion solicitud) {
    if (detector.esSpam(solicitud.getMotivo())){
      solicitud.rechazar();
      //RepoSolicitudesDeEliminacion.getInstance().registrarSpam();
    }
    RepoSolicitudesDeEliminacion.getInstance().registrarSolicituDeEliminacion(solicitud);
  }

}
