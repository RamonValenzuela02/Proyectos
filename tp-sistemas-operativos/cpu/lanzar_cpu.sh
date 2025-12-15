#!/bin/bash

# Verificación de argumentos
if [ $# -ne 3 ]; then
  echo "Uso: $0 <ID_CPU> <NRO_PRUEBA> <NRO_CONFIG>"
  echo "Ejemplo: $0 1 1 1  → ejecuta: ./bin/cpu 1 configs/prueba1/cpu1.config"
  exit 1
fi

ID_CPU="$1"
NRO_PRUEBA="$2"
NRO_CONFIG="$3"

CONFIG_PATH="configs/prueba${NRO_PRUEBA}/cpu${NRO_CONFIG}.config"

# Validación del archivo de configuración
if [ ! -f "$CONFIG_PATH" ]; then
  echo "No se encontró el archivo: $CONFIG_PATH"
  exit 2
fi

# Ejecutar CPU
./bin/cpu "$ID_CPU" "$CONFIG_PATH"
