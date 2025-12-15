package ar.edu.utn.frba.dds.model.Usuario;

import lombok.Getter;
import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.EnumType;
import javax.persistence.Enumerated;
import javax.persistence.GeneratedValue;
import javax.persistence.Id;

@Entity
public class Usuario {
  @Id
  @GeneratedValue
  @Getter
  private Long id;
  @Column
  @Getter
  private String nombre;
  @Column
  @Getter
  private String password;
  @Enumerated(EnumType.STRING)
  @Getter
  private TipoUsuario tipo;

  public Usuario(TipoUsuario tipo, String nombre, String password) {
    this.tipo = tipo;
    this.nombre = nombre;
    this.password = password;
  }

  public Usuario() {

  }
}
