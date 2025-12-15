package ar.edu.utn.frba.dds.model.criterio;

import ar.edu.utn.frba.dds.model.Hecho.Hecho;
import javax.persistence.DiscriminatorValue;
import javax.persistence.Entity;

@Entity
@DiscriminatorValue("SiempreCumple")
public class CriterioCumplidorSiempre extends Criterio {
  @Override
  public Boolean cumpleCriterio(Hecho hecho) {
    return true;
  }
}