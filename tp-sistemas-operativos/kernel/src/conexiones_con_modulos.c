#include "include/conexiones_con_modulos.h"

void* esperar_ios() {
    //espero conexiones
    while(!terminar){
        int fd = esperar_cliente(fd_server_io);
        //esperar handshake inicial
        op_code code = recibir_operacion(fd);
        // log_info(kernel_logger,"se recibio el cod_op: %d",code);
        if(code == HANDSHAKE_IO_KERNEL){
            //recibimos el nombre de la io
            t_buffer* buffer = recibir_todo_un_buffer(fd);
            char* nombre_io = extraer_string_del_buffer(buffer);

            log_info(kernel_logger,"## Se conecto una IO: %s",nombre_io);

            //agregamos la io nueva a la lista de ios conectadas 

            io_conectada* io_nueva = malloc(sizeof(io_conectada));
            if (io_nueva == NULL) {
                log_error(kernel_logger, "Error al alocar memoria para nueva io en esperar_ios()");
                return NULL;
            }
            char* copia = malloc(strlen(nombre_io)+1);
            strcpy(copia,nombre_io);
            free(nombre_io);

            io_nueva->nombre_io = copia;
            io_nueva->fd = fd;
            io_nueva->ocupado =false;
            io_nueva->proceso_actual = NULL;
            //io_nueva->procesos_esperando_io=list_create();
            pthread_mutex_init(&io_nueva->mutex_io,NULL);
            
            pthread_mutex_lock(&mutex_ios_conectadas);
            list_add(ios_conectadas,io_nueva);
            pthread_mutex_unlock(&mutex_ios_conectadas);
            
            conectados_ios(); //para ver las ios conectadas hasta el momento
            destruir_buffer(buffer);

            sem_post(&sem_proceso_bloqueado_por_io); //para el caso en que todas las ios esten bloqueadas y se conecte una nuve a
        }else{
            log_debug(kernel_logger,"falta handshake inicial (io-kernel)");
        }
    }
    return NULL;
}

void* conexiones_cpus() {  
    //esperando conexiones de cpu hasta detectar un enter
     while(!terminar){ 
        int fd_dispatch = esperar_cliente(fd_server_dispatch);
        int fd_interrupt = esperar_cliente(fd_server_interrupt);
        
        if(fd_interrupt==-1 || fd_dispatch==-1){
            log_info(kernel_logger,"## Se terminó conexiones con CPUs");
            break; 
        }else{
            int identificador_cpu = recibir_identificador_cpu(fd_dispatch);
            log_info(kernel_logger,"## Se conecto nueva CPU con ID %d",identificador_cpu );

            //agregamos la io nueva (asociada a ese nombre) a la lista de ios conectadas 
            cpu_conectada* cpu_nueva = malloc(sizeof(cpu_conectada));
            if (cpu_nueva == NULL) {
                log_error(kernel_logger, "Error al alocar memoria en cpu_nueva en conexiones_cpus()");
                return NULL;
            }

            cpu_nueva->id          = identificador_cpu;
            cpu_nueva->fd_interrupt= fd_interrupt;
            cpu_nueva->fd_dispatch = fd_dispatch;
            cpu_nueva->estado      = LIBRE;

            pthread_mutex_lock(&mutex_cpus_conectadas);
            list_add(cpus_conectadas,cpu_nueva);
            pthread_mutex_unlock(&mutex_cpus_conectadas);

             //lanzo un hilo que espere la rta de cpu
            pthread_t hilo_espera_rta_cpu;
            pthread_create(&hilo_espera_rta_cpu,NULL,espera_rta_cpu,(void *)cpu_nueva);
            pthread_detach(hilo_espera_rta_cpu);

            sem_post(&sem_hay_cpus_disponibles);
           
        }
     }
    conectados_cpus(); // loguea todas las cpus conectadas
    return NULL;
}

void* consola_interactiva() {
     while (!terminar) {
        char* prompt = readline("");
        if (prompt == NULL) continue; // Por si hay error

        if (strcmp(prompt, "") == 0) {
            log_info(kernel_logger, "## Se terminó de escuchar CPUs");
            shutdown(fd_server_dispatch, SHUT_RDWR);
            shutdown(fd_server_interrupt, SHUT_RDWR);
            close(fd_server_dispatch);
            close(fd_server_interrupt);
            free(prompt);
            //sem_post(&esperoAqueSeConectenTodasLasCpus);
            sem_post(&sem_iniciar_planificacion);
            sem_post(&sem_iniciar_planificacion);
            break; //termina hilo
        } else {
            log_info(kernel_logger, "prompt incorrecto");
            free(prompt);
        }
    }
    return NULL;
}
int recibir_identificador_cpu(int fd_conexion) {
    op_code code = recibir_operacion(fd_conexion);

    if(code == HANDSHAKE_CPU_KERNEL){
            //recibimos el nombre de la io
            t_buffer* buffer = recibir_todo_un_buffer(fd_conexion);
            int identificador = extraer_int_del_buffer(buffer);

            destruir_buffer(buffer);

            return identificador;
    }else{
            log_debug(kernel_logger,"falta handshake inicial con cpu, posiblemente el modulo que se quiere conectar no sea una CPU");
            return -1;
    }
}


//estas dos funciones las hice para ver como van quedando las listas de cpu_conectadas y ios_conectadas 

void conectados_ios() {
    //muestra las ios conectadas a kernel
    for(int i=0; i< list_size(ios_conectadas);i++){
        io_conectada* conexion = (io_conectada*) list_get(ios_conectadas, i);
        log_debug(kernel_logger,"## Nombre IO: %s, fd_conexion: %d", conexion->nombre_io, conexion->fd);
    }
}
void conectados_cpus() {
    //muestra las cpus que estan conectadas a kernel
    for(int i=0; i< list_size(cpus_conectadas);i++){
        cpu_conectada* conexion = (cpu_conectada*) list_get(cpus_conectadas, i);
        // log_debug(kernel_logger,"## CPU ID: %d, fd_dispatch: %d, fd_int: %d",conexion->id, conexion->fd_dispatch, conexion->fd_interrupt);
        if(conexion->estado == LIBRE){
            // log_debug(kernel_logger,"## Estado: LIBRE\n");
            log_debug(kernel_logger,"## CPU ID: %d, fd_dispatch: %d, fd_interrupt: %d, estado: LIBRE",conexion->id, conexion->fd_dispatch, conexion->fd_interrupt);

        }   
        else {
            // log_debug(kernel_logger,"## Estado: OCUPADO\n");
            log_debug(kernel_logger,"## CPU ID: %d, fd_dispatch: %d, fd_interrupt: %d, estado: OCUPADO",conexion->id, conexion->fd_dispatch, conexion->fd_interrupt);
            
        }
    }
}
