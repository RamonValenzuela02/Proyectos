#ifndef IO_H_
#define IO_H_


#include "gestor_io.h"

t_log* io_logger;
t_log* debug_logger;
t_config* io_config;

int fd_conexion_kernel;

volatile sig_atomic_t apagar_io = 0;  

char* IP_KERNEL;
char* PUERTO_KERNEL;
char* LOG_LEVEL;

char* nombre;

void iniciarIo();
void cargarConfigs();
void enviar_handshake();
void* esperar_peticiones_kernel();
void manejar_sigint(int senial);
void cerrar_conexiones();
void liberar_recursos();
#endif