package ar.edu.utn.frba.dds.model.criterio;


import ar.edu.utn.frba.dds.model.Hecho.Hecho;
import javax.persistence.Column;
import javax.persistence.DiscriminatorValue;
import lombok.Getter;

@Getter
@DiscriminatorValue("Ubicacion")
public class CriterioPorUbicacion extends Criterio {
    @Column
    private final String latitud;
    @Column
    private final String longitud;

    public CriterioPorUbicacion(String latitud, String longitud) {
        this.latitud = latitud;
        this.longitud = longitud;
    }
    @Override
    public Boolean cumpleCriterio(Hecho hecho) {
        return hecho.getUbicacion().getLatitud().equals(latitud) &&
                hecho.getUbicacion().getLongitud().equals(longitud);
    }
}