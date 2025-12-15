#ifndef CPU_H_
#define CPU_H_

#include "gestor_cpu.h"
#include <commons/string.h>
#include "ciclo_instruccion.h"






// pthread_t hilo_interrupt;

void iniciarCpu(int identificador);
void cargarConfigs();
// void* espera_peticiones_kernel_dispatch();
// void* espera_peticiones_kernel_interrupt();

void finalizar_cpu();
void cerrar_conexiones_cpu();
void manejar_sigint(int senial);
void liberarEntrada_tlb(void* entrada);
void liberaraEntrada_cache(void* entrada);
// void destroy_config();
extern pthread_mutex_t mutex_var_global_syscall;
extern volatile sig_atomic_t apagar_cpu;  




#endif