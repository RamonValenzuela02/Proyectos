package ar.edu.utn.frba.dds.model.criterio;


import ar.edu.utn.frba.dds.model.Hecho.Hecho;
import java.time.Duration;
import java.time.LocalDateTime;
import javax.persistence.Column;
import javax.persistence.DiscriminatorValue;
import javax.persistence.Entity;

@Entity
@DiscriminatorValue("Antiguedad")
public class CriterioPorAntiguedad extends Criterio {
  @Column
  private LocalDateTime  fechaReferencia;
  @Column
  private int antiguedad;

  public CriterioPorAntiguedad(LocalDateTime  fechaRefencia, int antiguedad ) {
    this.fechaReferencia = fechaRefencia;
    this.antiguedad = antiguedad;
  }

  protected CriterioPorAntiguedad() {};

  @Override
  public Boolean cumpleCriterio(Hecho hecho) {
    Duration duracion = Duration.between(hecho.getFechaCarga(), fechaReferencia);
    return duracion.toHours() <= antiguedad;
  }
}