#ifndef GESTOR_IO_H_
#define GESTOR_IO_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <commons/log.h>
#include <commons/config.h>

#include "../../utils/src/utils/include/conexiones.h"

extern t_log* io_logger;
extern t_log* debug_logger;
extern t_config* io_config;

extern int fd_conexion_kernel;

extern char* IP_KERNEL;
extern char* PUERTO_KERNEL;
extern char* LOG_LEVEL;

extern char* nombre;

#endif
