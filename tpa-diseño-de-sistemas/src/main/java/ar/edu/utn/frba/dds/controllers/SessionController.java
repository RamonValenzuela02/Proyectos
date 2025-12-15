package ar.edu.utn.frba.dds.controllers;

import ar.edu.utn.frba.dds.model.Usuario.TipoUsuario;
import ar.edu.utn.frba.dds.model.Usuario.Usuario;
import io.github.flbulgarelli.jpa.extras.TransactionalOps;
import io.github.flbulgarelli.jpa.extras.simple.WithSimplePersistenceUnit;
import io.javalin.http.Context;
import org.jetbrains.annotations.NotNull;
import java.util.HashMap;
import java.util.Map;
import ar.edu.utn.frba.dds.repositorios.RepoUsuarios;

public class SessionController implements WithSimplePersistenceUnit, TransactionalOps {


  public void showLogin(Context ctx) {
    if (ctx.sessionAttribute("user_id") != null) {
      ctx.redirect("/");
      return;
    }

    Map<String, Object> modelo = new HashMap<>();

    if ("true".equals(ctx.queryParam("error"))) {
      modelo.put("error", "Usuario o contraseña inválidas");
    }

    ctx.render("session/login.hbs", modelo);
  }

  public void createLogin(Context ctx) {
    try {
      RepoUsuarios repo = RepoUsuarios.getInstance();
      String nombre = ctx.formParam("nombre");
      String password = ctx.formParam("password");

      var usuario = repo.buscarPorNombreYPassword(nombre, password);

      ctx.sessionAttribute("user_id", usuario.getId());
      ctx.sessionAttribute("tipo_usuario", usuario.getTipo());
      if (usuario.getTipo() == TipoUsuario.ADMINISTRADOR) {
        ctx.redirect("/admin");
      } else {
        ctx.redirect("/home");
      }
    }catch(Exception e) {
      ctx.redirect("/login?error=true");
    }
  }

  public void logout(Context ctx) {
    ctx.req().getSession().invalidate();
    ctx.redirect("/home");
  }

  public void showRegistro(Context ctx) {
    Map<String, Object> modelo = new HashMap<>();
    ctx.render("session/registro.hbs", modelo);
  }

  public void createRegistro(Context ctx) {
    RepoUsuarios repo = RepoUsuarios.getInstance();
    String nombre = ctx.formParam("nombre");
    String password = ctx.formParam("password");

    repo.withTransaction(() -> repo.agregar(new Usuario(TipoUsuario.CONTRIBUYENTE,nombre,password)));

    ctx.redirect("/login");
  }
}
