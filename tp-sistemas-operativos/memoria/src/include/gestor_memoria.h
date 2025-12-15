#ifndef MEMORIA_GESTOR_H_
#define MEMORIA_GESTOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <commons/log.h>
#include <commons/config.h>
#include<readline/readline.h>

#include "../../utils/src/utils/include/conexiones.h"

#include "tabla.h"


extern int fd_server;


extern t_log* mem_logger;
extern t_log* logger_debug;
extern t_config* mem_config;

extern char* PUERTO_MEMORIA ;
extern int TAM_MEMORIA ;
extern int TAM_PAGINA ; 
extern int ENTRADAS_POR_TABLA ;  
extern int RETARDO_MEMORIA ; 
extern char* PATH_SWAP ; 
extern int RETARDO_SWAP ; 
extern char* LOG_LEVEL ; 
extern char* DUMP_PATH ;
extern int CANT_NIVELES;
extern char* PATH_INSTRUCCIONES;

extern void* memoria_usuario;
extern int cant_procesosMem;
extern t_list* lista_procesos;
extern t_tabla* tabla_pag_nivel1;
extern int TAM_OCUPADO;
extern int CANT_FRAMES;
extern int numTablaCreada;


extern pthread_mutex_t mutex_pc;
extern pthread_mutex_t mutex_pid;
extern pthread_mutex_t mutex_contador;
extern pthread_mutex_t mutex_add_lista;
extern pthread_mutex_t mutex_calculoTamMem;
extern pthread_mutex_t mutex_remove_lista;
extern pthread_mutex_t mutex_get_listaProcesos;
extern pthread_mutex_t mutex_get_listaInstrucciones;
extern pthread_mutex_t mutex_entradas_swap;
extern pthread_mutex_t mutex_sacar_liberar_espacio_suspencion;
extern pthread_mutex_t mutex_lista_paginas_proceso;
extern pthread_mutex_t hayProcesosSuspen;
extern pthread_mutex_t mutex_send_ok_kernel;

extern pthread_mutex_t send_ok_cpu;
extern pthread_mutex_t mutex_leer_pagina;
extern pthread_mutex_t mutex_actualizar_pagina;
extern pthread_mutex_t escribir_valor;
extern pthread_mutex_t solicitar_valor;
extern pthread_mutex_t pedir_marco;
extern pthread_mutex_t pedir_tabla;
extern pthread_mutex_t mutex_buscando_num_pag_global;

extern sem_t proceso_escribiendo;
extern sem_t modificar_tamanio_ocupado_en_memoria;

extern pthread_mutex_t mutex_conexion_con_kernel;



#endif