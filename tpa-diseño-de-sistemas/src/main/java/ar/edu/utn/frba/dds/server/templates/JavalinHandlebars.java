package ar.edu.utn.frba.dds.server.templates;

import com.github.jknack.handlebars.Handlebars;
import com.github.jknack.handlebars.Template;
import io.javalin.http.Context;
import io.javalin.http.HttpStatus;
import io.javalin.rendering.FileRenderer;
import org.jetbrains.annotations.NotNull;
import com.github.jknack.handlebars.Helper;
import com.github.jknack.handlebars.Options;

import java.io.IOException;
import java.util.Collection;
import java.util.Map;


public class JavalinHandlebars implements FileRenderer {
  Handlebars handlebars = new Handlebars();

  public JavalinHandlebars() {
    this.handlebars = new Handlebars();

    handlebars.registerHelper("eq", new Helper<Object>() {
      @Override
      public Object apply(Object context, Options options) throws IOException {
        Object left = context;
        Object right = null;

        if (options.params != null && options.params.length > 0) {
          right = options.param(0, null);
        }

        if (left == null && right == null) return true;
        if (left == null || right == null) return false;

        return left.toString().equalsIgnoreCase(right.toString());
      }
    });

    handlebars.registerHelper("ifIn", (context, options) -> {
      // context = valor (provincia)
      // param0 = lista (provSeleccionadas)
      Object listObj = options.param(0, null);
      if (listObj instanceof Collection<?> col) {
        boolean contiene = (context != null && col.contains(context));
        return contiene ? options.fn() : options.inverse();
      }
      return options.inverse();
    });
  }

  @NotNull
  @Override
  public String render(@NotNull String path, @NotNull Map<String, ?> model, @NotNull Context context) {
    Template template = null;
    try {
      template = handlebars.compile("templates/" + path.replace(".hbs",""));
      return template.apply(model);
    } catch (IOException e) {
      e.printStackTrace();
      context.status(HttpStatus.NOT_FOUND);
      return "No se encuentra la página indicada...";
    }
  }
}
