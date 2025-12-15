#ifndef INICIALIZACION_H_
#define INICIALIZACION_H_

#include "gestor_kernel.h"

void iniciarKernel();
void cargarConfigs();
//finalizacion----
void finalizar_kernel();
void destruir_proceso_esperado_io(void* proceso_wait);
void destruir_io_conectada(void* io_conec);
void destruir_pcb(void* proceso);
void destruir_cpu_conectada(void* cpu);
void finalizar_kernel();
#endif