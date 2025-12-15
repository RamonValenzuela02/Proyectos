package ar.edu.utn.frba.dds.model.tareasExternas;

import ar.edu.utn.frba.dds.model.fuente.Fuente;
import ar.edu.utn.frba.dds.repositorios.RepoFuentesDelSistema;

public class ActualizarFuenteDemo {

  public static void main(String[] args) {
     System.setProperty("hibernate.connection.url",
      System.getenv("DB_URL"));

    System.setProperty("hibernate.connection.username",
      System.getenv("DB_USER"));

    System.setProperty("hibernate.connection.password",
      System.getenv("DB_PASSWORD"));
    
    RepoFuentesDelSistema repoFuente = RepoFuentesDelSistema.getInstance();
    repoFuente.obtenerFuentes().forEach(Fuente::actualizarHechos);
  }

}
