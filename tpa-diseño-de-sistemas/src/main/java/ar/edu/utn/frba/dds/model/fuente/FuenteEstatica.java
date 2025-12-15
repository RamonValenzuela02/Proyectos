package ar.edu.utn.frba.dds.model.fuente;

import ar.edu.utn.frba.dds.model.criterio.Categoria;
import ar.edu.utn.frba.dds.model.Hecho.Hecho;
import com.opencsv.exceptions.CsvValidationException;
import java.io.FileReader;
import java.io.IOException;
import java.time.LocalDateTime;
import java.util.ArrayList;
import java.util.List;
import com.opencsv.CSVReader;
import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.Transient;

/**
 * clase que representa la carga de los hechos estaticos de un determinado path.
 */
@Entity
public class FuenteEstatica extends Fuente {
  @Transient
  List<Hecho> hechos = new ArrayList<>();
  @Column
  private String path;

  public FuenteEstatica(String path) {
    this.path = path;
    //RepoFuentesDelSistema.getInstance().agregarFuente(this);
  }

  protected FuenteEstatica(){};

  public List<Hecho> obtenerHechos() {
    try (CSVReader reader = new CSVReader(new FileReader(path))) {
      String[] fila;
      while ((fila = reader.readNext()) != null) {
        String titulo = fila[0];
        String descripcion = fila[1];
        Categoria categoria = Categoria.valueOf(fila[2]);
        String latitud = fila[3];
        String longitud = fila[4];
        LocalDateTime fecha = LocalDateTime.parse(fila[5]);

        Hecho hecho = new Hecho(titulo, descripcion, categoria, latitud, longitud, fecha);
        agregarHechoNuevo(hecho, hechos);
      }
    } catch (IOException | CsvValidationException e) {
      e.printStackTrace();
    }
    return hechos;
  }

  private void agregarHechoNuevo(Hecho hecho, List<Hecho> hechos) {
    hechos.removeIf(h -> h.getTitulo().equalsIgnoreCase(hecho.getTitulo()));
    hechos.add(hecho);
  }
}