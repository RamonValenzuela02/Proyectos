package ar.edu.utn.frba.dds.model.ubicacion;

public enum Provincia {

    BUENOS_AIRES("Buenos Aires"),
    CIUDAD_AUTONOMA_DE_BUENOS_AIRES("Ciudad Autonoma de Buenos Aires"),
    CATAMARCA("Catamarca"),
    CHACO("Chaco"),
    CHUBUT("Chubut"),
    CORDOBA("Cordoba"),
    CORRIENTES("Corrientes"),
    ENTRE_RIOS("Entre Rios"),
    FORMOSA("Formosa"),
    JUJUY("Jujuy"),
    LA_PAMPA("La Pampa"),
    LA_RIOJA("La Rioja"),
    MENDOZA("Mendoza"),
    MISIONES("Misiones"),
    NEUQUEN("Neuquen"),
    RIO_NEGRO("Rio Negro"),
    SALTA("Salta"),
    SAN_JUAN("San Juan"),
    SAN_LUIS("San Luis"),
    SANTA_CRUZ("Santa Cruz"),
    SANTA_FE("Santa Fe"),
    SANTIAGO_DEL_ESTERO("Santiago del Estero"),
    TIERRA_DEL_FUEGO("Tierra del Fuego"),
    TUCUMAN("Tucuman");

    private final String nombre;

    Provincia(String nombre) {
        this.nombre = nombre;
    }

    public String getNombre() {
        return nombre;
    }

    //para handlebars
    @Override
    public String toString() {
        return nombre;
    }

    private static String normalizar(String texto) {
        if (texto == null) return "";

        texto = texto.trim();

        texto = texto
                .replace("á", "a").replace("Á", "A")
                .replace("é", "e").replace("É", "E")
                .replace("í", "i").replace("Í", "I")
                .replace("ó", "o").replace("Ó", "O")
                .replace("ú", "u").replace("Ú", "U");

        texto = texto.replace(" ", "_");
        texto = texto.toUpperCase();
        return texto;
    }

    public static Provincia desdeTexto(String texto) {
        String provinciaToEnum = normalizar(texto);
        try {
            return Provincia.valueOf(provinciaToEnum);
        } catch (IllegalArgumentException e) {
            throw new RuntimeException("No se encontró la provincia: " + texto);
        }
    }
}
