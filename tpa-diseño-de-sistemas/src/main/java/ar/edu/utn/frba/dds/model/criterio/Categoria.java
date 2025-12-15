package ar.edu.utn.frba.dds.model.criterio;

/**
 * Representa la categoria de un Hecho. Sirve para filtrar en una fuente de datos.
 */

public enum Categoria {
  INCENDIO_FORESTAL,
  HOMICIDOS_DOLOSOS,
  ACCIDENTE_VIAL;

  public static Categoria fromString(String valor) {
    for (Categoria c : Categoria.values()) {
      if (c.name().equalsIgnoreCase(valor)) {
        return c;
      }
    }
    throw new IllegalArgumentException("Categoría inválida: " + valor);
  }
}