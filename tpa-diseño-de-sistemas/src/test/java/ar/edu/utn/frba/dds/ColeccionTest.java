package ar.edu.utn.frba.dds;
import ar.edu.utn.frba.dds.model.criterio.Categoria;
import ar.edu.utn.frba.dds.model.coleccion.Coleccion;
import ar.edu.utn.frba.dds.model.coleccion.ColeccionBuilder;
import ar.edu.utn.frba.dds.model.criterio.Criterio;
import ar.edu.utn.frba.dds.model.criterio.CriterioCumplidorSiempre;
import ar.edu.utn.frba.dds.model.criterio.CriterioPorCategoria;
import ar.edu.utn.frba.dds.model.fuente.FuenteDinamica;
import ar.edu.utn.frba.dds.model.fuente.FuenteEstatica;
import ar.edu.utn.frba.dds.model.Hecho.Hecho;
import java.time.LocalDateTime;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;


import static org.junit.jupiter.api.Assertions.*;
import static org.mockito.Mockito.mock;

import java.util.List;

public class ColeccionTest {

  @Test
  @DisplayName("Como persona administradora, deseo crear una colección")
  public void crearColeccion() {
    Criterio criterio = new CriterioCumplidorSiempre();
    List<Criterio> criterios = List.of(criterio);

    FuenteEstatica fuente = new FuenteEstatica("src/test/resources/prueba.csv");

    Coleccion coleccion = new ColeccionBuilder()
        .conHandle("ak1fjd1")
        .conTitulo("Incendios")
        .conDescripcion("Incendios en el norte")
        .conFuente(fuente)
        .conCriterios(criterios)
        .crear();

    assertEquals(4, coleccion.getHechos().size());
  }

  @DisplayName("Como persona visualizadora, deseo navegar todos los hechos disponibles de una colección, con algun filtro.")
  @Test
  public void cantidadDeIncendiosForestalesEs3() {

    Criterio criterio = new CriterioPorCategoria(Categoria.INCENDIO_FORESTAL);
    List<Criterio> criterios = List.of(criterio);

    FuenteEstatica fuente = new FuenteEstatica("src/test/resources/prueba.csv");

    Coleccion coleccion = new ColeccionBuilder()
        .conHandle("ak1fjd1")
        .conTitulo("Incendios")
        .conDescripcion("Incendios en el norte")
        .conFuente(fuente)
        .conCriterios(criterios)
        .crear();

    assertEquals(3, coleccion.getHechos().size());
  }
  @Test
  public void noSeMuestranHechosEliminadosEnUnaColeccion() {

    Criterio criterio = new CriterioPorCategoria(Categoria.INCENDIO_FORESTAL);
    List<Criterio> criterios = List.of(criterio);

    FuenteDinamica fuente = new FuenteDinamica();
    fuente.agregarHecho("habia una vez",
      "holaholhola",
      Categoria.INCENDIO_FORESTAL,
      "1234",
      "5678",
      LocalDateTime.now(),
      LocalDateTime.now());

    Coleccion coleccion = new ColeccionBuilder()
            .conHandle("ak1fjd1")
            .conTitulo("Para test")
            .conDescripcion("No se muestran hechos eliminados en una coleccion")
            .conFuente(fuente)
            .conCriterios(criterios)
            .crear();

    List<Hecho> hechosColeccion = coleccion.getHechos();

    assertTrue(hechosColeccion.isEmpty());
  }
/*
  @Test
  public void navegacionEnModoCuradoSoloDevuelveHechosConsensuados() {
    Hecho hecho1 = new Hecho(
            "Incendio Forestal",
            "Incendio en Bariloche",
            Categoria.INCENDIO_FORESTAL,
            "10.0",
            "20.0",
            LocalDate.of(2025, 6, 30),
            LocalDate.of(2025, 6, 30));

    Hecho hecho2 = new Hecho(
            "Incendio Forestal",
            "Incendio Misiones",
            Categoria.INCENDIO_FORESTAL,
            "10.0",
            "20.0",
            LocalDate.of(2025, 6, 30),
            LocalDate.of(2025, 6, 30));

    Fuente fuenteMock = mock(Fuente.class);
    when(fuenteMock.obtenerHechos()).thenReturn(List.of(hecho1, hecho2));

    AlgoritmoConsenso consensoMock = mock(AlgoritmoConsenso.class);
    when(consensoMock.estaConsensuado(hecho1, List.of(fuenteMock))).thenReturn(true);
    when(consensoMock.estaConsensuado(hecho2, List.of(fuenteMock))).thenReturn(false);

    Coleccion coleccion = new ColeccionBuilder()
            .conHandle("test")
            .conTitulo("Para Test")
            .conDescripcion("Test Navegación Curada")
            .conFuente(fuenteMock)
            .conModoNavegacion(ModoNavegacion.CURADA)
            .conAlgoritmoConsenso(consensoMock)
            .crear();

    List<Hecho> hechosCurados = coleccion.getHechos();

    assertEquals(1, hechosCurados.size());
    assertTrue(hechosCurados.contains(hecho1));
    assertFalse(hechosCurados.contains(hecho2));
  }
*/
  /*
  @Test
  public void navegacionEnModoIrrestrictaDevuelveTodosLosHechos() {
    Hecho hecho1 = new Hecho(
            "Incendio Forestal",
            "Incendio en Bariloche",
            Categoria.INCENDIO_FORESTAL,
            "10.0",
            "20.0",
            LocalDateTime.of(2025, 6, 30,17,0,0,0),
            LocalDateTime.of(2025, 6, 30,17,0,0,0));

    Hecho hecho2 = new Hecho(
            "Incendio Forestal",
            "Incendio Misiones",
            Categoria.INCENDIO_FORESTAL,
            "10.0",
            "20.0",
            LocalDateTime.of(2025, 6, 30,17,0,0,0),
            LocalDateTime.of(2025, 6, 30,17,0,0,0));

    Fuente fuenteMock = mock(Fuente.class);
    when(fuenteMock.obtenerHechos()).thenReturn(List.of(hecho1, hecho2));

    AlgoritmoConsenso consensoMock = mock(AlgoritmoConsenso.class); // no se usará

    Coleccion coleccion = new ColeccionBuilder()
            .conHandle("test")
            .conTitulo("Para test")
            .conDescripcion("Test Navegacion Irrestricta")
            .conFuente(fuenteMock)
            .conAlgoritmoConsenso(consensoMock)
            .crear();

    List<Hecho> hechos = coleccion.getHechos();

    assertEquals(2, hechos.size());
    assertTrue(hechos.contains(hecho1));
    assertTrue(hechos.contains(hecho2));
  }

   */
}

