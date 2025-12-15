#include "include/manejo_de_io.h"


void* conexion_con_io(){
    while(!terminar){
    sem_wait(&sem_proceso_bloqueado_por_io);
    log_debug(kernel_logger,"se activo el buscar_io");
    io_conectada* io_disponible = busco_si_hay_io_libre_para_req();

    if (io_disponible!=NULL){
        log_debug(kernel_logger, "paso buscar_si_hat_io_librePara");
        proceso_esperando_io* nueva = tomar_proximo_req_de_io(io_disponible->nombre_io);

        pthread_mutex_lock(&io_disponible->mutex_io);
        io_disponible->ocupado =true;
        io_disponible->proceso_actual = nueva->proceso;
        pthread_mutex_unlock(&io_disponible->mutex_io);

        log_debug(kernel_logger,"envio a io %d %f",nueva->proceso->id, nueva->tiempo_espera);

        //conectarse con ese fd, mandandole el tiempo
        crear_msj_y_mandarselo_a_io(io_disponible->fd,nueva->proceso->id ,nueva->tiempo_espera);
        
        liberar_proceso_esperando_io(nueva);//donde se hace un free de todo menos el pcb

        //lanzar un hilo para esperar que termine de ejecutar esa io (puse un hilo porque mientras podria procesar otras procesos_esperando_io)
        pthread_t hilo_espera_io;
        pthread_create(&hilo_espera_io,NULL,esperar_rta_io,(void *)io_disponible);
        pthread_detach(hilo_espera_io);

     }else{
        //si esta ocupada la io, que se encole en la lista de procesos_esperado_io 
        pthread_mutex_lock(&mutex_ios_conectadas);
        bool hay_ios_disponibles = !list_is_empty(ios_conectadas);
        pthread_mutex_unlock(&mutex_ios_conectadas);

        pthread_mutex_lock(&mutex_procesos_esperando_ios);
        bool hay_requerimientos_disponibles = !list_is_empty(procesos_esperando_ios);
        pthread_mutex_unlock(&mutex_procesos_esperando_ios);

        if(hay_ios_disponibles && hay_requerimientos_disponibles){
            log_debug(kernel_logger,"el reque de io se encolo");
            // sleep(3);
            //sem_post(&sem_proceso_bloqueado_por_io); //agrege este sleep por si se conceta una io nueva para que vuelva a fijarse 
        }
     }
     
    }
    return NULL;
}


