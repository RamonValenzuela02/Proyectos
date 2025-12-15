#ifndef PLANIFICACION_LARGO_PLAZO_H_
#define PLANIFICACION_LARGO_PLAZO_H_

#include "syscalls_y_utils.h"

 
// int seNecesitaSeguirPreguntandoAMem;
void* planificador_largo_plazo();
void* new_a_ready_PLANI_LARGO_PLAZO();
bool lista_new_vacia();
bool lista_susp_ready_vacia();
bool hay_espacio_para_proceso(int id, int peso);
void enviar_path_a_memoria(char* archivo_en_pseudoCodigo, int id_proceso, int tamanio_proceso);
pcb_t* tomar_proceso_proximo_para_memoria(bool* fue_de_suspendido);
void enviar_deSuspenderProceso(int id);

//FINALIZACION
void* exit_a_fin_PLANI_LARGO_PLAZO();
void loguear_metricas(pcb_t* proceso);
void liberar_pcb(pcb_t* proceso);
void notificar_finalizacion_proceso_memoria_y_esperar_confirmacion(pcb_t* proceso);



#endif