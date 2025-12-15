
#ifndef CICLO_INSTRUCCIONES_H_
#define CICLO_INSTRUCCIONES_H_

#include "bibliotecas.h"
#include "gestor_cpu.h"
// #include "cpu.h"
#include "mmu.h"

//ciclo de instruccion
void fetch();
bool decode();
void execute();



void recibirProceso();
void enviar_contexto_a_kernel();
void enviar_kernel_io(char* dispositivo,int tiempo);
void enviar_kernel_init_process(char* dispositivo,int tiempo);
void enviar_kernel_dump_memory(int pid);
char* recibirInstruccionesAMemoria(int pid, int pc);
void enviar_kernel_exit(int pid, int pc);
void solicitarinstruccionAMemoria(int pid, int pc);
void liberar_ejecucion();
void atender_cpu_dispatch();
void liberar_instruccion();
void ejecutar_proceso();
void* atender_cpu_interrupt(void* args);
bool checkInterrupt();
void enviar_paginas_modificadas_a_memoria(int pid);
void enviar_valor_a_memoria(int direccion_fisica, char* valor, int pid_de_pagina_a_actualizar);
// void enviar_valor_a_memoria(int direccion_fisica, char* valor);
//void enviar_paginas_modificadas_a_memoria(uint32_t pid);
const char* op_codeAstring(e_tipo_instruccion operacion);
void asignarParametros(t_instruccion* instruccion, t_buffer* buffer);
bool mandar_syscall_a_ker(char* syscall, t_buffer* buffer);
void incializar_proceso_anterior();
void liberarInstruccionAnterior();

extern t_proceso* proceso;
extern t_proceso* proceso_anterior;
extern t_instruccion* instrucciones;
extern int direccion_fisica;
extern char* valor_leido_memoria;
extern pthread_mutex_t semaforo_pc;

#endif
