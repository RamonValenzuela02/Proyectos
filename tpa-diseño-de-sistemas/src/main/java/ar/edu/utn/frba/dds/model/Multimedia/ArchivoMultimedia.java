package ar.edu.utn.frba.dds.model.Multimedia;

import ar.edu.utn.frba.dds.model.Hecho.Hecho;
import lombok.Getter;
import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.GeneratedValue;
import javax.persistence.Id;
import javax.persistence.JoinColumn;
import javax.persistence.ManyToOne;

@Entity
public class ArchivoMultimedia {
  @Id
  @GeneratedValue
  @Getter
  private Long id;
  @Getter
  @Column(nullable = false)
  private String nombreArchivo;
  @Getter
  @Column(nullable = false)
  private String rutaArchivo; // ruta relativa o absoluta del archivo en el sistema
  @Getter
  @ManyToOne
  @JoinColumn(name = "hecho_id")
  private Hecho hecho;

  public ArchivoMultimedia() {}

  public ArchivoMultimedia(String nombreArchivo, String rutaArchivo, Hecho hecho) {
    this.nombreArchivo = nombreArchivo;
    this.rutaArchivo = rutaArchivo;
    this.hecho = hecho;
  }

}
