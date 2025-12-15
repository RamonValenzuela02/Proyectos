package ar.edu.utn.frba.dds.scripts;


import ar.edu.utn.frba.dds.model.Hecho.Hecho;
import ar.edu.utn.frba.dds.model.Multimedia.ArchivoMultimedia;
import ar.edu.utn.frba.dds.model.coleccion.Coleccion;
import ar.edu.utn.frba.dds.model.coleccion.ColeccionBuilder;
import ar.edu.utn.frba.dds.model.consenso.AlgoritmoConsenso;
import ar.edu.utn.frba.dds.model.criterio.Categoria;
import ar.edu.utn.frba.dds.model.criterio.Criterio;
import ar.edu.utn.frba.dds.model.criterio.CriterioPorCategoria;
import ar.edu.utn.frba.dds.model.fuente.Fuente;
import ar.edu.utn.frba.dds.model.fuente.FuenteDemo;
import ar.edu.utn.frba.dds.model.fuente.FuenteDinamica;
import ar.edu.utn.frba.dds.model.fuente.FuenteEstatica;
import ar.edu.utn.frba.dds.model.solicitud.DetectorDeSpamBasico;
import ar.edu.utn.frba.dds.model.solicitud.ServicioDeSolicitudesEliminacion;
import ar.edu.utn.frba.dds.model.solicitud.SolicitudDeEliminacion;
import ar.edu.utn.frba.dds.model.solicitud.SolicitudDinamica;
import ar.edu.utn.frba.dds.repositorios.*;
import io.github.flbulgarelli.jpa.extras.simple.WithSimplePersistenceUnit;
import java.time.LocalDateTime;
import ar.edu.utn.frba.dds.model.Usuario.TipoUsuario;
import ar.edu.utn.frba.dds.model.Usuario.Usuario;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;


public class Bootstrap implements WithSimplePersistenceUnit {


  List<Hecho> hechos = new ArrayList<>();
  List<SolicitudDeEliminacion> solicitudes = new ArrayList<>();
  String titulo = "";
  String descripcion = "";
  Categoria categoria = null;
  String latitud = "";
  String longitud = "";

  String latCordoba = "-31.427502308472217";
  String longCordoba =  "-64.15248731961331";

  String latRioNegro = "-41.572616430443965";
  String longRioNegro = "-70.67058748910377";

  String latMisiones = "-25.66544912461599";
  String longMisiones = "-54.3568595214326";

  String latBsAs = "-34.70627880394875";
  String longBsAs = "-58.29374525297";

  String latMendoza = "-34.114778246085464";
  String longMendoza = "-68.29033825309313";

  String latSalta = "-24.83171438815388";
  String longSalta = "-65.53878627692313";

  String latNeuquen = "-37.997973139146936";
  String longNeuquen = "-70.12887302774908";

