#include "include/planificador_corto_plazo.h"


void* planificacion_corto(){
    while(!terminar){
        sem_wait(&sem_hay_cpus_disponibles);
        log_debug(kernel_logger,"hay cpu disponible");
        sem_wait(&sem_hay_procesos_en_ready); // una vez se envia el proceso a memoria este semaforo se activa

        //busco la que esta disponible y validarlo 
        cpu_conectada* cpu = buscar_cpu_libre();
        if(cpu==NULL){
            log_info(kernel_logger,"se activa el semaforo que hay cpu LIBRE, pero no se encontro ninguna");
            continue; 
        }else{
            log_debug(kernel_logger, "## Se le asigna la CPU %d para ejecutar el proceso", cpu->id);
        }

        //busco el proximo proceso a ejecutar teniendo en cuenta los dif algoritmos y validarlo
        pcb_t *proceso_proximo = buscar_proximo_proceso(); 


        if(proceso_proximo==NULL){
            log_error(kernel_logger,"se activa el semaforo que hay procesos en READY, pero no se encontro ninguno");
            continue; 
        } 

        pthread_mutex_lock(&proceso_proximo->mutex_pc);
        int pc_a_enviar = proceso_proximo->pc;
        pthread_mutex_unlock(&proceso_proximo->mutex_pc);

        
        pasar_proceso_de_estado_a_otro(proceso_proximo,READY,EXEC);
        log_info(kernel_logger,"## (<%d>) Pasa del estado <READY> al estado <EXEC>",proceso_proximo->id);

        //marco la cpu como ocupada
        pthread_mutex_lock(&mutex_cpus_conectadas);
        cpu->id_proceso=proceso_proximo->id;
        cpu->estado=OCUPADA;
        pthread_mutex_unlock(&mutex_cpus_conectadas);

        log_debug(kernel_logger,"## El proceso (<%d>) es tomado por la CPU %d",proceso_proximo->id,cpu->id);

        //le mando un msj a cpu (no se si lleva mutex o no )
      
        mandar_mensaje_a_cpu(cpu, proceso_proximo->id, pc_a_enviar);
        proceso_proximo->cronometro = temporal_create();
    
        log_debug(kernel_logger, "## Se le envia a CPU el PID: %d PC:%d ",proceso_proximo->id,pc_a_enviar);

    }
    return NULL;
}

///////FUNCIONALIDADES PARA QUE FUNCIONE EL PLANIFICADOR 
cpu_conectada* buscar_cpu_libre(){
    bool cpu_libre(void* ptr) {
        cpu_conectada* cpu = (cpu_conectada*) ptr;
        return cpu->estado==LIBRE;
    }
    pthread_mutex_lock(&mutex_cpus_conectadas);
    cpu_conectada* cpu = (cpu_conectada*) list_find(cpus_conectadas, cpu_libre);
    pthread_mutex_unlock(&mutex_cpus_conectadas);
    
    return cpu;
    
}

pcb_t* buscar_proximo_proceso(){
    pcb_t *proceso_proximo;

    if(strcmp(ALGORITMO_CORTO_PLAZO, "FIFO") ==0){
        pthread_mutex_lock(&mutex_lista_ready);
        proceso_proximo = list_get(procesos_ready, 0);
        pthread_mutex_unlock(&mutex_lista_ready);
    }else if(strcmp(ALGORITMO_CORTO_PLAZO, "SJF") ==0){

        pthread_mutex_lock(&mutex_lista_ready);
        proceso_proximo = list_get(procesos_ready,0);
        pthread_mutex_unlock(&mutex_lista_ready);
        log_debug(kernel_logger,"(<%d>) - Seleccionado para ejecutar por algoritmo SJF",proceso_proximo->id);

    }else if(strcmp(ALGORITMO_CORTO_PLAZO,"SRT")==0){

        pthread_mutex_lock(&mutex_lista_ready);
        proceso_proximo = list_get(procesos_ready,0);
        pthread_mutex_unlock(&mutex_lista_ready);
        log_debug(kernel_logger,"(<%d>) - Seleccionado para ejecutar por algoritmo SRT",proceso_proximo->id);
    }else{
        log_error(kernel_logger,"EL ALGORTIMO DE CORTO PLAZO ES ERRONEO");
        return NULL;
    }
    return proceso_proximo;
}

void mandar_mensaje_a_cpu(cpu_conectada* cpu, int pid, int pc){
    t_buffer* buffer = crear_buffer();
    cargar_entero_al_buffer(buffer,pid);

    pthread_mutex_lock(&mutex_lista_blocked);
    cargar_entero_al_buffer(buffer,pc);
    pthread_mutex_unlock(&mutex_lista_blocked);

    t_paquete* paquete = crear_paquete(EJECUCION_DE_PROCESO_KER_CPU, buffer);
    enviar_paquete(paquete,cpu->fd_dispatch);
    eliminar_paquete(paquete);
}

void mandar_mensaje_por_interrupt_a_cpu(cpu_conectada* cpu, int pid){
    t_buffer* buffer_interrupt = crear_buffer();
    cargar_entero_al_buffer(buffer_interrupt,pid);
    t_paquete* paquete_interrupt = crear_paquete(DESALOJAR_PROCESO_KER_CPU, buffer_interrupt);
    enviar_paquete(paquete_interrupt,cpu->fd_interrupt);
    eliminar_paquete(paquete_interrupt);
}

pcb_t* buscar_proceso_con_menor_estimacion(){
    
    pcb_t* proceso_mas_corto;

    proceso_mas_corto = list_get_minimum(procesos_ready,menor_estimacion);

    return proceso_mas_corto;
}

