#ifndef CONEXIONES_CON_MODULOS_H_
#define CONEXIONES_CON_MODULOS_H_

#include "syscalls_y_utils.h"


void* esperar_ios();
void* conexiones_cpus();
void* consola_interactiva();
int recibir_identificador_cpu(int fd_conexion);

//estas dos funciones las hice para ver como van quedando las listas de cpu_conectadas y ios_conectadas 
void conectados_ios();
void conectados_cpus();

#endif
