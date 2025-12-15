package ar.edu.utn.frba.dds.model.solicitud;

public class DetectorDeSpamBasico implements DetectorDeSpam {
  @Override
  public boolean esSpam(String texto) {
    return false;
  }
}
