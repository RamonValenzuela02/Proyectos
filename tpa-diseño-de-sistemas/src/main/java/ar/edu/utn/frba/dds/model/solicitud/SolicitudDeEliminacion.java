package ar.edu.utn.frba.dds.model.solicitud;

import static java.util.Objects.requireNonNull;

import ar.edu.utn.frba.dds.model.Hecho.Hecho;
import ar.edu.utn.frba.dds.model.fuente.Fuente;

import javax.persistence.*;

import lombok.Getter;

import java.time.LocalDateTime;

@Entity
public class SolicitudDeEliminacion {
  @Id
  @GeneratedValue
  @Getter
  private Long id;
  @ManyToOne
  @Getter
  private Hecho hecho;
  @Column(length = 2000)
  @Getter
  private String motivo;
  @Enumerated(EnumType.STRING)
  @Getter
  private Estado estado;
  @Getter
  private LocalDateTime fechaSolicitud;
  @OneToOne
  private Fuente fuente;

  protected SolicitudDeEliminacion() {};

  public SolicitudDeEliminacion(Hecho hecho, String motivo) {
    this.hecho = hecho;
    this.motivo = motivo;
    this.estado = Estado.PENDIENTE;
    this.fechaSolicitud = LocalDateTime.now();
    validarSolicitud();
  }

  private void validarSolicitud() {
    requireNonNull(hecho);
    requireNonNull(motivo);
  }

  public void aceptar() {
    this.estado = Estado.ACEPTADA;
  }

  public void rechazar() {
    this.estado = Estado.RECHAZADA;
  }

  public void pendiente() { this.estado = Estado.PENDIENTE; }

}