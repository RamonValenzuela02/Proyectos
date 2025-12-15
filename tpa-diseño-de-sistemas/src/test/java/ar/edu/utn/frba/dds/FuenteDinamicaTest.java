package ar.edu.utn.frba.dds;

import static java.lang.Thread.sleep;
import static org.junit.jupiter.api.Assertions.*;

import ar.edu.utn.frba.dds.model.solicitud.SolicitudDinamica;
import ar.edu.utn.frba.dds.model.criterio.Categoria;
import ar.edu.utn.frba.dds.model.fuente.FuenteDinamica;
import ar.edu.utn.frba.dds.repositorios.RepoSolicitudesDinamicas;
import java.time.LocalDateTime;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;

/*
class FuenteDinamicaTest {
  RepoSolicitudesDinamicas repoSolicitudes = RepoSolicitudesDinamicas.getInstance();

  @DisplayName("Como persona contribuyente, deseo poder crear un hecho a partir de una fuente dinámica. ") // req 1
  @Test
  void agregarHechoNoAgregaDirectamenteAListaDeHechos() {
    FuenteDinamica fuente = new FuenteDinamica();
    fuente.agregarHecho("habia una vez",
      "holaholhola",
      Categoria.INCENDIO_FORESTAL,
      "1234",
      "5678",
      LocalDateTime.now(),
      LocalDateTime.now());

    assertEquals(1, fuente.obtenerHechos().size());
  }

  @Test
  void agregarHechoYAceptarLaSolicitud() {
    FuenteDinamica fuente = new FuenteDinamica();
    fuente.agregarHecho("habia una vez",
      "holaholhola",
      Categoria.INCENDIO_FORESTAL,
      "1234",
      "5678",
      LocalDateTime.now(),
      LocalDateTime.now());

    SolicitudDinamica solicitud  = RepoSolicitudesDinamicas.getInstance().getSolicitudes().get(0);
    solicitud.aceptar();

    assertEquals(1, fuente.obtenerHechos().size());
  }

  @Test
  void agregarHechoYRechazarSolicitud() {
    FuenteDinamica fuente = new FuenteDinamica();
    fuente.agregarHecho("habia una vez",
      "holaholhola",
      Categoria.INCENDIO_FORESTAL,
      "1234",
      "5678",
      LocalDateTime.now(),
      LocalDateTime.now());

    SolicitudDinamica solicitud  = RepoSolicitudesDinamicas.getInstance().getSolicitudes().get(0);

    solicitud.rechazar();
    assertEquals(0, fuente.obtenerHechos().size());
  }

  @Test
  void agregarHechoYAceptarLaSolicitudConSugerencia() {
    FuenteDinamica fuente = new FuenteDinamica();
    fuente.agregarHecho("habia una vez",
      "holaholhola",
      Categoria.INCENDIO_FORESTAL,
      "1234",
      "5678",
      LocalDateTime.now(),
      LocalDateTime.now());

    SolicitudDinamica solicitud  = RepoSolicitudesDinamicas.getInstance().getSolicitudes().get(0);

    solicitud.aceptarConSugerencia("latitud invalida");
    
    assertEquals(1,fuente.obtenerHechos().size());
  }
}

 */
