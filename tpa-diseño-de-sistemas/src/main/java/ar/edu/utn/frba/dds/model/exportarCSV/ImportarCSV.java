package ar.edu.utn.frba.dds.model.exportarCSV;

import ar.edu.utn.frba.dds.model.Hecho.Hecho;
import ar.edu.utn.frba.dds.model.criterio.Categoria;
import com.opencsv.CSVReader;
import com.opencsv.exceptions.CsvValidationException;

import java.io.*;
import java.time.LocalDateTime;
import java.util.ArrayList;
import java.util.List;

public class ImportarCSV {
    public ImportarCSV() {};

    public List<Hecho> importarDesdePath(String path) {
        try (Reader reader = new FileReader(path)) {
            return parsear(reader);
        } catch (IOException | CsvValidationException e) {
            throw new RuntimeException(e);
        }
    }

    public List<Hecho> importarDesdeStream(InputStream inputStream) {
        try (Reader reader = new InputStreamReader(inputStream)) {
            return parsear(reader);
        } catch (IOException | CsvValidationException e) {
            throw new RuntimeException(e);
        }
    }

    private List<Hecho> parsear(Reader reader) throws IOException, CsvValidationException {
        List<Hecho> hechos = new ArrayList<>();

        try (CSVReader csvReader = new CSVReader(reader)) {
            String[] fila;

            while ((fila = csvReader.readNext()) != null) {
                String titulo = fila[0];
                String descripcion = fila[1];
                Categoria categoria = Categoria.valueOf(fila[2].trim().toUpperCase());
                String latitud = fila[3];
                String longitud = fila[4];
                LocalDateTime fechaDelHecho = LocalDateTime.parse(fila[5]);

                Hecho hecho = new Hecho(titulo, descripcion, categoria, latitud, longitud, fechaDelHecho);
                agregarHechoNuevo(hecho, hechos);
            }
        }
        return hechos;
    }

    private void agregarHechoNuevo(Hecho hecho, List<Hecho> hechos) {
        hechos.removeIf(h -> h.getTitulo().equalsIgnoreCase(hecho.getTitulo()));
        hechos.add(hecho);
    }
}