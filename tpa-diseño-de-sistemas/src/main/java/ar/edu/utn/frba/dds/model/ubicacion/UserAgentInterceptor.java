package ar.edu.utn.frba.dds.model.ubicacion;

import java.io.IOException;
import okhttp3.Interceptor;
import okhttp3.Request;
import okhttp3.Response;

public class UserAgentInterceptor implements Interceptor {
  @Override
  public Response intercept(Chain chain) throws IOException{
    Request originalRequest = chain.request();
    Request requestWithUserAgent = originalRequest.newBuilder()
      .header("User-Agent", "MiApp/1.0 (ra02valenzuela@gmail.com)")
      .build();
    return chain.proceed(requestWithUserAgent);
  }
}
