package ar.edu.utn.frba.dds.model.exportarCSV;

import ar.edu.utn.frba.dds.model.solicitud.DetectorDeSpam;
import ar.edu.utn.frba.dds.repositorios.RepoSolicitudesDeEliminacion;
import com.opencsv.CSVWriter;

import java.io.FileWriter;
import java.io.IOException;

public class ExportarCSVSpam {

    private final RepoSolicitudesDeEliminacion repoSolicitudes;
    private final DetectorDeSpam detector;

    public ExportarCSVSpam(RepoSolicitudesDeEliminacion repoSolicitudes,  DetectorDeSpam detector) {
        this.repoSolicitudes = repoSolicitudes;
        this.detector = detector;
    };

    public void exportarCantidadSolicitudesDeSpam() {
        long cantidad = repoSolicitudes.getSolicitudes().stream()
                .filter(s -> detector.esSpam(s.getMotivo()))
                .count();

        try (FileWriter fileWriter = new FileWriter("cantidad_solicitudes_spam.csv");
             CSVWriter writer = new CSVWriter(fileWriter)) {

            writer.writeNext(new String[]{"Cantidad_Total_Spam"});
            writer.writeNext(new String[]{String.valueOf(cantidad)});

        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
