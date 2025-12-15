#ifndef PLANIFICACION_CORTO_PLAZO_H_
#define PLANIFICACION_CORTO_PLAZO_H_

#include "syscalls_y_utils.h"


void* planificacion_corto();

///////FUNCIONALIDADES PARA QUE FUNCIONE EL PLANIFICADOR 
cpu_conectada* buscar_cpu_libre();
void mandar_mensaje_a_cpu(cpu_conectada* cpu, int pid,int pc);
//void* espera_rta_cpu(void* arg);
//pcb_t* buscar_proceso_en_exec_con_pid(int pid);
pcb_t* buscar_proximo_proceso();
//void actualizar_pc_de_proceso(pcb_t* proceso_en_exec,int pc);
//void recibir_contexto_de_proceso(cpu_conectada* arg);


// FUNCIONALIDADES PARA SJF Y SRT

pcb_t* buscar_proceso_con_menor_estimacion();
void* menor_estimacion(void* a, void* b);
//pcb_t* proceso_largo_en_exec();
pcb_t* buscar_proceso_con_mayor_estimacion();
//void cambiar_estimacion(void* ptr);
void* mayor_estimacion(void* a, void* b);
//bool hay_cpu_libre();
//void desalojar_proceso(int pid);
//void mandar_mensaje_por_interrupt_a_cpu(cpu_conectada* cpu, int pid);

#endif