package ar.edu.utn.frba.dds.controllers;

import ar.edu.utn.frba.dds.model.Hecho.Hecho;
import ar.edu.utn.frba.dds.model.solicitud.DetectorDeSpamBasico;
import ar.edu.utn.frba.dds.model.solicitud.EstadoSolicitudDinamica;
import ar.edu.utn.frba.dds.model.solicitud.ServicioDeSolicitudesEliminacion;
import ar.edu.utn.frba.dds.model.solicitud.SolicitudDeEliminacion;
import ar.edu.utn.frba.dds.repositorios.RepoHechosDinamicos;
import ar.edu.utn.frba.dds.repositorios.RepoSolicitudesDeEliminacion;
import io.javalin.http.Context;
import org.jetbrains.annotations.NotNull;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;

public class SolicitudController {

    public void listarSolicitudes(@NotNull Context ctx) {
        String estadoStr = ctx.queryParam("estado");
        RepoSolicitudesDeEliminacion repo = RepoSolicitudesDeEliminacion.getInstance();

        EstadoSolicitudDinamica estado;
        if(estadoStr == null || estadoStr.isBlank()){
            estado = EstadoSolicitudDinamica.PENDIENTE;
        }else {
            estado = EstadoSolicitudDinamica.valueOf(estadoStr.toUpperCase());
        }

        List<SolicitudDeEliminacion> solicitudes = repo.getSolicitudesPorEstado(estado);

        Map<String, Object> model = new HashMap<>();
        model.put("solicitudes",solicitudes);
        model.put("pendiente", estado == EstadoSolicitudDinamica.PENDIENTE);
        model.put("cantidadSpam",repo.getCantidadDeSpam());

        boolean esHtmx = "true".equalsIgnoreCase(ctx.header("HX-Request"));

        if (esHtmx) {
            ctx.render("partials/resultados.solicitudes.hbs", model);
        } else {
            ctx.render("solicitudes/listar.solicitudes.hbs", model);
        }
    }

    public void actualizarSolicitud(@NotNull Context ctx) {
        long id = Long.valueOf(ctx.pathParam("id"));
        String eleccion =  ctx.formParam("eleccion");

        RepoSolicitudesDeEliminacion repo = RepoSolicitudesDeEliminacion.getInstance();
        SolicitudDeEliminacion soli = repo.getSolicitudPorId(id);

        if(eleccion.equalsIgnoreCase("APROBADA")) {
            repo.withTransaction(() -> {
                SolicitudDeEliminacion solicitud = repo.getSolicitudPorId(id);
                if (solicitud != null) {
                    solicitud.aceptar();
                    solicitud.getHecho().setEliminado(true);
                    repo.entityManager().merge(solicitud);
                }
            });
        }else {
            repo.withTransaction(() -> {
                SolicitudDeEliminacion solicitud = repo.getSolicitudPorId(id);
                if (solicitud != null) {
                    solicitud.rechazar();
                    repo.entityManager().merge(solicitud);
                }
            });
        }

        boolean esHtmx = "true".equalsIgnoreCase(ctx.header("HX-Request"));

        if (esHtmx) {
            listarSolicitudes(ctx);
        }else{
            ctx.redirect("/admin");
        }
    }

    public void solicitarEliminacionForm(@NotNull Context ctx) {
        Long hechoId = Long.parseLong(Objects.requireNonNull(ctx.queryParam("hechoId")));

        Hecho hecho = RepoHechosDinamicos.getInstance().obtenerHechoPorId(hechoId);

        Map<String,Object> model = new HashMap<>();
        model.put("hecho",hecho);
        ctx.render("solicitudes/solicitud.eliminacion.nueva.hbs", model);
    }

    public void solicitarEliminacion(@NotNull Context context) {

        long hechoId = Long.parseLong(Objects.requireNonNull(context.formParam("hechoId")));
        String motivo = context.formParam("motivo");

        Hecho hecho = RepoHechosDinamicos.getInstance().obtenerHechoPorId(hechoId);

        if (motivo.length() < 500) {
            Map<String, Object> model = new HashMap<>();
            model.put("hecho", hecho);
            model.put("motivo", motivo);
            model.put("error", "El motivo debe tener al menos 500 caracteres.");

            context.status(400)
                    .render("solicitudes/solicitud.eliminacion.nueva.hbs", model);
            return;
        }

        SolicitudDeEliminacion solicitudDeEliminacion = new SolicitudDeEliminacion(hecho,motivo);
        ServicioDeSolicitudesEliminacion servicio = new ServicioDeSolicitudesEliminacion(new DetectorDeSpamBasico());

        RepoSolicitudesDeEliminacion.getInstance().withTransaction(() -> {
            servicio.registrarSolicituDeEliminacion(solicitudDeEliminacion);
        });

        context.redirect("/hecho/" + hechoId + "?ok=1");
    }
}
