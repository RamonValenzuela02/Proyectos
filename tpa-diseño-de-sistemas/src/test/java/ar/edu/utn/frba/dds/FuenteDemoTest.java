package ar.edu.utn.frba.dds;

import static org.junit.jupiter.api.Assertions.*;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

import ar.edu.utn.frba.dds.model.coleccion.Coleccion;
import ar.edu.utn.frba.dds.model.consenso.AlgoritmoConsenso;
import ar.edu.utn.frba.dds.model.criterio.Criterio;
import ar.edu.utn.frba.dds.model.criterio.CriterioPorAntiguedad;
import ar.edu.utn.frba.dds.model.fuente.Conexion;
import ar.edu.utn.frba.dds.model.fuente.FuenteDemo;
import java.net.MalformedURLException;
import java.net.URL;
import java.time.LocalDate;
import java.time.LocalDateTime;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;

/*
class FuenteDemoTest {
  private Conexion conexion;
  private FuenteDemo fuenteDemo;

  @BeforeEach
  void iniciarConexion() throws MalformedURLException {
    conexion = mock(Conexion.class);
    fuenteDemo = new FuenteDemo(conexion, new URL("http://hola"));
  }

  private Map<String, Object> crearMapConHechoDePrueba() {
    Map<String, Object> rta = new HashMap<>();
    rta.put("titulo", "demolicion");
    rta.put("descripcion", "demolicion de camiones");
    rta.put("categoria", "INCENDIO_FORESTAL");
    rta.put("latitud", "10.0");
    rta.put("longitud", "20.0");
    rta.put("fechaHecho", LocalDate.now());
    rta.put("fechaCarga", LocalDate.now());
    return rta;
  }

  @Test
  void agregarHechoAFuenteDemo() throws MalformedURLException {
    Map<String, Object> rta = crearMapConHechoDePrueba();
    //when(conexion.siguienteHecho(eq(new URL("http://hola")), any(LocalDateTime.class))).thenReturn(rta).thenReturn(null);
    when(conexion.siguienteHecho(any(URL.class), any(LocalDateTime.class)))
      .thenReturn(rta)
      .thenReturn(null);

    fuenteDemo.actualizarHechos();

    assertEquals(1, fuenteDemo.obtenerHechos().size());
  }

  @Test
  void nosDevuelteNullSiguienteHecho() throws MalformedURLException {
    when(conexion.siguienteHecho(eq(new URL("http://hola")), any(LocalDateTime.class))).thenReturn(null);

    fuenteDemo.actualizarHechos();

    assertEquals(0, fuenteDemo.obtenerHechos().size());
  }

  @DisplayName("Como persona usuaria, quiero poder obtener todos los hechos de una fuente proxy demo configurada en una colección, con un nivel de antigüedad máximo de una hora")
  @Test
  void obtenerHechosDeColeccionConFuenteDemo() throws MalformedURLException {
    Map<String, Object> hechoReciente = new HashMap<>();
    hechoReciente.put("titulo", "Hecho Reciente");
    hechoReciente.put("descripcion", "desc");
    hechoReciente.put("categoria", "INCENDIO_FORESTAL");
    hechoReciente.put("latitud", "1.0");
    hechoReciente.put("longitud", "2.0");
    hechoReciente.put("fechaHecho", LocalDate.now());
    hechoReciente.put("fechaCarga", LocalDate.now());

    Map<String, Object> hechoViejo = new HashMap<>();
    hechoViejo.put("titulo", "Hecho Viejo");
    hechoViejo.put("descripcion", "desc");
    hechoViejo.put("categoria", "INCENDIO_FORESTAL");
    hechoViejo.put("latitud", "1.0");
    hechoViejo.put("longitud", "2.0");
    hechoViejo.put("fechaHecho", LocalDate.now().minusDays(2));
    hechoViejo.put("fechaCarga", LocalDate.now().minusDays(2));

    when(conexion.siguienteHecho(eq(new URL("http://hola")), any(LocalDateTime.class)))
        .thenReturn(hechoReciente)
        .thenReturn(hechoViejo)
        .thenReturn(null);

    fuenteDemo.actualizarHechos();

    Criterio criterio = new CriterioPorAntiguedad(LocalDateTime.now(), 1);

    Coleccion coleccion = new Coleccion("handle",
      "titulo",
      "descripcion",
      fuenteDemo,
      List.of(criterio),
      AlgoritmoConsenso.CONSENSO_ABSOLUTO);

    assertEquals(1, coleccion.getHechos().size());
  }
}

 */
