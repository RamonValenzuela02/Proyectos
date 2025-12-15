package ar.edu.utn.frba.dds.model.fuente;

import ar.edu.utn.frba.dds.model.criterio.Categoria;
import ar.edu.utn.frba.dds.model.Hecho.Hecho;
import java.net.URL;
import java.time.LocalDateTime;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import javax.persistence.Column;
import javax.persistence.Embedded;
import javax.persistence.Entity;
import javax.persistence.Transient;

@Entity
public class FuenteDemo extends  Fuente {
  @Embedded
  private Conexion conexion;
  @Transient
  private URL url;
  @Column
  private LocalDateTime fechaUltimaConsulta;
  @Transient
  private List<Hecho> hechosDemo;

  protected FuenteDemo(){};
  public FuenteDemo(Conexion conexion, URL url) {
    this.conexion = conexion;
    this.url = url;
    this.fechaUltimaConsulta = LocalDateTime.now();
    this.hechosDemo = new ArrayList<>();
    //RepoFuentesDelSistema.getInstance().agregarFuente(this);
  }

  @Override
  public void actualizarHechos() {
    Map<String, Object> datosHecho = conexion.siguienteHecho(url, fechaUltimaConsulta);
    while (datosHecho != null) {
      Hecho hecho = mapearHecho(datosHecho);
      hechosDemo.add(hecho);
      fechaUltimaConsulta = LocalDateTime.now();
      datosHecho = conexion.siguienteHecho(url, fechaUltimaConsulta);
    }
  }

  private Hecho mapearHecho(Map<String, Object> datos) {
    String titulo = (String) datos.get("titulo");
    String descripcion = (String) datos.get("descripcion");
    Categoria categoria = Categoria.fromString((String) datos.get("categoria"));
    String latitud = (String) datos.get("latitud");
    String longitud = (String) datos.get("longitud");
    LocalDateTime fechaHecho = (LocalDateTime) datos.get("fechaHecho");

    return new Hecho(titulo, descripcion, categoria, latitud, longitud, fechaHecho);
  }

  public List<Hecho> obtenerHechos() {
    return hechosDemo;
  }
}