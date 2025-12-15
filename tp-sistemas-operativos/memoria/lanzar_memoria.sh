#!/bin/bash

# Verificar argumentos
if [ $# -ne 2 ]; then
  echo "Uso: $0 <NRO_PRUEBA> <NRO_CONFIG>"
  echo "Ejemplo: $0 1 1  → ejecuta: ./bin/memoria configs/prueba1/memoria1.config"
  exit 1
fi

NRO_PRUEBA="$1"
NRO_CONFIG="$2"

CONFIG_PATH="configs/prueba${NRO_PRUEBA}/memoria${NRO_CONFIG}.config"

# Verificar existencia del archivo
if [ ! -f "$CONFIG_PATH" ]; then
  echo "❌ No se encontró el archivo: $CONFIG_PATH"
  exit 2
fi

# Ejecutar Memoria
./bin/memoria "$CONFIG_PATH"
