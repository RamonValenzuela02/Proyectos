package ar.edu.utn.frba.dds.model.coleccion;

import java.util.ArrayList;
import java.util.List;
import java.util.stream.Collectors;
import ar.edu.utn.frba.dds.model.consenso.AlgoritmoConsenso;
import ar.edu.utn.frba.dds.model.criterio.Criterio;
import ar.edu.utn.frba.dds.model.Hecho.Hecho;
import ar.edu.utn.frba.dds.model.fuente.Fuente;
import javax.persistence.CascadeType;
import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.EnumType;
import javax.persistence.Enumerated;
import javax.persistence.GeneratedValue;
import javax.persistence.Id;
import javax.persistence.ManyToMany;
import javax.persistence.ManyToOne;

import ar.edu.utn.frba.dds.model.ubicacion.Provincia;
import lombok.Getter;
import lombok.Setter;

/**
 * Representa una colección de hechos basada en un criterio y una fuente.
 */
@Entity
public class Coleccion {
  @Id
  @GeneratedValue
  @Getter
  private Long id;
  @Getter
  private String handle;
  @Column
  @Getter
  @Setter
  private String titulo;
  @Column
  @Getter
  @Setter
  private String descripcion;
  @ManyToOne
  //JoinColumn me parece que iria
  @Getter
  @Setter
  private Fuente fuente;
  @ManyToMany(cascade = CascadeType.ALL) //TODO
  @Getter
  private List<Criterio> criterios = new ArrayList<>();
  @Enumerated(EnumType.STRING)
  @Getter
  @Setter
  private AlgoritmoConsenso algoritmoConsenso;
  @ManyToMany
  @Getter
  @Setter
  private List<Hecho> hechosConsensuados = new ArrayList<>(); //TODO

  protected Coleccion(){};

  public Coleccion(String handle, String titulo, String descripcion, Fuente fuente, List<Criterio> criterios, AlgoritmoConsenso algoritmoConsenso) {
    this.handle = handle; //es un alias que se le da a una coleccion que sirve para identificarla cuando la exponemos por API REST
    this.titulo = titulo;
    this.descripcion = descripcion;
    this.fuente = fuente;
    this.criterios = criterios;
    this.algoritmoConsenso = algoritmoConsenso;
  }

  public List<Hecho> getHechos() {
    return fuente.obtenerHechos().stream()
      .filter(hecho -> !hecho.estaEliminado())
      .filter(this::cumpleTodosLosCriterios)
      .collect(Collectors.toList());
  }

  public List<Hecho> getHechosPorProvincia(Provincia provincia) {
    return fuente.obtenerHechos().stream()
            .filter(hecho -> !hecho.estaEliminado())
            .filter(hecho -> hecho.getProvincia().equals(provincia))
            .collect(Collectors.toList());
  }

  private boolean cumpleTodosLosCriterios(Hecho hecho) {
    return criterios.stream().allMatch(criterio -> criterio.cumpleCriterio(hecho));
  }

  private void actualizarConsensuado(Hecho hecho) {
    if (algoritmoConsenso != null) {
      if(algoritmoConsenso.estaConsensuado(hecho)){
        hechosConsensuados.add(hecho);
      }
    }
  }

  public List<Hecho> getHechosConNavegacion(ModoNavegacion modoNavegacion) {
    if(modoNavegacion==ModoNavegacion.IRRESTRICTA){
      return getHechos();
    }else{
      return hechosConsensuados;
    }
  }

  public void agregarCriterio(Criterio criterio) {
    criterios.add(criterio);
  }

  public void borrarCriterios(){
    criterios.clear();
  }
  public void actualizarHechosConsensuados() {
    fuente.obtenerHechos().forEach(this::actualizarConsensuado);
  }

}