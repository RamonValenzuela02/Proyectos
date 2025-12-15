package ar.edu.utn.frba.dds.model.criterio;

import ar.edu.utn.frba.dds.model.Hecho.Hecho;
import java.time.LocalDateTime;
import javax.persistence.Column;
import javax.persistence.DiscriminatorValue;
import javax.persistence.Entity;
import lombok.Getter;

@Entity
@Getter
@DiscriminatorValue("FechaCarga")
public class CriterioPorFechaCarga extends Criterio {
  @Column
  private LocalDateTime fechaDesde;
  @Column
  private LocalDateTime fechaHasta;

  public CriterioPorFechaCarga(String fechaFiltro, String fechaDesde, String fechaHasta) {
    this.fechaDesde = LocalDateTime.parse(fechaDesde);
    this.fechaHasta = LocalDateTime.parse(fechaHasta);
  }

  public CriterioPorFechaCarga() {};

  @Override
  public Boolean cumpleCriterio(Hecho hecho) {
    return !(hecho.getFechaCarga().isAfter(fechaHasta)) && !(hecho.getFechaCarga().isBefore(fechaDesde));
  }
}