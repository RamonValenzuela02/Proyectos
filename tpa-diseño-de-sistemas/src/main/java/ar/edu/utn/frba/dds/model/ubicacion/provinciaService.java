package ar.edu.utn.frba.dds.model.ubicacion;

import retrofit2.Call;
import retrofit2.http.GET;
import retrofit2.http.Query;

public interface provinciaService {
  @GET("reverse")
  Call<ClaseMoldeProvincia> getProvincia(
    @Query("lat") String latitud,
    @Query("lon") String longitud,
    @Query("format") String format,
    @Query("addressdetails") int details);
}