void* menor_estimacion(void* a, void* b){
	    pcb_t* proceso_a = (pcb_t*) a;
        pcb_t* proceso_b = (pcb_t*) b;

        if(strcmp(ALGORITMO_CORTO_PLAZO,"SRT")==0){
            return proceso_a->tiempo_restante_rafaga <= proceso_b->tiempo_restante_rafaga ? proceso_a : proceso_b;
        }
        else
	        return proceso_a->estimacion_proxima_rafaga <= proceso_b->estimacion_proxima_rafaga ? proceso_a : proceso_b;
}

//FUNCIONES PARA SRT

pcb_t* proceso_largo_en_exec(){

    pcb_t *proceso_corto_en_ready;
    pcb_t *proceso_largo_en_exec;

    // Busco el proceso con menor estimacion de READY

    pthread_mutex_lock(&mutex_lista_ready);
    pthread_mutex_lock(&mutex_lista_exec);

    if(list_is_empty(procesos_exec)){
        log_debug(kernel_logger,"No hay procesos en EXEC para desalojar");
        pthread_mutex_unlock(&mutex_lista_ready);
        pthread_mutex_unlock(&mutex_lista_exec);
        return NULL;
    }
    if(list_is_empty(procesos_ready)){
        log_debug(kernel_logger,"No hay procesos en READY para buscar");
        pthread_mutex_unlock(&mutex_lista_ready);
        pthread_mutex_unlock(&mutex_lista_exec);
        return NULL;
    }

    proceso_corto_en_ready = buscar_proceso_con_menor_estimacion();

    // Busco el proceso con mayor estimacion en EXEC

    proceso_largo_en_exec = buscar_proceso_con_mayor_estimacion();

    if(proceso_largo_en_exec==NULL){
        pthread_mutex_unlock(&mutex_lista_exec);
        pthread_mutex_unlock(&mutex_lista_ready);
        return proceso_largo_en_exec;
    }

    double tiempo_ejecutando = temporal_gettime(proceso_largo_en_exec->cronometro);

    log_info(kernel_logger,"Rafaga del proceso más corto %f",proceso_corto_en_ready->tiempo_restante_rafaga);
    log_info(kernel_logger,"Rafaga del proceso más largo %f", (proceso_largo_en_exec->tiempo_restante_rafaga)-tiempo_ejecutando);

    bool resultado = proceso_corto_en_ready->tiempo_restante_rafaga < ((proceso_largo_en_exec->tiempo_restante_rafaga)-tiempo_ejecutando);

    pthread_mutex_unlock(&mutex_lista_exec);
    pthread_mutex_unlock(&mutex_lista_ready);

    if(resultado){
        proceso_largo_en_exec->tiempo_restante_rafaga = proceso_largo_en_exec->tiempo_restante_rafaga - tiempo_ejecutando;
        proceso_largo_en_exec->tiempo_antes_de_syscall += tiempo_ejecutando;
        temporal_stop(proceso_largo_en_exec->cronometro);
        return proceso_largo_en_exec;
    }else
    {
        return NULL;
    }
}

pcb_t* buscar_proceso_con_mayor_estimacion(){
    
    bool condicion(void* ptr){
        pcb_t* proceso = (pcb_t*) ptr;
        return !(proceso->desalojado);
    }

    pcb_t* proceso_mas_largo;
    t_list* procesos_no_desalojados = list_filter(procesos_exec,condicion);

    if(list_is_empty(procesos_no_desalojados))
        return NULL;

    proceso_mas_largo = list_get_maximum(procesos_no_desalojados,mayor_estimacion);

    list_destroy(procesos_no_desalojados);

    return proceso_mas_largo;
}

void* mayor_estimacion(void* a, void* b){
	pcb_t* proceso_a = (pcb_t*) a;
    pcb_t* proceso_b = (pcb_t*) b;

    double tiempo_ejecutandoA = 1000*(calculo_tiempo_proceso_en_estado(proceso_a));
    double tiempo_ejecutandoB = 1000*(calculo_tiempo_proceso_en_estado(proceso_b));

    return ((proceso_a->tiempo_restante_rafaga)-tiempo_ejecutandoA) >= ((proceso_b->tiempo_restante_rafaga)-tiempo_ejecutandoB) ? proceso_a : proceso_b;
}

bool hay_cpu_libre(){

    pthread_mutex_lock(&mutex_cpus_conectadas);
    if(list_is_empty(cpus_conectadas)){
        pthread_mutex_unlock(&mutex_cpus_conectadas);
        return true;
    }

    bool condicion(void* ptr){
        cpu_conectada* cpu = (cpu_conectada*) ptr;
        return cpu->estado == LIBRE;
    }
    bool respuesta = list_any_satisfy(cpus_conectadas,condicion);
    pthread_mutex_unlock(&mutex_cpus_conectadas);

    return respuesta;
}

void desalojar_proceso(pcb_t* proceso){

    bool mismo_id_proceso(void* ptr) {
        cpu_conectada* cpu = (cpu_conectada*) ptr;
        return cpu->id_proceso == proceso->id;
    }

    pthread_mutex_lock(&mutex_cpus_conectadas);
    cpu_conectada* cpu_a_liberar = list_find(cpus_conectadas,mismo_id_proceso);
    pthread_mutex_unlock(&mutex_cpus_conectadas);

    if(cpu_a_liberar == NULL)
        log_debug(kernel_logger,"no se encontro la cpu");

    mandar_mensaje_por_interrupt_a_cpu(cpu_a_liberar,proceso->id);

    log_warning(kernel_logger,"## (<%d>) - Se solicito su desalojo por algoritmo SJF/SRT", proceso->id);

    log_warning(kernel_logger,"Se envio el mensaje a la CPU %d para desalojar el proceso %d",cpu_a_liberar->id, proceso->id);
}