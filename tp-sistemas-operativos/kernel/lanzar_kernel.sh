#!/bin/bash

# Verificación de argumentos
if [ $# -ne 4 ]; then
  echo "Uso: $0 <NOMBRE_PRUEBA> <TAMANIO_ARCHIVO> <NRO_PRUEBA> <NRO_CONFIG>"
  echo "Ejemplo: $0 PLANI_LYM_PLAZO 15 1 1"
  exit 1
fi

NOMBRE_PRUEBA="$1"
TAMANIO_ARCHIVO="$2"
NRO_PRUEBA="$3"
NRO_CONFIG="$4"

# Ruta al archivo de configuración
CONFIG_PATH="configs/prueba${NRO_PRUEBA}/kernel${NRO_CONFIG}.config"

# Verificar que el archivo exista
if [ ! -f "$CONFIG_PATH" ]; then
  echo "No se encontró el archivo: $CONFIG_PATH"
  exit 2
fi

# Ejecutar el kernel con los argumentos correctos
./bin/kernel "$NOMBRE_PRUEBA" "$TAMANIO_ARCHIVO" "$CONFIG_PATH"
