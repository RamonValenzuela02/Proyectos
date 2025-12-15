package ar.edu.utn.frba.dds.model.consenso;

import ar.edu.utn.frba.dds.model.Hecho.Hecho;
import ar.edu.utn.frba.dds.model.fuente.Fuente;
import ar.edu.utn.frba.dds.repositorios.RepoFuentesDelSistema;
import java.util.List;


public enum AlgoritmoConsenso {
    CONSENSO_ABSOLUTO {
        @Override
        public boolean estaConsensuado(Hecho hecho) {
            List<Fuente> fuentes = RepoFuentesDelSistema.getInstance().obtenerFuentes();
            return fuentes.stream().allMatch(f -> f.obtenerHechos().contains(hecho));
        }
    },
    CONSENSO_MAYORIA_SIEMPLE{
        @Override
        public boolean estaConsensuado(Hecho hecho){
            List<Fuente> fuentes =RepoFuentesDelSistema.getInstance().obtenerFuentes();
            long cantidadMenciones = fuentes.stream()
              .filter(f -> f.obtenerHechos().contains(hecho)).count();

            return  cantidadMenciones >= (fuentes.size() / 2);
        }
    },
    CONSENSO_MULTIPLES_MENSIONES{
        @Override
        public boolean estaConsensuado(Hecho hecho){
            List<Fuente> fuentes = RepoFuentesDelSistema.getInstance().obtenerFuentes();
            long cantidadMenciones = fuentes.stream()
              .filter(f -> f.obtenerHechos().contains(hecho)).count();

            boolean existenVersionesDistintasDeUnHecho = fuentes.stream()
              .flatMap(f -> f.obtenerHechos().stream())
              .filter(h -> h.getTitulo().equals(hecho.getTitulo()))
              .allMatch(h -> h.equals(hecho));

            return cantidadMenciones >= 2 && !existenVersionesDistintasDeUnHecho;
        }
    };
    public abstract boolean estaConsensuado(Hecho hecho);
    public String toString() {
      return switch (this) {
        case CONSENSO_ABSOLUTO -> "CONSENSO_ABSOLUTO";
        case CONSENSO_MAYORIA_SIEMPLE -> "CONSENSO_MAYORIA_SIEMPLE";
        case CONSENSO_MULTIPLES_MENSIONES -> "CONSENSO_MULTIPLES_MENSIONES";
        default -> "NO HAY ALGORITMO DE CONSENSO ASOCIADO";
      };
    }
}
