package ar.edu.utn.frba.dds.model.ubicacion;

import java.io.IOException;
import okhttp3.OkHttpClient;
import retrofit2.Call;
import retrofit2.Response;
import retrofit2.Retrofit;
import retrofit2.converter.gson.GsonConverterFactory;

public class ServicioProvincia {
  private static ServicioProvincia instancia;
  private static final String urlApi = "https://nominatim.openstreetmap.org/";
  private Retrofit retrofit;
  private final provinciaService service;

  private ServicioProvincia() {
    OkHttpClient client = new OkHttpClient.Builder()
      .addInterceptor(new UserAgentInterceptor())
      .build();

    this.retrofit = new Retrofit.Builder()
      .baseUrl(urlApi)
      .client(client)
      .addConverterFactory(GsonConverterFactory.create())
      .build();

    this.service = retrofit.create(provinciaService.class);
  }

  public static ServicioProvincia getInstancia() {
    if (instancia == null) {
      instancia = new ServicioProvincia();
    }
    return instancia;
  }

  public String provincia(String latitud, String longitud) {

    try {
      Call<ClaseMoldeProvincia> call = service.getProvincia(latitud, longitud, "json",1);


      System.out.println("URL -> " + call.request().url()); // una solicitud por hecho, merjo tenerlas en cache


      Response<ClaseMoldeProvincia> response = call.execute();
      if (response.isSuccessful()) {
        return response.body().getAddress().getState();
      } else {
        throw new RuntimeException("Error en Api: " + response.message());
      }
    } catch (IOException e) {
      throw new RuntimeException("Error en Api: " + e.getMessage());
    }
  }

}
