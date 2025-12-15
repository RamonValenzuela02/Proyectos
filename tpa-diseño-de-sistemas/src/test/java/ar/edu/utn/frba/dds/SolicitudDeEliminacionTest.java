package ar.edu.utn.frba.dds;

import ar.edu.utn.frba.dds.model.Hecho.Hecho;
import ar.edu.utn.frba.dds.model.criterio.Categoria;
import ar.edu.utn.frba.dds.model.solicitud.DetectorDeSpam;
import ar.edu.utn.frba.dds.model.solicitud.Estado;
import ar.edu.utn.frba.dds.model.fuente.FuenteDinamica;
import ar.edu.utn.frba.dds.model.solicitud.SolicitudDeEliminacion;
import ar.edu.utn.frba.dds.repositorios.RepoSolicitudesDeEliminacion;
import java.time.LocalDateTime;
import java.util.ArrayList;
import java.util.List;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.*;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

public class SolicitudDeEliminacionTest {
  DetectorDeSpam detector;
  RepoSolicitudesDeEliminacion repo;
  FuenteDinamica fuente;
  Hecho hecho;

  @BeforeEach
  public void mockeoDetectorDeSpam() {
    detector = mock(DetectorDeSpam.class);
    repo = mock(RepoSolicitudesDeEliminacion.class);
    fuente = mock(FuenteDinamica.class);
    hecho = new Hecho(
      "lluvia torrencial en sudafrica","dia soliado en alemania",
      Categoria.INCENDIO_FORESTAL,
      "1000",
      "2000",
      LocalDateTime.now(),
      LocalDateTime.now());
  }

  @Test
  public void crearSolicitudDeEliminacion() {
    when(detector.esSpam("no es compatible titulo con su descripcion")).thenReturn(false);
    Hecho hecho = new Hecho(
      "lluvia torrencial en sudafrica","dia soliado en alemania",
      Categoria.INCENDIO_FORESTAL,
      "1000",
      "2000",
      LocalDateTime.now(),
      LocalDateTime.now());
    when(fuente.obtenerHechos()).thenReturn(new ArrayList<>());

    SolicitudDeEliminacion solicitud = new SolicitudDeEliminacion(hecho,
      "no es compatible titulo con su descripcion",
      fuente);

    List<SolicitudDeEliminacion> solicitudes = new ArrayList<>();
    solicitudes.add(solicitud);

    when(repo.getSolicitudes()).thenReturn(solicitudes);

    //se agrega a la lista de solicitudes y se verifica si es spam
    repo.registrarSolicituDeEliminacion(solicitud);

    assertEquals(1, repo.getSolicitudes().size());
    assertEquals(Estado.PENDIENTE, solicitud.getEstado());

  }

  @Test
  public void rechazarPorSpamSolicitudDeEliminacion() {
    when(detector.esSpam("no es compatible titulo con su descripcion")).thenReturn(true);

    RepoSolicitudesDeEliminacion repo = RepoSolicitudesDeEliminacion.getInstance();

    FuenteDinamica fuente = new FuenteDinamica();
    SolicitudDeEliminacion solicitud = new SolicitudDeEliminacion(
      hecho,
      "no es compatible titulo con su descripcion",
      fuente
    );

    repo.registrarSolicituDeEliminacion(solicitud);

    assertEquals(Estado.RECHAZADA, solicitud.getEstado());
  }
}