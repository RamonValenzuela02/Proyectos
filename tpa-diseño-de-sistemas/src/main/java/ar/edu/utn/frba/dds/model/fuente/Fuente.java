package ar.edu.utn.frba.dds.model.fuente;

import ar.edu.utn.frba.dds.model.Hecho.Hecho;
import java.util.List;
import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.GeneratedValue;
import javax.persistence.Id;
import javax.persistence.Inheritance;
import javax.persistence.InheritanceType;
import lombok.Getter;
import lombok.Setter;

@Entity
@Inheritance(strategy = InheritanceType.JOINED) //TODO single table
public abstract class Fuente {
  @Id
  @GeneratedValue
  @Getter
  @Setter
  private Long id;
  @Column
  @Getter
  @Setter
  private String nombre;

  public abstract List<Hecho> obtenerHechos();

  //es para la fuenteDemo pero no se me ocurre otra cosa
  public void actualizarHechos() {}

  public Hecho obtenerHechoConId(int hechoId) {
    return obtenerHechos().
      stream().
      filter(h -> h.getId() == hechoId)
      .findFirst()
      .orElse(null);
  }
}