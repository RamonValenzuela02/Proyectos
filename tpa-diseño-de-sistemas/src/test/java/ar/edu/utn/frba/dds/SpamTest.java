package ar.edu.utn.frba.dds;

import static org.junit.jupiter.api.Assertions.*;
import static org.mockito.Mockito.*;

import ar.edu.utn.frba.dds.model.solicitud.DetectorDeSpam;
import ar.edu.utn.frba.dds.model.solicitud.Estado;
import ar.edu.utn.frba.dds.model.fuente.FuenteDinamica;
import ar.edu.utn.frba.dds.model.solicitud.SolicitudDeEliminacion;
import ar.edu.utn.frba.dds.repositorios.RepoSolicitudesDeEliminacion;
import org.junit.jupiter.api.Test;

/*
public class SpamTest {

    @Test
    public void registrarSolicitud_noEsSpam_quedaPendiente() {
        // Mockeo el detector para que NO detecte spam
        DetectorDeSpam detector = mock(DetectorDeSpam.class);
        when(detector.esSpam("no es compatible titulo con su descripcion")).thenReturn(false);

        RepoSolicitudesDeEliminacion gestor = new RepoSolicitudesDeEliminacion(detector);
        FuenteDinamica fuente = new FuenteDinamica();

        SolicitudDeEliminacion solicitud = new SolicitudDeEliminacion(
                "no compatibilidad",
                "no es compatible titulo con su descripcion",
                fuente);

        gestor.registrarSolicituDeEliminacion(solicitud);

        assertTrue(gestor.getSolicitudes().contains(solicitud));
        assertEquals(Estado.PENDIENTE, solicitud.getEstado());
    }

    @Test
    public void registrarSolicitud_esSpam_rechazaAutomaticamente() {
        // Mockeo el detector para que detecte spam
        DetectorDeSpam detector = mock(DetectorDeSpam.class);
        when(detector.esSpam("no es compatible titulo con su descripcion")).thenReturn(true);

        RepoSolicitudesDeEliminacion gestor = new RepoSolicitudesDeEliminacion(detector);
        FuenteDinamica fuente = new FuenteDinamica();

        SolicitudDeEliminacion solicitud = new SolicitudDeEliminacion(
                "no compatibilidad",
                "no es compatible titulo con su descripcion",
                fuente);

        gestor.registrarSolicituDeEliminacion(solicitud);

        assertTrue(gestor.getSolicitudes().contains(solicitud));
        assertEquals(Estado.RECHAZADA, solicitud.getEstado());
    }
}

 */
