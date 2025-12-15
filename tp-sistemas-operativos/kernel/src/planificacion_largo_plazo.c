#include "include/planificacion_largo_plazo.h"


void* planificador_largo_plazo(){
   
    //sem_wait(&sem_iniciar_planificacion); //se activa cuando llega el enter
    
    // log_info(kernel_logger,"El planificador de largo plazo deja de estar en STOP hasta que se escuchen todas las cpus");

    pthread_t hilo_inicializador;
    pthread_t hilo_finalizador;
  

    pthread_create(&hilo_inicializador,NULL, new_a_ready_PLANI_LARGO_PLAZO,NULL);
    pthread_create(&hilo_finalizador,NULL,exit_a_fin_PLANI_LARGO_PLAZO,NULL);
    // pthread_create(&hilo_mediano_plazo,NULL,planiMedianoPlazo,NULL);

    // pthread_join(hilo_mediano_plazo,NULL);
    pthread_join(hilo_inicializador,NULL);
    pthread_join(hilo_finalizador,NULL);

    return NULL;
}
    
void* new_a_ready_PLANI_LARGO_PLAZO(){
    while(!terminar){ 
        sem_wait(&sem_iniciar_planificacion); //nuevo
        log_debug(kernel_logger,"SE ACTIVO");
        sem_wait(&sem_hay_procesos_esp_ser_ini_en_mem);  
        log_debug(kernel_logger,"SE ACTIVO EL PLANI LARGO PLAZO");

        pthread_mutex_lock(&mutex_agotamiento_memoria);
        bool rta = se_agoto_memoria_alguna_vez;
        pthread_mutex_unlock(&mutex_agotamiento_memoria);
        // if(rta){      //lo que hace esto es fijarse si alguna vez 
        //                                      //se agoto memeoria, ya que si no se agoto no tendria q esperar a que se libere un proceso
        //     sem_wait(&espacio_para_proceso); 
            
        // }
       
        log_info(kernel_logger, "Planificador largo plazo activo. NEW: %d, SUSP_READY: %d",list_size(procesos_new), list_size(procesos_ready_suspend));
  

        if(!lista_new_vacia() || !lista_susp_ready_vacia()){   
            bool fue_de_ready_suspend;
            pcb_t* proceso = tomar_proceso_proximo_para_memoria(&fue_de_ready_suspend);
            if(hay_espacio_para_proceso(proceso->id, proceso->peso_pseudocodigo)){
                //preguntar si se pudo cargar el proceso en memoria
                log_debug(kernel_logger, "le pregunta a memoria si hay espacio para el proceso %d",proceso->id);

                    if(fue_de_ready_suspend){
                        enviar_deSuspenderProceso(proceso->id);
                        pasar_proceso_de_estado_a_otro(proceso,SUSP_READY,READY);
                        log_info(kernel_logger,"## (<%d>) Pasa del estado <READY_SUSPEND> al estado <READY>",proceso->id);                
                    
                        sem_post(&sem_hay_procesos_en_ready);
                    }else{
                        enviar_path_a_memoria(proceso->archivo_en_pseudoCodigo, proceso->id, proceso->peso_pseudocodigo);
                        pasar_proceso_de_estado_a_otro(proceso,NEW,READY);
                        log_debug(kernel_logger, "linea 51");
                        log_info(kernel_logger,"## (<%d>) Pasa del estado <NEW> al estado <READY>",proceso->id);                
                        sem_post(&sem_hay_procesos_en_ready);
                    }

            }else{
                //como se agoto memoria una vez se tiene que poner el se_agoto_memoria_alguna_vez en true 
                //para que ahora se active el semaforo y cada vez que se libera un proceso en memoria se fije si puede pasarlo a memoria 
                pthread_mutex_lock(&mutex_agotamiento_memoria);
                se_agoto_memoria_alguna_vez= true;
                pthread_mutex_unlock(&mutex_agotamiento_memoria);
                log_debug(kernel_logger, "No hay espacio para el proceso %d en Memoria",proceso->id);
                sem_post(&sem_hay_procesos_esp_ser_ini_en_mem);  //como no lo pudo cargar a ready, sigue estando el proceso en new
                sleep(2);
                log_debug(kernel_logger,"proceso no pudo cargarse en memoria por falta de espacio");
            }
        }else{
            log_debug(kernel_logger,"no hay procesos en NEW-RARO PORQUE TENGO UN SEM");
        }
        sem_post(&sem_iniciar_planificacion);
    }
    return NULL;
}