void* esperar_rta_io(void* arg){
    io_conectada* io = (io_conectada *) arg;
    int codigo = recibir_operacion(io->fd);

    //aca tendria que sacar el conetenido pero no se que tendria ese msj
    if(codigo == RTA_SOLICITUD_IO_DE_KERNEL){
        descargasBuffer(io->fd);
        //cancelar_timer_de_supension(io->proceso_actual);
        pthread_mutex_lock(&io->proceso_actual->mutex_esta_siendo_suspendido);
        if (esta_suspendido(io->proceso_actual)) {
                pasar_proceso_de_estado_a_otro(io->proceso_actual,SUSP_BLOCKED, SUSP_READY);
                log_info(kernel_logger,"## (<%d>) Pasa del estado <SUSP_BLOCKED> al estado <READY_SUSPEND>",io->proceso_actual->id);
                //sem_post(&io->proceso_actual->sem_cancelar_suspension);
                sem_post(&sem_hay_procesos_esp_ser_ini_en_mem);
                log_debug(kernel_logger,"el semaforo sem procesos esperando ser ini aumento para proceso %d",io->proceso_actual->id);
        }
        else {
            // log_debug(kernel_logger,"## (<%d>) finalizó IO y pasa a READY", io->proceso_actual->id);
            log_info(kernel_logger,"## (<%d>) Pasa del estado <BLOCKED> al estado <READY>",io->proceso_actual->id);
            pasar_proceso_de_estado_a_otro(io->proceso_actual,BLOCKED,READY);
            sem_post(&io->proceso_actual->sem_cancelar_suspension);
            sem_post(&sem_hay_procesos_en_ready);
        }
        pthread_mutex_unlock(&io->proceso_actual->mutex_esta_siendo_suspendido);
        proceso_esperando_io* nueva = tomar_proximo_req_de_io(io->nombre_io);
        if(nueva!=NULL){
            pthread_mutex_lock(&io->mutex_io);
            io->proceso_actual=nueva->proceso;
            pthread_mutex_unlock(&io->mutex_io);
        }else {
            pthread_mutex_lock(&io->mutex_io);
            io->proceso_actual=NULL;
            pthread_mutex_unlock(&io->mutex_io);
        }
    
        while(nueva!=NULL){
            crear_msj_y_mandarselo_a_io(io->fd, nueva->proceso->id, nueva->tiempo_espera);

            if(recibir_msj_de_io(io->fd)==0){
                //cancelar_timer_de_supension(io->proceso_actual);
                pthread_mutex_lock(&io->proceso_actual->mutex_esta_siendo_suspendido);
                if (esta_suspendido(io->proceso_actual)) {
                    pasar_proceso_de_estado_a_otro(io->proceso_actual,SUSP_BLOCKED, SUSP_READY);
                    log_info(kernel_logger,"## (<%d>) Pasa del estado <SUSP_BLOCKED> al estado <READY_SUSPEND>",io->proceso_actual->id);
                    sem_post(&sem_hay_procesos_esp_ser_ini_en_mem);
                    log_debug(kernel_logger,"el semaforo sem procesos esperando ser ini aumento para proceso %d",io->proceso_actual->id);
                }
                else {
                    log_info(kernel_logger,"## (<%d>) Pasa del estado <BLOCKED> al estado <READY>",io->proceso_actual->id);
                    pasar_proceso_de_estado_a_otro(io->proceso_actual,BLOCKED,READY);
                    sem_post(&io->proceso_actual->sem_cancelar_suspension);
                    sem_post(&sem_hay_procesos_en_ready);
                }
                pthread_mutex_unlock(&io->proceso_actual->mutex_esta_siendo_suspendido);

                nueva = tomar_proximo_req_de_io(io->nombre_io);
                if(nueva!=NULL){
                     pthread_mutex_lock(&io->mutex_io);
                    io->proceso_actual=nueva->proceso;
                    pthread_mutex_unlock(&io->mutex_io);
                }else {
                    pthread_mutex_lock(&io->mutex_io);
                    io->proceso_actual=NULL;
                    pthread_mutex_unlock(&io->mutex_io);
                }

                log_debug(kernel_logger,"se tomo el el reque encolado");
            }else{
                pthread_mutex_lock(&io->proceso_actual->mutex_esta_siendo_suspendido);
                cancelar_timer_de_supension(io->proceso_actual);
                log_info(kernel_logger,"se desconecto IO (%s) de kernel",io->nombre_io);
                liberar_io(io);
                pthread_mutex_unlock(&io->proceso_actual->mutex_esta_siendo_suspendido);
                return NULL;
            }
        }
        pthread_mutex_lock(&io->mutex_io);
        io->ocupado =false; 
        pthread_mutex_unlock(&io->mutex_io);
    }else{
        pthread_mutex_lock(&io->proceso_actual->mutex_esta_siendo_suspendido);
        cancelar_timer_de_supension(io->proceso_actual);
        log_info(kernel_logger,"se desconecto IO (%s) de kernel",io->nombre_io);
        liberar_io(io);
        pthread_mutex_unlock(&io->proceso_actual->mutex_esta_siendo_suspendido);
    }
    pthread_mutex_lock(&io->mutex_io);
    io->ocupado =false; 
    pthread_mutex_unlock(&io->mutex_io);

    return NULL;
} 


