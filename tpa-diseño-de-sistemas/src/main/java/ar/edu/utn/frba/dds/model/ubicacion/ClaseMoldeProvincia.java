package ar.edu.utn.frba.dds.model.ubicacion;


public class ClaseMoldeProvincia {
  private Address address;

  public Address getAddress() {
    return address;
  }
  public static class Address {
    private String state;
    private String country;

    public String getState() {
      return state;
    }
    public String getCountry() {
      return country;
    }
  }
}
