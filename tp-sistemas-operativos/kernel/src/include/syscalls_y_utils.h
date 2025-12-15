#ifndef SYSCALLS_Y_UTILS_H_
#define SYSCALLS_Y_UTILS_H_

#include "gestor_kernel.h"

//SYSCALLS 
void SYSCALL_INIT_PROC(char* archivo_en_pseudoCodigo, int tamanio_proceso);
void SISCALL_EXIT(pcb_t* proceso);
//
void SISCALL_IO(pcb_t* proceso, char* nombre_io, double miliSegundos);
void* SISCALL_DUMP_MEMORY(void* proceso);
//EXTRAS PARA SYSCALLS
bool comparador_proceso_mas_chico(void *pcb_a, void* pcb_b);
bool es_valida_io(char* nombre_io);
io_conectada* find_by_name_(t_list* ios_conectadas, char* nombre);
double calculo_tiempo_proceso_en_estado(pcb_t* proceso);

//funcionalidades de cambio entre listas 
void pasar_proceso_de_estado_a_otro(pcb_t* proceso, EstadoProceso estado_inicial,EstadoProceso estado_final);
void sacar_proceso_de_estado(pcb_t* proceso,EstadoProceso estado);
void agregar_proceso_a_estado(pcb_t* proceso,EstadoProceso estado);
void agregar_proceso_a_new(pcb_t* proceso);
void agregar_proceso_a_susp_ready(pcb_t* proceso);

//MANEJO DE SENIALES 
void manejar_ctrl_c(int sig);

// Funcionalidad de algoritmos SJF y SRT
double calcular_estimacion_proxima_rafaga(pcb_t* proceso);
bool hay_cpu_libre();
void desalojar_proceso(pcb_t* pid);
void mandar_mensaje_por_interrupt_a_cpu(cpu_conectada* cpu, int pid);
pcb_t* proceso_largo_en_exec();
void verificarAlgoritmoSRT();
bool cpu_no_disponible();

//planificador mediano plazo 
void* manejador_de_cambio_de_procesos_a_disco(void * proceso_bloqueado);
bool sigue_proceso_bloqueado(pcb_t* proceso_bloc);
void mensaje_para_memoria(int id, op_code codigo);
void cancelar_timer_de_supension(pcb_t* proceso_actual);
bool esta_suspendido(pcb_t* proceso_actual);

//para el planificardor largo plazo
void* espera_rta_cpu(void* arg);
pcb_t* buscar_proceso_en_exec_con_pid(int pid);
// pcb_t* buscar_proceso_en_blocked_con_pid(int pid);
pcb_t* buscar_proceso_en_bloqued_con_pid(int pid);
pcb_t* buscar_proceso_en_ready_con_pid(int pid);
void actualizar_pc_de_proceso(pcb_t* proceso_en_exec,int pc);
void actualizar_pc_de_proceso_bloc(pcb_t* proceso_en_exec,int pc);
void actualizar_pc_de_proceso_ready(pcb_t* proceso_en_ready,int pc);

const char* nombre_estado(EstadoProceso estado);
pthread_mutex_t* obtener_mutex_estado(EstadoProceso estado);
t_list** obtener_lista_estado(EstadoProceso estado);
void pasar_proceso_de_estado_a_otro(pcb_t* proceso, EstadoProceso origen, EstadoProceso destino);
void ordenar_por_menor_estimacion();

#endif