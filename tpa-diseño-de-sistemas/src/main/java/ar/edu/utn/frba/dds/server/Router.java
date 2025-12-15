package ar.edu.utn.frba.dds.server;

import ar.edu.utn.frba.dds.controllers.*;
import io.github.flbulgarelli.jpa.extras.test.SimplePersistenceTest;
import io.javalin.Javalin;

public class Router implements SimplePersistenceTest {
  public void configure(Javalin app) {

    LandingController landingController = new LandingController();
    HomeController homeController = new HomeController();
    HechoController hechoController = new HechoController();
    SolicitudController solicitudController = new SolicitudController();
    ColeccionController coleccionController = new ColeccionController();
    EstadisticaController estadisticasController = new EstadisticaController();

    SessionController session = new SessionController();

    app.before(ctx -> {
      entityManager().clear();
      ctx.contentType("text/html; charset=UTF-8");
    });

    app.get("/", context -> context.redirect("/inicio"));
    app.get("/inicio", landingController::showLanding);
    app.get("/home", homeController::showHome);
    app.get("/admin", homeController::showAdminHome);

    app.get("/hechos/nuevo", hechoController::formHechoNuevo);
    app.post("/hechos", hechoController::crearHecho);
    app.get("/hechos/{id}", hechoController::ver);
    app.get("/mapa", hechoController::mostrarMapa);

    app.get("/solicitud/nueva", solicitudController::solicitarEliminacionForm);
    app.post("/solicitud/nueva", solicitudController::solicitarEliminacion);

    app.get("/login", session::showLogin);
    app.post("/login", session::createLogin);
    app.get("/logout", session::logout);
    app.get("/registrarse", session::showRegistro);
    app.post("/registrarse", session::createRegistro);

    app.get("/admin/colecciones", coleccionController::listarColecciones);
    app.get("/admin/colecciones/{id}/configuracion", coleccionController::actualizarColeccionForm);
    app.post("/admin/colecciones/{id}/configuracion", coleccionController::actualizarColeccion);
    app.get("/admin/coleccion/nueva", coleccionController::formNuevaColeccion);
    app.post("/admin/coleccion/nueva", coleccionController::crearColeccion);

    app.get("/admin/estadisticas/coleccion", estadisticasController::mostrarEstadisticaColeccion);
    app.get("/admin/estadisticas/categoria", estadisticasController::mostrarEstadisticaCategoria);

    app.get("/admin/solicitudes", solicitudController::listarSolicitudes);
    app.post("/admin/solicitudes/{id}", solicitudController::actualizarSolicitud);

    app.get("/admin/hechos/importacion", hechoController::mostrarImportarHechos);
    app.post("/admin/hechos/importacion", hechoController::importarHechosDesdeCsv);
  }
}
