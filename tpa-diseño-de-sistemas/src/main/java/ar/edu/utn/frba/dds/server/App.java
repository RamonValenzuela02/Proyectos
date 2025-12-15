package ar.edu.utn.frba.dds.server;

import ar.edu.utn.frba.dds.scripts.Bootstrap;

public class App {
  public static void main(String[] args) {
    //new Bootstrap().init();
    System.setProperty("hibernate.connection.url",
      System.getenv("DB_URL"));

  System.setProperty("hibernate.connection.username",
      System.getenv("DB_USER"));

  System.setProperty("hibernate.connection.password",
      System.getenv("DB_PASSWORD"));
    new Server().start();

  }
}
