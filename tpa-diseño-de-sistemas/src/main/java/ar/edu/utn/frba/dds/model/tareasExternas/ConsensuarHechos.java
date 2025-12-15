package ar.edu.utn.frba.dds.model.tareasExternas;

import ar.edu.utn.frba.dds.model.coleccion.Coleccion;
import ar.edu.utn.frba.dds.repositorios.RepoDeColecciones;


public class ConsensuarHechos {

  public static void main(String[] args) {
     System.setProperty("hibernate.connection.url",
      System.getenv("DB_URL"));

  System.setProperty("hibernate.connection.username",
      System.getenv("DB_USER"));

  System.setProperty("hibernate.connection.password",
      System.getenv("DB_PASSWORD"));
    RepoDeColecciones repo = RepoDeColecciones.getInstance();
    repo.getColecciones().forEach(Coleccion::actualizarHechosConsensuados);
  }

}
