package ar.edu.utn.frba.dds.model.criterio;


import ar.edu.utn.frba.dds.model.Hecho.Hecho;

import javax.persistence.*;

import ar.edu.utn.frba.dds.model.ubicacion.Provincia;
import lombok.Getter;

@Entity
@DiscriminatorValue("Provincia")
public class CriterioPorProvincia extends Criterio {

  @Enumerated(EnumType.STRING)
  @Column
  private Provincia provincia;

  protected CriterioPorProvincia() {};
  public CriterioPorProvincia(Provincia provincia) {
    this.provincia = provincia;
  }
  @Override
  public Boolean cumpleCriterio(Hecho hecho) {
    return hecho.getProvincia() == provincia;
  }
}