proceso_esperando_io* tomar_proximo_req_de_io(char* nombre_io_sol){
    bool mismo_nombre(void* elem) {
        proceso_esperando_io* req_io = (proceso_esperando_io*)elem;
        return strcmp(req_io->nombre_io, nombre_io_sol) == 0;
    };
    pthread_mutex_lock(&mutex_procesos_esperando_ios);
    proceso_esperando_io* req_io_con_mismo_nombre =(proceso_esperando_io*) list_remove_by_condition(procesos_esperando_ios, mismo_nombre);
    pthread_mutex_unlock(&mutex_procesos_esperando_ios);

    return req_io_con_mismo_nombre;
}


//FUNCIONALIDADES EXTRAS PARA EL FUNCIONAMIENTO DEL CODIGO
void crear_msj_y_mandarselo_a_io(int fd,int id, double tiempo){
    t_buffer* buffer = crear_buffer();
    cargar_entero_al_buffer(buffer,tiempo);
    cargar_entero_al_buffer(buffer,id);
    t_paquete* paquete = crear_paquete(SOLICITUD_IO_DE_KERNEL, buffer);
    enviar_paquete(paquete, fd);
    eliminar_paquete(paquete);
} 

int recibir_msj_de_io(int fd){
    int codigo = recibir_operacion(fd);
    if(codigo == RTA_SOLICITUD_IO_DE_KERNEL){
         descargasBuffer(fd);
        return 0;
    }else{
        log_debug(kernel_logger,"se recibio el cod_op de io %d", codigo);
        return 1;
    }
}

void liberar_io(io_conectada* io){
    log_debug(kernel_logger,"se llamo a liberar_io IMPORTANTE");
    
    bool misma_io(void* elem) {
        io_conectada* io_conectada_lista = (io_conectada*)elem;
        return io_conectada_lista == io;
    };
    pthread_mutex_lock(&mutex_ios_conectadas);
    io_conectada* io_eliminada = list_remove_by_condition(ios_conectadas, misma_io);
    pthread_mutex_unlock(&mutex_ios_conectadas);

    if(io_eliminada==NULL){
        perror("se trato de eliminar una io que no estaba en ios_conectadas");
        return;
    }

    //fuerza a exit al proceso_actual
    if(esta_suspendido(io_eliminada->proceso_actual)){
        pasar_proceso_de_estado_a_otro(io_eliminada->proceso_actual,SUSP_BLOCKED,EXIT);
        log_info(kernel_logger,"## (<%d>) Pasa del estado <SUSP_BLOCKED> al estado <EXIT>",io_eliminada->proceso_actual->id);
    }else{
        pasar_proceso_de_estado_a_otro(io_eliminada->proceso_actual,BLOCKED,EXIT);
        log_info(kernel_logger,"## (<%d>) Pasa del estado <BLOCKED> al estado <EXIT>",io_eliminada->proceso_actual->id);
    }
   
    sem_post(&sem_hay_procesos_en_exit);

    
    if(no_hay_mas_ios_con_mismo_nombre(io_eliminada->nombre_io)){
        forzar_exit_a_todos_los_bloqueados(io_eliminada->nombre_io);
    }

    //fuera a exit a todos los enconlados
    //forzar_exit_a_todos_los_bloqueados(io_eliminada);

    free(io_eliminada->nombre_io);
    //list_destroy(io_eliminada->procesos_esperando_io); 
    pthread_mutex_destroy(&io_eliminada->mutex_io);
    free(io_eliminada);
}


bool no_hay_mas_ios_con_mismo_nombre(char* nombre){
    bool mismo_nombre_io(void* elem) {
        io_conectada* io_conectada_lista = (io_conectada*)elem;
        return strcmp(io_conectada_lista->nombre_io, nombre) == 0;
    };
    pthread_mutex_lock(&mutex_ios_conectadas);
    bool hay_ios_con_nombre_igual = list_any_satisfy(ios_conectadas, mismo_nombre_io);
    pthread_mutex_unlock(&mutex_ios_conectadas);

    return hay_ios_con_nombre_igual==false;
}