bool lista_new_vacia(){
    pthread_mutex_lock(&mutex_lista_new);
    bool rta = list_size(procesos_new)==0;
    pthread_mutex_unlock(&mutex_lista_new);

    return rta;
} 
bool lista_susp_ready_vacia() {
    pthread_mutex_lock(&mutex_lista_ready_suspend);
    bool rta = list_size(procesos_ready_suspend)==0;
    pthread_mutex_unlock(&mutex_lista_ready_suspend);

    return rta;

}

bool hay_espacio_para_proceso(int id, int peso){
    //le preg a memoria si tiene espacio para ese proceso y si tiene lo guarda 
    
    pthread_mutex_lock(&mutex_conexion_memoria_nuevo);

    int conexion = crear_conexion(IP_MEMORIA,PUERTO_MEMORIA);
    enviar_hand_shake_kernel_mem(HANDSHAKE_MEM_KERNEL, conexion);
    log_info(kernel_logger,"## Se conecto kernel a memoria");

    t_buffer* buffer = crear_buffer();
    cargar_entero_al_buffer(buffer,id);
    cargar_entero_al_buffer(buffer,peso);
    t_paquete* paquete = crear_paquete(ESPACIO_DISPONIBLE_PARA_PROCESO,buffer);
    enviar_paquete(paquete, conexion);
    eliminar_paquete(paquete);

    //espera de respuesta de memoria
    int codigo = recibir_operacion(conexion);
    if(codigo == RTA_ESPACIO_DISPONIBLE_PARA_PROCESO){
        t_buffer* buffer = recibir_todo_un_buffer(conexion);
        int hay_espacio = extraer_int_del_buffer(buffer); //le puse int, pero en realidad es bool donde si hay espacio retornaria 0, y si no hay 1
        destruir_buffer(buffer);
        close(conexion);
        pthread_mutex_unlock(&mutex_conexion_memoria_nuevo);
        return hay_espacio==0;

    }else{
        log_error(kernel_logger,"operacion incorrecta, se recibio <%d> (se espera RTA_ESPACIO_DISPONIBLE_PROCESO)", codigo);
        close(conexion);
        pthread_mutex_unlock(&mutex_conexion_memoria_nuevo);
        
    } 
}
void enviar_path_a_memoria(char* archivo_en_pseudoCodigo, int id_proceso, int tamanio_proceso){
    pthread_mutex_lock(&mutex_conexion_memoria_nuevo);
    int conexion = crear_conexion(IP_MEMORIA,PUERTO_MEMORIA);
    enviar_hand_shake_kernel_mem(HANDSHAKE_MEM_KERNEL, conexion);
    log_info(kernel_logger,"## Se conecto kernel a memoria");

    t_buffer* buffer = crear_buffer();
    cargar_entero_al_buffer(buffer,id_proceso);
    cargar_string_al_buffer(buffer,archivo_en_pseudoCodigo);
    cargar_entero_al_buffer(buffer,tamanio_proceso);
    t_paquete* paquete = crear_paquete(CREATE_PROCESS,buffer);
    enviar_paquete(paquete, conexion);
    eliminar_paquete(paquete);
    log_debug(kernel_logger,"Kernel espera la respuesta de memoria para el CREATE_PROCESS");
    //espera de respuesta de memoria
    int codigo = recibir_operacion(conexion);
    log_debug(kernel_logger,"codigo de operacion recibido de Memoria: %d",codigo);
    if(codigo == RTA_CREATE_PROCESS){
        t_buffer* buffer = recibir_todo_un_buffer(conexion);
        int hay_espacio = extraer_int_del_buffer(buffer); //le puse int, pero en realidad es bool donde si hay espacio retornaria 0, y si no hay 1
        destruir_buffer(buffer);
        close(conexion);
        pthread_mutex_unlock(&mutex_conexion_memoria_nuevo);

    }else{
        log_error(kernel_logger,"operacion incorrecta (se espera RTA_CREATE_PROCESS)");
        close(conexion);
        pthread_mutex_unlock(&mutex_conexion_memoria_nuevo);
        return;
    } 
    
    close(conexion);
    pthread_mutex_unlock(&mutex_conexion_memoria_nuevo);
     //cierro la conexión porque ya no la necesito mas, ya que no voy a esperar respuesta de memoria
    //enviar la conexión al handler a que espere la respuesta de memoria y pueda cerrar la conexión


}


