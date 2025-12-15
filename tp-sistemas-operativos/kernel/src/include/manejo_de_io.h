#ifndef MANEJO_DE_IO_H_
#define MANEJO_DE_IO_H_

#include "syscalls_y_utils.h"

void* conexion_con_io();
void* esperar_rta_io(void* arg);

//FUNCIONALIDADES EXTRAS PARA EL FUNCIONAMIENTO DEL CODIGO
void crear_msj_y_mandarselo_a_io(int fd, int id, double tiempo);
int recibir_msj_de_io(int fd);
void liberar_io(io_conectada* io);
void encolar_proceso_a_io(proceso_esperando_io* proceso);
void forzar_exit_a_todos_los_bloqueados(char* io);
void liberar_proceso_esperando_io(proceso_esperando_io* proceso_esperando);
bool esta_ocupada_io(char* nombre_io);
void descargasBuffer(int fd);
//bool esta_suspendido(pcb_t* proceso_actual);
io_conectada* buscar_io_libre_con_nombre(char* nombre );
io_conectada* buscar_io_libre(t_list* ios_seleccionadas);
//void cancelar_timer_de_supension(pcb_t* proceso_actual);
bool no_hay_mas_ios_con_mismo_nombre(char* nombre);
proceso_esperando_io* tomar_proximo_req_de_io(char* nombre_io_sol);
io_conectada* busco_si_hay_io_libre_para_req();
bool tiene_requerimiento(char* nombre);
#endif