/*
void liberar_io(io_conectada* io){
    //sacar io de io_conectadass 
    log_debug(kernel_logger,"se llamo a liberar_io IMPORTANTE");
    //pthread_mutex_lock(&mutex_ios_conectadas);
    //io_conectada* io_eliminada = (io_conectada*)list_remove_element(ios_conectadas, io);
    //pthread_mutex_unlock(&mutex_ios_conectadas);

    bool mismo_nombre_io(void* elem) {
        io_conectada* io_conectada_lista = (io_conectada*)elem;
        return strcmp(io_conectada_lista->nombre_io, io->nombre_io) == 0;
    };
    pthread_mutex_lock(&mutex_ios_conectadas);
    io_conectada* io_eliminada = list_remove_by_condition(ios_conectadas, mismo_nombre_io);
    pthread_mutex_unlock(&mutex_ios_conectadas);

    if(io_eliminada==NULL){
        perror("se trato de eliminar una io que no estaba en ios_conectadas");
        return;
    }

    //fuerza a exit al proceso_actual
    if(esta_suspendido(io_eliminada->proceso_actual)){
        pasar_proceso_de_estado_a_otro(io_eliminada->proceso_actual,SUSP_BLOCKED,EXIT);
        log_info(kernel_logger,"## (<%d>) Pasa del estado <SUSP_BLOCKED> al estado <EXIT>",io_eliminada->proceso_actual->id);
    }else{
        pasar_proceso_de_estado_a_otro(io_eliminada->proceso_actual,BLOCKED,EXIT);
        log_info(kernel_logger,"## (<%d>) Pasa del estado <BLOCKED> al estado <EXIT>",io_eliminada->proceso_actual->id);
    }
   
    sem_post(&sem_hay_procesos_en_exit);

    //fuera a exit a todos los enconlados
    forzar_exit_a_todos_los_bloqueados(io_eliminada);

    free(io_eliminada->nombre_io);
    list_destroy(io_eliminada->procesos_esperando_io); 
    pthread_mutex_destroy(&io_eliminada->mutex_io);
    free(io_eliminada);
}
*/
/*
void encolar_proceso_a_io(proceso_esperando_io* proceso){
    io_conectada* io = find_by_name_(ios_conectadas,proceso->nombre_io);

    pthread_mutex_lock(&io->mutex_io);
    list_add(io->procesos_esperando_io,proceso);
    pthread_mutex_unlock(&io->mutex_io);
}
*/

void forzar_exit_a_todos_los_bloqueados(char* io_nombre) {
    log_debug(kernel_logger,"entra a forzar_exit_todos_los_procesos IMPORTANTE");

    proceso_esperando_io* p = tomar_proximo_req_de_io(io_nombre);

    while (p!=NULL) {
        //fuerza a exit 
        if(esta_suspendido(p->proceso)){
            pasar_proceso_de_estado_a_otro(p->proceso,SUSP_BLOCKED,EXIT);
            log_info(kernel_logger,"## (<%d>) Pasa del estado <SUSP_BLOCKED> al estado <EXIT>",p->proceso->id);
        }else{
            pasar_proceso_de_estado_a_otro(p->proceso,BLOCKED,EXIT);
            log_info(kernel_logger,"## (<%d>) Pasa del estado <BLOCKED> al estado <EXIT>",p->proceso->id);
        }
        sem_post(&sem_hay_procesos_en_exit);
    
        liberar_proceso_esperando_io(p);
        p = tomar_proximo_req_de_io(io_nombre);
    }
}



