
package ar.edu.utn.frba.dds.model.criterio;

import ar.edu.utn.frba.dds.model.Hecho.Hecho;
import java.time.LocalDateTime;
import javax.persistence.Column;
import javax.persistence.DiscriminatorValue;
import javax.persistence.Entity;
import lombok.Getter;

@Entity
@Getter
@DiscriminatorValue("FechaHecho")
public class CriterioPorRangoFecha extends Criterio {
  @Column
  private LocalDateTime fechaDesde;
  @Column
  private LocalDateTime fechaHasta;

  public CriterioPorRangoFecha(String fechaDesde, String fechaHasta) {
    this.fechaDesde = LocalDateTime.parse(fechaDesde);
    this.fechaHasta = LocalDateTime.parse(fechaHasta);
  }

  public CriterioPorRangoFecha() {};

  @Override
  public Boolean cumpleCriterio(Hecho hecho) {
    return !(hecho.getFechaHecho().isAfter(fechaHasta)) && !(hecho.getFechaHecho().isBefore(fechaDesde));
  }
}
