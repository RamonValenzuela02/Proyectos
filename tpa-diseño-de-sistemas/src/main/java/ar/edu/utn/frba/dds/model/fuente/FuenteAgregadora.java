package ar.edu.utn.frba.dds.model.fuente;

import ar.edu.utn.frba.dds.model.Hecho.Hecho;
import java.util.*;
import java.util.stream.Collectors;
import javax.persistence.Entity;
import javax.persistence.ManyToMany;

@Entity
public class FuenteAgregadora extends Fuente {
  @ManyToMany
  private List<Hecho> hechos = new ArrayList<>();
  @ManyToMany
  private List<Fuente> fuentesAgregadas;

  public FuenteAgregadora() {
    this.fuentesAgregadas = new ArrayList<>();
  }

  public void agregarFuente(Fuente fuente) {
    if (!this.fuentesAgregadas.contains(fuente)) {
      this.fuentesAgregadas.add(fuente);
    }
  }

  public void removerFuente(Fuente fuente) {
    this.fuentesAgregadas.remove(fuente);
  }

  @Override
  public List<Hecho> obtenerHechos() {
    hechos = this.fuentesAgregadas.stream()
        .flatMap(fuente -> fuente.obtenerHechos().stream())
        .collect(Collectors.toList());

    return  hechos;
  }

}
