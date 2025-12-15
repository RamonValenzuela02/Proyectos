package ar.edu.utn.frba.dds.model;

import ar.edu.utn.frba.dds.model.Hecho.Hecho;
import ar.edu.utn.frba.dds.model.criterio.Categoria;
import ar.edu.utn.frba.dds.model.criterio.Criterio;
import ar.edu.utn.frba.dds.model.criterio.CriterioPorCategoria;
import ar.edu.utn.frba.dds.model.criterio.CriterioPorFechaCarga;
import ar.edu.utn.frba.dds.model.criterio.CriterioPorRangoFecha;
import ar.edu.utn.frba.dds.model.criterio.CriterioPorUbicacion;
import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.core.type.TypeReference;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.sun.jersey.api.client.Client;
import com.sun.jersey.api.client.ClientResponse;
import com.sun.jersey.api.client.WebResource;

import java.time.LocalDateTime;
import javax.ws.rs.core.MediaType;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Optional;

// Clase que encapsula la lógica de comunicación con un servicio HTTP
public class Api {
    private final Client client;
    private final String baseUrl; // URL base del servicio externo
    private final ObjectMapper mapper; // Mapper de Jackson para convertir entre JSON y objetos Java

    // Constructor: recibe la URL base y el mapper, crea el cliente HTTP
    public Api(String baseUrl, ObjectMapper mapper) {
        this.client = Client.create();
        this.baseUrl = baseUrl;
        this.mapper = mapper;
    }

    // Realiza un GET a un endpoint específico y devuelve una lista de hechos
    public List<Hecho> getHechos(String endpoint, Optional<List<Criterio>> criterios) {
        WebResource url = client.resource(baseUrl).path(endpoint);

        // Si hay un criterio presente, se agregan los parámetros a la URL
        if (criterios.isPresent()) {
            url = agregarQueryParams(url, criterios.get());
        }

        WebResource.Builder builder = url.accept(MediaType.APPLICATION_JSON);

        // Realiza la petición GET
        ClientResponse response = builder.get(ClientResponse.class);

        if (response.getStatus() >= 400) {
            throw new RuntimeException("Error al obtener hechos: " + response.getStatus());
        }

        try {
            // Obtiene el contenido JSON de la respuesta como String
            String json = response.getEntity(String.class);

            // Deserializa el JSON en una lista de mapas (clave-valor)
            List<Map<String, Object>> datos = mapper.readValue(
                    json, new TypeReference<List<Map<String, Object>>>() {}
            );

            // Transforma cada mapa en un objeto Hecho y lo agrega a la lista
            List<Hecho> hechos = new ArrayList<>();
            for (Map<String, Object> dato : datos) {
                hechos.add(construirHechoDesdeMap(dato));
            }
            return hechos;

        } catch (JsonProcessingException e) {
            throw new RuntimeException("Error al parsear JSON de hechos", e);
        }
    }

    // Realiza un POST a un endpoint enviando un objeto serializado como JSON
    public void postJson(String endpoint, Object objeto) {
        // Construye la URL del endpoint
        WebResource url = client.resource(baseUrl).path(endpoint);

        // Configura los encabezados para enviar y recibir JSON
        WebResource.Builder builder = url.type(MediaType.APPLICATION_JSON).accept(MediaType.APPLICATION_JSON);

        // Convierte el objeto a un string JSON
        String body;
        try {
            body = mapper.writeValueAsString(objeto);
        } catch (JsonProcessingException e) {
            throw new RuntimeException("Error al serializar el objeto a JSON", e);
        }

        // Realiza la petición POST con el cuerpo JSON
        ClientResponse response = builder.post(ClientResponse.class, body);

        if (response.getStatus() != 200 && response.getStatus() != 201) {
            throw new RuntimeException("Error al hacer POST: " + response.getStatus());
        }
    }

    // Agrega parámetros de búsqueda (query params) a la URL según el tipo de criterio
    private WebResource agregarQueryParams(WebResource url, List<Criterio> criterios) {
        if (criterios instanceof CriterioPorCategoria c) {
            url = url.queryParam("categoria", c.getCategoria().toString());
        } else if (criterios instanceof CriterioPorRangoFecha c) {
            url = url.queryParam("fechaDesde", c.getFechaDesde().toString())
                    .queryParam("fechaHasta", c.getFechaHasta().toString());
        } else if (criterios instanceof CriterioPorFechaCarga c) {
            url = url.queryParam("fechaDesde", c.getFechaDesde().toString())
                    .queryParam("fechaHasta", c.getFechaHasta().toString());
        } else if (criterios instanceof CriterioPorUbicacion c) {
            url = url.queryParam("latitud", c.getLatitud())
                    .queryParam("longitud", c.getLongitud());
        }
        return url;
    } //REVISAR

    // Construye un objeto Hecho a partir de un Map<String, Object> con los datos
    private Hecho construirHechoDesdeMap(Map<String, Object> dato) {
        String titulo = (String) dato.get("titulo");
        String descripcion = (String) dato.get("descripcion");
        Categoria categoria = Categoria.valueOf((String) dato.get("categoria"));
        String latitud = (String) dato.get("latitud");
        String longitud = (String) dato.get("longitud");
        LocalDateTime fechaHecho = LocalDateTime.parse((String) dato.get("fechaHecho"));

        return new Hecho(titulo, descripcion, categoria, latitud, longitud, fechaHecho);
    }
}
