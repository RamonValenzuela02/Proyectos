package ar.edu.utn.frba.dds;

import static org.junit.jupiter.api.Assertions.*;
import static org.mockito.Mockito.*;

import ar.edu.utn.frba.dds.*;
import java.time.LocalDate;
import java.util.List;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

public class FuenteMetaMapaTest{
  /*
    private FuenteMetaMapa fuenteMock;

    @BeforeEach
    public void setup() {

        fuenteMock = mock(FuenteMetaMapa.class);

        Hecho hecho1 = new Hecho(
            "Hecho 1", "Descripcion 1", Categoria.INCENDIO_FORESTAL,
            "-34.61", "-58.38",
            LocalDate.of(2025, 6, 1), LocalDate.of(2025, 6, 2)
        );

        Hecho hecho2 = new Hecho(
            "Hecho 2", "Descripcion 2", Categoria.INCENDIO_FORESTAL,
            "-34.60", "-58.37",
            LocalDate.of(2025, 5, 15), LocalDate.of(2025, 5, 20)
        );

        Criterio criterio = new CriterioCumplidorSiempre();
        List<Criterio> criterios = List.of(criterio);

        FuenteMetaMapa fuente = new FuenteMetaMapa();

      Coleccion coleccion = new ColeccionBuilder()
          .conHandle("ak1fjd1")
          .conTitulo("Incendios")
          .conDescripcion("Incendios en el norte")
          .conFuente(fuente)
          .conCriterios(criterios)
          .conModoNavegacion(ModoNavegacion.IRRESTRICTA)
          .crear();

    }

    @Test
    public void testObtenerHechosDevuelveListaDesdeFuente() {
        List<Hecho> hechos = coleccion.();

        // Verificamos que la lista tenga los hechos mockeados
        assertEquals(2, hechos.size());
        assertEquals("Hecho 1", hechos.get(0).getTitulo());
        assertEquals("Hecho 2", hechos.get(1).getTitulo());
    }

   */
}