void enviar_deSuspenderProceso(int id){
    pthread_mutex_lock(&mutex_conexion_memoria_nuevo);
    int conexion = crear_conexion(IP_MEMORIA,PUERTO_MEMORIA);
    enviar_hand_shake_kernel_mem(HANDSHAKE_MEM_KERNEL, conexion);
    log_info(kernel_logger,"## Se conecto kernel a memoria");

    t_buffer* buffer = crear_buffer();
    cargar_entero_al_buffer(buffer,id);
    t_paquete* paquete = crear_paquete(DESSUSPENDER_PROCESO,buffer);
    enviar_paquete(paquete, conexion);
    eliminar_paquete(paquete);

    int codigo = recibir_operacion(conexion);
    if(codigo == OK){ 
        log_debug(kernel_logger,"se dessuspendio el proceos");
    }
    close(conexion);
    pthread_mutex_unlock(&mutex_conexion_memoria_nuevo);
}





//---------------------------------FINALIZACION -----------------------------------------------------------------------------------------------------------------------------------------------------------------

void* exit_a_fin_PLANI_LARGO_PLAZO(){
    while(!terminar){
        sem_wait(&sem_iniciar_planificacion);
        sem_wait(&sem_hay_procesos_en_exit);  //se desbloquea cuando alla procesos en exit 

        pthread_mutex_lock(&mutex_lista_exit);
        pcb_t* proceso =list_get(procesos_exit,0);
        pthread_mutex_unlock(&mutex_lista_exit);

        notificar_finalizacion_proceso_memoria_y_esperar_confirmacion(proceso);
        sacar_proceso_de_estado(proceso,EXIT);
        log_info(kernel_logger,"## (<%d>) Pasa del estado <EXIT> a ser finalizado",proceso->id);
    
        loguear_metricas(proceso);
        liberar_pcb(proceso);

        //si se agoto alguna vez memoria quiero que me avise cuando sale un proceso, para que intente cargar un nuevo_proceso a memoria
        pthread_mutex_lock(&mutex_agotamiento_memoria);
        if(se_agoto_memoria_alguna_vez){
            sem_post(&espacio_para_proceso);
        }
        pthread_mutex_unlock(&mutex_agotamiento_memoria);
        sem_post(&sem_iniciar_planificacion);
    }
    return NULL;
}