/*
void forzar_exit_a_todos_los_bloqueados(io_conectada* io) {
    log_debug(kernel_logger,"entra a forzar_exit_todos_los_procesos IMPORTANTE");

    pthread_mutex_lock(&io->mutex_io);

     if (io->procesos_esperando_io == NULL) {
        log_error(kernel_logger, "procesos_esperando_io es NULL al intentar forzar exit");
        pthread_mutex_unlock(&io->mutex_io);
        return;
    }

    proceso_esperando_io* p;
    while (!list_is_empty(io->procesos_esperando_io)) {
        //fuerza a exit 
        log_debug(kernel_logger, "la lista no esta vacia IMPORTANTE");
        p = (proceso_esperando_io*) list_remove(io->procesos_esperando_io, 0);
        if(esta_suspendido(p->proceso)){
            pasar_proceso_de_estado_a_otro(p->proceso,SUSP_BLOCKED,EXIT);
            log_info(kernel_logger,"## (<%d>) Pasa del estado <SUSP_BLOCKED> al estado <EXIT>",p->proceso->id);
        }else{
            pasar_proceso_de_estado_a_otro(p->proceso,BLOCKED,EXIT);
            log_info(kernel_logger,"## (<%d>) Pasa del estado <BLOCKED> al estado <EXIT>",p->proceso->id);
        }
   
        sem_post(&sem_hay_procesos_en_exit);
    
        liberar_proceso_esperando_io(p);
    }
    pthread_mutex_unlock(&io->mutex_io);
}
*/

void liberar_proceso_esperando_io(proceso_esperando_io* proceso_esperando){
    free(proceso_esperando->nombre_io);
    free(proceso_esperando);
}

bool esta_ocupada_io(char* nombre_io){
    io_conectada* io = find_by_name_(ios_conectadas, nombre_io);

    pthread_mutex_lock(&io->mutex_io);
    bool rta = io->ocupado;
    pthread_mutex_unlock(&io->mutex_io);
    return rta;
}


io_conectada* buscar_io_libre_con_nombre(char* nombre ){
    bool mismo_nombre_io(void* elem) {
        io_conectada* io_conectada_lista = (io_conectada*)elem;
        return strcmp(io_conectada_lista->nombre_io, nombre) == 0;
    };
    pthread_mutex_lock(&mutex_ios_conectadas);
    t_list* ios_con_nombre_igual = list_filter(ios_conectadas, mismo_nombre_io);
    pthread_mutex_unlock(&mutex_ios_conectadas);

    io_conectada* libre = buscar_io_libre(ios_con_nombre_igual);
    list_destroy(ios_con_nombre_igual);
    return libre;

}
io_conectada* buscar_io_libre(t_list* ios_seleccionadas) {
    bool esta_libre_io(void* elem) {
        io_conectada* io_conectada_lista = (io_conectada*)elem;
        return io_conectada_lista->ocupado == false;
    };
    pthread_mutex_lock(&mutex_ios_conectadas);
    io_conectada* io_libre = list_find(ios_seleccionadas,esta_libre_io);
    pthread_mutex_unlock(&mutex_ios_conectadas);
    return io_libre; 
}

void descargasBuffer(int fd){
    t_buffer *buffer = recibir_todo_un_buffer(fd);
    if(buffer != NULL) {
        char* nombre = extraer_string_del_buffer(buffer);
        if(nombre != NULL) {
            free(nombre);
        }
        destruir_buffer(buffer);
    }
}

io_conectada* busco_si_hay_io_libre_para_req(){
    pthread_mutex_lock(&mutex_ios_conectadas);
    for (int i = 0; i < list_size(ios_conectadas); i++) {
        io_conectada* io = list_get(ios_conectadas, i);
        pthread_mutex_lock(&io->mutex_io);
        bool esta_libre = (io->ocupado == false);
        pthread_mutex_unlock(&io->mutex_io);
        if (esta_libre && tiene_requerimiento(io->nombre_io)) {
            pthread_mutex_unlock(&mutex_ios_conectadas);
            return io;
        }
    }
    pthread_mutex_unlock(&mutex_ios_conectadas);
    log_debug(kernel_logger,"no encontro io libre para req");
    return NULL;
}

bool tiene_requerimiento(char* nombre){
     bool existe_req(void* elem) {
        proceso_esperando_io* req = (proceso_esperando_io*)elem;
        return strcmp(req->nombre_io,nombre)==0;
    };
    pthread_mutex_lock(&mutex_procesos_esperando_ios);
    bool rta = list_any_satisfy(procesos_esperando_ios,existe_req);
    pthread_mutex_unlock(&mutex_procesos_esperando_ios);
    return rta;
}

     

