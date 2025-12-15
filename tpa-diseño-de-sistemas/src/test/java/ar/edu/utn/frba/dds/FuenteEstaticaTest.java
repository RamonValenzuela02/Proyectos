package ar.edu.utn.frba.dds;

import ar.edu.utn.frba.dds.model.criterio.Categoria;
import ar.edu.utn.frba.dds.model.fuente.FuenteEstatica;
import ar.edu.utn.frba.dds.model.Hecho.Hecho;
import java.time.LocalDateTime;
import org.junit.jupiter.api.Test;

import java.util.List;

import static org.junit.jupiter.api.Assertions.*;

public class FuenteEstaticaTest {

  @Test
  public void lecturaCorrectaDeArchivoCSV() {

    FuenteEstatica fuenteEstatica = new FuenteEstatica("src/test/resources/prueba.csv");
    List<Hecho> hechos = fuenteEstatica.obtenerHechos();

    assertEquals(4, hechos.size());

    Hecho hecho1 = hechos.get(0);
    assertEquals("Incendio Forestal en Bariloche", hecho1.getTitulo());
    assertEquals("Hubo un incendio en Bariloche", hecho1.getDescripcion());
    assertEquals(Categoria.INCENDIO_FORESTAL, hecho1.getCategoria());
    assertEquals("11.2222", hecho1.getUbicacion().getLatitud());
    assertEquals("33.4444", hecho1.getUbicacion().getLongitud());
    assertEquals(LocalDateTime.parse("2025-04-05T15:30:45"), hecho1.getFechaHecho());

    Hecho hecho2 = hechos.get(1);
    assertEquals("Accidente Fatal en General Paz", hecho2.getTitulo());
    assertEquals("Accidente fatal en General Paz. Hubo 3 victimas.", hecho2.getDescripcion());
    assertEquals(Categoria.ACCIDENTE_VIAL, hecho2.getCategoria());
    assertEquals("55.6666", hecho2.getUbicacion().getLatitud());
    assertEquals("77.8888", hecho2.getUbicacion().getLongitud());
    assertEquals(LocalDateTime.parse("2025-04-05T15:30:45"), hecho2.getFechaHecho());

    Hecho hecho3 = hechos.get(2);
    assertEquals("Incendio Forestal en Peru", hecho3.getTitulo());
    assertEquals("Hubo un incendio en la capital", hecho3.getDescripcion());
    assertEquals(Categoria.INCENDIO_FORESTAL, hecho3.getCategoria());
    assertEquals("55.6666", hecho3.getUbicacion().getLatitud());
    assertEquals("77.8888", hecho3.getUbicacion().getLongitud());
    assertEquals(LocalDateTime.parse("2025-05-07T15:30:45"), hecho3.getFechaHecho());

    Hecho hecho4 = hechos.get(3);
    assertEquals("Incendio Forestal en Cordoba", hecho4.getTitulo());
    assertEquals("Hubo un incendio en Cordoba", hecho4.getDescripcion());
    assertEquals(Categoria.INCENDIO_FORESTAL, hecho4.getCategoria());
    assertEquals("55.6666", hecho4.getUbicacion().getLatitud());
    assertEquals("98.5521", hecho4.getUbicacion().getLongitud());
    assertEquals(LocalDateTime.parse("2025-04-05T15:30:45"), hecho4.getFechaHecho());
  }

}