void loguear_metricas(pcb_t* proceso){
    log_info(kernel_logger,"## (<%d>) - Métricas de estado: NEW (%d) (%f), READY (%d) (%f) (%f ms), EXEC (%d) (%f), BLOCKED (%d) (%f), SUSP_BLOCKED (%d) (%f), SUSP_READY (%d) (%f), EXIT (%d) (%f)",
              proceso->id, proceso->metricas_estado[NEW], proceso->metricas_tiempo[NEW], 
                           proceso->metricas_estado[READY],proceso->metricas_tiempo[READY],(double)((1000)*(proceso->metricas_tiempo[READY]/proceso->metricas_estado[READY])),
                           proceso->metricas_estado[EXEC],proceso->metricas_tiempo[EXEC],
                           proceso->metricas_estado[BLOCKED],proceso->metricas_tiempo[BLOCKED],
                           proceso->metricas_estado[SUSP_BLOCKED],proceso->metricas_tiempo[SUSP_BLOCKED],
                           proceso->metricas_estado[SUSP_READY],proceso->metricas_tiempo[SUSP_READY],
                           proceso->metricas_estado[EXIT],proceso->metricas_tiempo[EXIT]);
}
void liberar_pcb(pcb_t* proceso){
    list_remove_element(procesos_exit,proceso);
    free(proceso->archivo_en_pseudoCodigo);
    sem_destroy(&proceso->sem_cancelar_suspension);
    sem_destroy(&proceso->sem_activar_suspension);
    pthread_mutex_destroy(&proceso->mutex_esta_siendo_suspendido);
    pthread_mutex_destroy(&proceso->mutex_pc);
    free(proceso);
}

void notificar_finalizacion_proceso_memoria_y_esperar_confirmacion(pcb_t* proceso){
    //se queda esperando hasta que memoria le confirme que elimino proceso
    pthread_mutex_lock(&mutex_conexion_memoria_nuevo);
    int conexion = crear_conexion(IP_MEMORIA,PUERTO_MEMORIA);
    enviar_hand_shake_kernel_mem(HANDSHAKE_MEM_KERNEL, conexion);
    log_info(kernel_logger,"## Se conecto kernel a memoria");

    t_buffer* buffer = crear_buffer();
    cargar_entero_al_buffer(buffer,proceso->id);
    t_paquete* paquete = crear_paquete(ELIMINAR_PROCESO_DE_MEMORIA,buffer);
    enviar_paquete(paquete, conexion);
    eliminar_paquete(paquete);

    //espera de respuesta de memoria
    int codigo = recibir_operacion(conexion);
    if(codigo == RTA_ELIMINAR_PROCESO_DE_MEMORIA){
        t_buffer* buffer = recibir_todo_un_buffer(conexion);
        int pudo_eliminar = extraer_int_del_buffer(buffer); //le puse int, pero en realidad es bool donde si hay espacio retornaria 0, y si no hay 1
        destruir_buffer(buffer);
        if(pudo_eliminar==1){
            perror("la memoria no pudo eliminar el proceso");
        }
        pthread_mutex_unlock(&mutex_conexion_memoria_nuevo);
    }else{
        log_debug(kernel_logger,"operacion incorrecta se esperaba RTA_ELIMINAR_PROCESO_DE_MEMORIA");
        pthread_mutex_unlock(&mutex_conexion_memoria_nuevo);
    } 
}


pcb_t* tomar_proceso_proximo_para_memoria(bool* fue_de_ready_suspend) {
    pcb_t* proceso = NULL;

    // Primero intentamos obtener un proceso de READY_SUSPEND
    pthread_mutex_lock(&mutex_lista_ready_suspend);

    if (!list_is_empty(procesos_ready_suspend)){
        proceso = (pcb_t*) list_get(procesos_ready_suspend, 0);
        log_info(kernel_logger, "Se tomo el PID %d de cola READY-SUSPEND",proceso->id);
        *fue_de_ready_suspend = true;
    }
    pthread_mutex_unlock(&mutex_lista_ready_suspend);

    // Si no había procesos en READY_SUSPEND, probamos con NEW
    if (proceso == NULL) {
        pthread_mutex_lock(&mutex_lista_new);
        if (!list_is_empty(procesos_new)) {
            proceso = (pcb_t*) list_get(procesos_new, 0);
            log_info(kernel_logger, "Se tomo el PID %d de cola NEW",proceso->id);
        }
        pthread_mutex_unlock(&mutex_lista_new);
        *fue_de_ready_suspend = false;
    }

    return proceso;
}

// void* planiMedianoPlazo(){  
//     while(seNecesitaSeguirPreguntandoAMem){
//         sleep(5);
//         sem_post(&espacio_para_proceso);
//     }  
// }

