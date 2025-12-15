  package ar.edu.utn.frba.dds.model.fuente;

  import ar.edu.utn.frba.dds.model.Api;
  import ar.edu.utn.frba.dds.model.criterio.Criterio;
  import ar.edu.utn.frba.dds.model.Hecho.Hecho;
  import ar.edu.utn.frba.dds.model.solicitud.SolicitudDeEliminacion;
  import java.util.List;
  import java.util.Map;
  import java.util.Optional;
  import javax.persistence.Entity;
  import javax.persistence.Transient;

  @Entity
  public class FuenteMetaMapa extends Fuente {
    @Transient
    private Api api;

    public FuenteMetaMapa(Api api) {
      this.api = api;
      //RepoFuentesDelSistema.getInstance().agregarFuente(this);
    }

    protected FuenteMetaMapa() {};

    // Implementación del metodo abstracto de Fuente.
    // Devuelve todos los hechos sin ningún criterio (GET a /hecho)
    @Override
    public List<Hecho> obtenerHechos() {
      return api.getHechos("hecho", Optional.empty());
    }

    // Obtiene hechos pertenecientes a una colección específica, con un criterio opcional
    public List<Hecho> obtenerHechosDeColeccion(String idColeccion, Optional<List<Criterio>> criterios) {
      // Arma el endpoint dinámicamente con el id de la colección
      String endpoint = "colecciones/" + idColeccion + "/hechos";
      // Llama a la API con ese endpoint y el criterio si existe
      return api.getHechos(endpoint, criterios);
    }

    // Envía una solicitud de eliminación de un hecho a la API (POST a /solicitudes)
    public void enviarSolicitudDeEliminacion(SolicitudDeEliminacion solicitud) {
      // Arma un JSON con los datos de la solicitud y lo envía por POST
      api.postJson("solicitudes", Map.of(
              "tituloHecho", solicitud.getHecho().getTitulo(),
              "motivo", solicitud.getMotivo()
      ));
    }
  }
