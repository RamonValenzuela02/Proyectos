#ifndef KERNEL_H_
#define KERNEL_H_

#include "syscalls_y_utils.h"
#include "inicializacion.h"
#include "planificacion_largo_plazo.h"
#include "manejo_de_io.h"
#include "planificador_corto_plazo.h"
#include "conexiones_con_modulos.h"

t_log* kernel_logger;
// t_log* kernel_logger;
t_config* kernel_config;

int fd_memoria;
int fd_server_dispatch;
int fd_server_interrupt;
int fd_server_io;

char* IP_MEMORIA;
char* ALGORITMO_CORTO_PLAZO;
char*  PUERTO_MEMORIA;
int TIEMPO_SUSPENSION;
char* LOG_LEVEL;
char* PUERTO_ESCUCHA_DISPATCH;
char* PUERTO_ESCUCHA_INTERRUPT;
char* PUERTO_ESCUCHA_IO;
char* ALGORITMO_INGRESO_A_READY;
double ALFA;
int ESTIMACION_INICIAL;



int id_pcb_generador;
bool se_agoto_memoria_alguna_vez;
double ultima_rafaga;

//seniales 
volatile sig_atomic_t terminar;


//listas 
t_list* procesos_new;
t_list* procesos_ready;
t_list* procesos_exec;
t_list* procesos_exit;
t_list* procesos_blocked;
t_list* procesos_blocked_suspend;
t_list* procesos_ready_suspend;

t_list* ios_conectadas;
t_list* cpus_conectadas;
t_list* procesos_esperando_ios;


//mutex y semaforos 
sem_t sem_iniciar_planificacion;
pthread_mutex_t mutex_lista_new;
pthread_mutex_t mutex_lista_ready;
pthread_mutex_t mutex_lista_exec;
pthread_mutex_t mutex_lista_blocked;
pthread_mutex_t mutex_lista_ready_suspend;
pthread_mutex_t mutex_lista_blocked_suspend;
pthread_mutex_t mutex_lista_exit;



pthread_mutex_t mutex_id_pcb_generador;
pthread_mutex_t mutex_agotamiento_memoria;
sem_t espacio_para_proceso;
sem_t sem_proceso_bloqueado_por_io; 
pthread_mutex_t mutex_procesos_esperando_ios; 
pthread_mutex_t mutex_ios_conectadas; 
sem_t sem_hay_cpus_disponibles;   
sem_t sem_hay_procesos_en_ready;  
pthread_mutex_t mutex_lista_exec; 
pthread_mutex_t mutex_cpus_conectadas; 
pthread_mutex_t mutex_lista_exit;  
pthread_mutex_t mutex_lista_blocked; 
sem_t sem_hay_procesos_esp_ser_ini_en_mem;
sem_t sem_hay_procesos_en_exit;
sem_t se_conecta_cpu;
sem_t esperoAqueSeConectenTodasLasCpus;

pthread_mutex_t mutex_actualizar_pc;

pthread_mutex_t mutex_conexion_memoria_nuevo;



//FUNCIONES 
void* esperar_ios();
void* conexiones_cpus();
void* consola_interactiva();
int recibir_identificador_cpu(int fd_conexion);
void conectados_ios(); 
void conectados_cpus();
#endif