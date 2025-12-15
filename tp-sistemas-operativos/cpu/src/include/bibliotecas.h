#ifndef BIBLIOTECAS_H
#define BIBLIOTECAS_H

// #include "cpu.h"
#include "../../utils/src/utils/include/conexiones.h"


typedef enum TipoInstruccion {
    NOOP,
    WRITE,
    READ,
    GOTO,
    IO,
    INIT_PROC,
    INST_DUMP_MEMORY,
    EXIT,
} e_tipo_instruccion;

typedef struct {
    int valor;
    int direccion_fisica;
    int tamanio;
    int tiempo;
    int direccion_logica;
    char* dispositivo;
    char* archivo;
    char* datos;
} t_argumentos_genericos;

typedef struct {
    char *operacion;
    t_argumentos_genericos *args;
} t_instruccion;

typedef struct {
    int pid;
    int pc;
} t_proceso;




extern t_proceso* proceso;
extern t_instruccion* instruccion;


#endif
