package ar.edu.utn.frba.dds.model.solicitud;

public interface DetectorDeSpam {
  boolean esSpam(String texto);
}