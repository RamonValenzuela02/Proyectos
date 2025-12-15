#ifndef CPU_GESTOR_H_
#define CPU_GESTOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <commons/log.h>
#include <commons/config.h>

#include "../../utils/src/utils/include/conexiones.h"


extern int fd_conexion_dispatch;
extern int fd_conexion_interrupt;
extern int fd_conexion_memoria;

extern t_log* cpu_logger;
extern t_log* logger_debug;
extern t_config* cpu_config;

extern char* IP_MEMORIA;
extern char*PUERTO_MEMORIA;
extern char* PUERTO_DISPATCH; //le cmabie el nombre porq era escucha (y es cliente)
extern char* PUERTO_INTERRUPT;  //le cambie el nombre porq era escucha(y es cliente )
extern int ENTRADAS_TLB;
extern char* REEMPLAZO_TLB;
extern int ENTRADAS_CACHE;
extern char* REEMPLAZO_CACHE;
extern int RETARDO_CACHE;
extern char* LOG_LEVEL;
extern char* IP_KERNEL;
extern int TAM_PAGINA;

// extern t_proceso* proceso_anterior;
// extern t_proceso* proceso;
extern pthread_mutex_t semaforo_pc;
extern t_list* cache;
extern t_list* tlb;



#endif