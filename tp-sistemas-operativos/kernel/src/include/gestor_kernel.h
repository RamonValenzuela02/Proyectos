
#ifndef GESTOR_KERNEL_H_
#define GESTOR_KERNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include<readline/readline.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <commons/temporal.h>


//#include "../../utils/src/utils/include/conexiones.h"

#include "../../../utils/src/utils/include/conexiones.h" 

extern t_log* kernel_logger;
// extern t_log* kernel_logger;
extern t_config* kernel_config;

extern int fd_memoria;
extern int fd_server_dispatch;
extern int fd_server_interrupt;
extern int fd_server_io;

extern char* IP_MEMORIA;
extern char* ALGORITMO_CORTO_PLAZO;
extern char* PUERTO_MEMORIA;
extern int TIEMPO_SUSPENSION;
extern char* LOG_LEVEL;
extern char* PUERTO_ESCUCHA_DISPATCH;
extern char* PUERTO_ESCUCHA_INTERRUPT;
extern char* PUERTO_ESCUCHA_IO;
extern char* ALGORITMO_INGRESO_A_READY;
extern double ALFA;
extern int ESTIMACION_INICIAL;

typedef enum {
    NEW,       
    READY,   
    EXEC,      
    BLOCKED,   
    EXIT, 
    SUSP_BLOCKED,
    SUSP_READY,     
    CANT_ESTADOS
} EstadoProceso;
typedef enum{ //lo agrego para no suspender un proceso si solo se bloqueo por un dump memory
    IO,
    MEMORY_DUMP
}motivo_blocked;
typedef struct{
    int id;
    char* archivo_en_pseudoCodigo; // prueba.txt
    int peso_pseudocodigo;
    int pc;
    int metricas_estado[CANT_ESTADOS];
    double metricas_tiempo[CANT_ESTADOS];
    struct timespec tiempo_entrada_a_estado;
    double estimacion_proxima_rafaga;
    double tiempo_antes_de_syscall;
    double tiempo_restante_rafaga;
    double tiempo_real_ejecutado;
    bool desalojado;
    t_temporal* cronometro;
    sem_t sem_cancelar_suspension;
    sem_t sem_activar_suspension; //provisorio
    pthread_mutex_t mutex_esta_siendo_suspendido;
    pthread_mutex_t mutex_pc;
    motivo_blocked motivoDeBloqueo;
}pcb_t;
typedef struct{
    char* nombre_io;
    int fd;
    //nuevo
    bool ocupado;
    //t_list* procesos_esperando_io;   
    pcb_t* proceso_actual;                            
    pthread_mutex_t mutex_io;         
}io_conectada;

typedef struct{
    char* nombre_io;
    double tiempo_espera;
    pcb_t* proceso;
}proceso_esperando_io;

typedef enum{
    LIBRE,
    OCUPADA
}estados_cpu;

typedef struct{
    int id;
    int fd_dispatch;
    int fd_interrupt;
    int id_proceso;
    estados_cpu estado;
}cpu_conectada;



extern int id_pcb_generador;
extern bool se_agoto_memoria_alguna_vez;
extern double ultima_rafaga; 

//seniales 
extern volatile sig_atomic_t terminar;


//listas 
extern t_list* procesos_new;
extern t_list* procesos_ready;
extern t_list* procesos_exit; 
extern t_list* procesos_exec;
extern t_list* procesos_blocked; 
extern t_list* procesos_esperando_ios;  
extern t_list* ios_conectadas;
extern t_list* cpus_conectadas;
extern t_list* procesos_blocked_suspend;
extern t_list* procesos_ready_suspend;


//mutex y semaforos 
extern sem_t sem_iniciar_planificacion;
extern pthread_mutex_t mutex_lista_new;
extern pthread_mutex_t mutex_lista_ready;
extern pthread_mutex_t mutex_id_pcb_generador;
extern pthread_mutex_t mutex_agotamiento_memoria;
extern sem_t espacio_para_proceso;
extern sem_t sem_proceso_bloqueado_por_io; 
extern pthread_mutex_t mutex_procesos_esperando_ios; 
extern pthread_mutex_t mutex_ios_conectadas; 
extern sem_t sem_hay_cpus_disponibles;    
extern sem_t sem_hay_procesos_en_ready;  
extern pthread_mutex_t mutex_lista_exec;  
extern pthread_mutex_t mutex_cpus_conectadas; 
extern pthread_mutex_t mutex_lista_exit; 
extern pthread_mutex_t mutex_lista_blocked; 
extern sem_t sem_hay_procesos_esp_ser_ini_en_mem;
extern sem_t sem_hay_procesos_en_exit; 
extern sem_t se_conecta_cpu;
extern pthread_mutex_t mutex_lista_blocked_suspend;
extern pthread_mutex_t mutex_lista_ready_suspend;
extern sem_t esperoAqueSeConectenTodasLasCpus;
extern pthread_mutex_t mutex_conexion_memoria_nuevo;
// extern int seNecesitaSeguirPreguntandoAMem;



#endif