  public void init() {

    for(int i = 1; i <= 10; i++) {

      titulo = "Titulo del hecho " + i;
      descripcion = "Descripcion del hecho " + i;

      obtenerCategoria(i);
      obtenerLatitudyLongitud(i);

      Hecho hecho = new Hecho(titulo, descripcion, categoria, latitud, longitud, LocalDateTime.now());
      if(i <= 9){
        if(categoria.equals(Categoria.INCENDIO_FORESTAL)){
          hecho.agregarArchivo(new ArchivoMultimedia("incendio-cataratas.jpg","/uploads/incendio-cataratas.jpg",hecho));
          hecho.agregarArchivo(new ArchivoMultimedia("incendio-patagonia.jpg","/uploads/incendio-patagonia.jpg",hecho));
        } else if (categoria.equals(Categoria.ACCIDENTE_VIAL)) {
          hecho.agregarArchivo(new ArchivoMultimedia("accidente-vial-1.jpg","/uploads/accidente-vial-1.jpg",hecho));
          hecho.agregarArchivo(new ArchivoMultimedia("accidente-vial-2.jpg","/uploads/accidente-vial-2.jpg",hecho));
        }
      }
      hechos.add(hecho);
    }

    withTransaction(() -> {

      Fuente fuenteDinamica = new FuenteDinamica();
      fuenteDinamica.setNombre("FuenteDinamica");

      RepoHechosDinamicos repoHechosDinamicos = RepoHechosDinamicos.getInstance();
      for(Hecho h : hechos){
        repoHechosDinamicos.agregarHecho(h);
      }

      RepoFuentesDelSistema.getInstance().agregarFuente(fuenteDinamica);

      RepoDeColecciones repoDeColecciones = RepoDeColecciones.getInstance();
      Coleccion colIncendiosForestales = new ColeccionBuilder()
              .conTitulo("Incendios Forestales")
              .conHandle("incendios")
              .conDescripcion("Incendios Forestales en el pais")
              .conCriterios(List.of (new CriterioPorCategoria(Categoria.INCENDIO_FORESTAL)))
              .conAlgoritmoConsenso(AlgoritmoConsenso.CONSENSO_ABSOLUTO)
              .conFuente(fuenteDinamica)
              .crear();

      Coleccion colAccidentes = new ColeccionBuilder()
              .conTitulo("Accidentes")
              .conHandle("accidentes")
              .conDescripcion("Accidentes en el pais")
              .conCriterios(List.of (new CriterioPorCategoria(Categoria.ACCIDENTE_VIAL)))
              .conFuente(fuenteDinamica)
              .crear();

      Coleccion colHomicidiosDolosos = new ColeccionBuilder()
              .conTitulo("Homicidios Dolosos")
              .conHandle("homicidios_dolosos")
              .conDescripcion("Homicidios en el pais")
              .conCriterios(List.of (new CriterioPorCategoria(Categoria.HOMICIDOS_DOLOSOS)))
              .conFuente(fuenteDinamica)
              .crear();

      repoDeColecciones.agregarColeccion(colIncendiosForestales);
      repoDeColecciones.agregarColeccion(colAccidentes);
      repoDeColecciones.agregarColeccion(colHomicidiosDolosos);

      //SOLICITUDES DE ELIMINACION BASE
      for(int i = 1; i <= 3; i++) {
        solicitudes.add(new SolicitudDeEliminacion(hechos.get(i), "Solicitud del hecho " + i));
      }

      RepoSolicitudesDeEliminacion repoSolicitudes = RepoSolicitudesDeEliminacion.getInstance();
      for(SolicitudDeEliminacion s : solicitudes) {
        repoSolicitudes.registrarSolicituDeEliminacion(s);
      }

    });

    // usuarios
    withTransaction(() -> {
      RepoUsuarios repo = RepoUsuarios.getInstance();
      repo.agregar(new Usuario(TipoUsuario.ADMINISTRADOR, "Facundo", "Facundo"));
      repo.agregar(new Usuario(TipoUsuario.CONTRIBUYENTE, "Juan", "Juan"));
      repo.agregar(new Usuario(TipoUsuario.ADMINISTRADOR, "Mati", "Mati"));
    });

  }

  private void obtenerCategoria(int i) {
    if(i % 3 == 0){
      this.categoria = Categoria.INCENDIO_FORESTAL;
    } else if (i % 3 == 1){
      this.categoria = Categoria.ACCIDENTE_VIAL;
    }else {
      this.categoria = Categoria.HOMICIDOS_DOLOSOS;
    }
  }

  private void obtenerLatitudyLongitud(int i){
    if(i % 7 == 0){
      this.latitud = latBsAs;
      this.longitud = longBsAs;
    } else if (i % 7 == 1){
      this.latitud = latCordoba;
      this.longitud = longCordoba;
    } else if (i % 7 == 2){
      this.latitud = latMisiones;
      this.longitud = longMisiones;
    } else if (i % 7 == 3){
      this.latitud = latMendoza;
      this.longitud = longMendoza;
    }else if (i % 7 == 4){
      this.latitud = latRioNegro;
      this.longitud = longRioNegro;
    }else if (i % 7 == 5){
      this.latitud = latSalta;
      this.longitud = longSalta;
    }else {
      this.latitud = latNeuquen;
      this.longitud = longNeuquen;
    }
  }
}
