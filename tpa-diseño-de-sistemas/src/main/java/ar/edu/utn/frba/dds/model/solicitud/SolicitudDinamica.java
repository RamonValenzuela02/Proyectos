package ar.edu.utn.frba.dds.model.solicitud;

import ar.edu.utn.frba.dds.model.Hecho.Hecho;
import ar.edu.utn.frba.dds.model.criterio.Categoria;
import ar.edu.utn.frba.dds.repositorios.RepoHechosDinamicos;
import java.time.LocalDateTime;
import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.EnumType;
import javax.persistence.Enumerated;
import javax.persistence.GeneratedValue;
import javax.persistence.Id;
import lombok.Setter;

@Entity
public class SolicitudDinamica {
  @Id
  @GeneratedValue
  private Long id;
  @Column
  private String titulo;
  @Column
  private String descripcion;
  @Enumerated(EnumType.STRING)
  private Categoria categoria;
  @Column
  private String latitud;
  @Column
  private String longitud;
  @Column
  private LocalDateTime fechaHecho;
  @Column
  private LocalDateTime fechaCarga;
  @Enumerated(EnumType.STRING)
  private EstadoSolicitudDinamica estado;
  @Column
  @Setter
  private String sugerencia;

  public SolicitudDinamica(String titulo, String descripcion, Categoria categoria, String latitud, String longitud, LocalDateTime fechaHecho, LocalDateTime fechaCarga) {
    this.titulo = titulo;
    this.descripcion = descripcion;
    this.categoria = categoria;
    this.latitud = latitud;
    this.longitud = longitud;
    this.fechaHecho = fechaHecho;
    this.fechaCarga = fechaCarga;
    this.estado = EstadoSolicitudDinamica.PENDIENTE;
  }

  protected SolicitudDinamica() {};

  public void aceptar() {
    this.estado = EstadoSolicitudDinamica.ACEPTADA;
    Hecho hecho = new Hecho(titulo,descripcion,categoria,latitud,longitud,fechaHecho);
    RepoHechosDinamicos.getInstance().agregarHecho(hecho);
  }

  public void aceptarConSugerencia(String sugerencia) {
    this.sugerencia = sugerencia;
    this.estado = EstadoSolicitudDinamica.ACEPTADA_CON_SUGERENCIA;
    Hecho hecho = new Hecho(titulo,descripcion,categoria,latitud,longitud,fechaHecho);
    RepoHechosDinamicos.getInstance().agregarHecho(hecho);
  }

  public void rechazar() {
    this.estado = EstadoSolicitudDinamica.RECHAZADA;
  }

}