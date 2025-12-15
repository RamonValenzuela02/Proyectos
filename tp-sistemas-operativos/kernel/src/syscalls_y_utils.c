#include "include/syscalls_y_utils.h"

//--------------------------------------SYSCALLS ----------------------------------------------------------------
void SYSCALL_INIT_PROC(char* archivo_en_pseudoCodigo, int tamanio_proceso){

    log_debug(kernel_logger, "entra a la funcion SYSCALL_INIT_PROC");
    //creo el nuevo pcb con sus datos iniciales
    pcb_t* pcb_nuevo = malloc(sizeof(pcb_t));
    
    if (pcb_nuevo == NULL) {
        log_error(kernel_logger, "Error al alocar memoria para SYSCALL_INIT_PROC");
        exit(EXIT_FAILURE);
    }


  
    pthread_mutex_lock(&mutex_id_pcb_generador); 
    pcb_nuevo->id = id_pcb_generador;
    id_pcb_generador++;
    pthread_mutex_unlock(&mutex_id_pcb_generador);

  
    
    pcb_nuevo->peso_pseudocodigo = tamanio_proceso;  
    pcb_nuevo->archivo_en_pseudoCodigo = strdup(archivo_en_pseudoCodigo); 
    pcb_nuevo->pc =0;
    memset(pcb_nuevo->metricas_estado, 0, sizeof(pcb_nuevo->metricas_estado)); //seteo las metricas en cero
    memset(pcb_nuevo->metricas_tiempo, 0, sizeof(pcb_nuevo->metricas_tiempo)); //setea las metricas en cero
    pcb_nuevo->estimacion_proxima_rafaga = (float)ESTIMACION_INICIAL;
    pcb_nuevo->tiempo_restante_rafaga = (float)ESTIMACION_INICIAL;
    pcb_nuevo->desalojado = false;
    pcb_nuevo->tiempo_antes_de_syscall = 0; // Si hace syscall (ej IO) guarda todo el tiempo que estuvo en CPU y con ese se calcula la estimación
    pcb_nuevo->tiempo_real_ejecutado = 0;
    sem_init(&pcb_nuevo->sem_cancelar_suspension, 0, 0);
    sem_init(&pcb_nuevo->sem_activar_suspension, 0, 0); 
    pthread_mutex_init(&pcb_nuevo->mutex_esta_siendo_suspendido,NULL); 
    pthread_mutex_init(&(pcb_nuevo->mutex_pc), NULL); 
  
   
    // pcb_nuevo->estado_actual=NEW
    agregar_proceso_a_estado(pcb_nuevo,NEW); //agrega el proceso a new 
   

    log_info(kernel_logger, "## (<%d>) - Solicitó syscall: <INIT_PROCESO>",pcb_nuevo->id);

    log_info(kernel_logger, "## (<%d>) Se crea el proceso - Estado: NEW", pcb_nuevo->id);
    sem_post(&sem_hay_procesos_esp_ser_ini_en_mem);
}

void SISCALL_EXIT(pcb_t* proceso){
    log_info(kernel_logger, "## (<%d>) - Solicitó syscall: <EXIT>",proceso->id);
    pasar_proceso_de_estado_a_otro(proceso,EXEC,EXIT);
    log_info(kernel_logger,"## (<%d>) Pasa del estado <EXEC> al estado <EXIT>",proceso->id);
    
    sem_post(&sem_hay_procesos_en_exit);
}

void* SISCALL_DUMP_MEMORY(void* pro){
    pcb_t* proceso = (pcb_t *)pro;
    
    log_info(kernel_logger, "## (<%d>) - Solicitó syscall: <DUMP_MEMORY>",proceso->id);

    pthread_mutex_lock(&mutex_conexion_memoria_nuevo);
    int conexion = crear_conexion(IP_MEMORIA,PUERTO_MEMORIA);
    enviar_hand_shake_kernel_mem(HANDSHAKE_MEM_KERNEL, conexion);
    log_debug(kernel_logger,"## Se conecto kernel a memoria");

   
    t_buffer* buffer = crear_buffer();
    cargar_entero_al_buffer(buffer,proceso->id);
    t_paquete* paquete = crear_paquete(DUMP_MEMORY,buffer);
    enviar_paquete(paquete, conexion);
    eliminar_paquete(paquete);


    pasar_proceso_de_estado_a_otro(proceso,EXEC,BLOCKED);
    proceso->estimacion_proxima_rafaga = calcular_estimacion_proxima_rafaga(proceso);
    proceso->tiempo_restante_rafaga = proceso->estimacion_proxima_rafaga;
    proceso->tiempo_antes_de_syscall = 0;
    // esperar_activacion_tiempo_suspencion(proceso); //LO COMENTO PORQUE SINO ME MANDA A SUSPENDER EL PROCESO MIENTRAS ESCRIBE

    log_info(kernel_logger, "## (<%d>) Pasa del estado <EXEC> al estado <BLOCKED>",proceso->id);
    log_info(kernel_logger, "## (<%d>) - Bloqueado por syscall: <DUMP_MEMORY>",proceso->id);

    int codigo = recibir_operacion(conexion);
    // cancelar_timer_de_supension(proceso); 
    pthread_mutex_unlock(&mutex_conexion_memoria_nuevo);
    if(codigo == RTA_DUMP_MEMORY){
        t_buffer* buffer = recibir_todo_un_buffer(conexion);
        int confirmacion = extraer_int_del_buffer(buffer);
        destruir_buffer(buffer);

        if(confirmacion){
            pasar_proceso_de_estado_a_otro(proceso,BLOCKED,READY);
            log_info(kernel_logger, "## (<%d>) Pasa del estado <BLOCKED> al estado <READY>",proceso->id);
            sem_post(&sem_hay_procesos_en_ready);
        }else{
            pasar_proceso_de_estado_a_otro(proceso,BLOCKED,EXIT);
            log_info(kernel_logger, "## (<%d>) Pasa del estado <BLOCKED> al estado <EXIT>",proceso->id);
            sem_post(&sem_hay_procesos_en_exit);
        }
    }else{
        log_error(kernel_logger,"Operacion incorrecta, se espera RTA_DUMP_MEMORY");
        return NULL;
    }

    return NULL;
}


void SISCALL_IO(pcb_t* proceso, char* nombre_io, double miliSegundos){
    if(es_valida_io(nombre_io)){
        proceso_esperando_io* nuevo_proceso = malloc(sizeof(proceso_esperando_io));
        if (nuevo_proceso == NULL) {
            log_error(kernel_logger, "Error al alocar memoria para para SISCALL_IO");
            exit(EXIT_FAILURE);
        }
        nuevo_proceso->proceso = proceso;
        nuevo_proceso->tiempo_espera = miliSegundos;
        nuevo_proceso->nombre_io = strdup(nombre_io);

        proceso->motivoDeBloqueo = IO; // le seteo el motivo
        pasar_proceso_de_estado_a_otro(proceso,EXEC,BLOCKED);
        proceso->estimacion_proxima_rafaga = calcular_estimacion_proxima_rafaga(proceso);
        proceso->tiempo_restante_rafaga = proceso->estimacion_proxima_rafaga;
        proceso->tiempo_antes_de_syscall = 0;
        esperar_activacion_tiempo_suspencion(proceso);

        log_info(kernel_logger,"## (<%d>) - Bloqueado por IO: <%s>",proceso->id,nombre_io);
        log_info(kernel_logger,"## (<%d>) Pasa del estado <EXEC> al estado <BLOCKED>",proceso->id);

        pthread_mutex_lock(&mutex_procesos_esperando_ios);
        list_add(procesos_esperando_ios, nuevo_proceso);
        pthread_mutex_unlock(&mutex_procesos_esperando_ios);

        sem_post(&sem_proceso_bloqueado_por_io);
    }else{
        //si no es valida el proceso pasar a exit
        log_debug(kernel_logger,"la IO solicitada por el proceso %d no es valida",proceso->id);

        pasar_proceso_de_estado_a_otro(proceso,EXEC,EXIT);
        log_info(kernel_logger,"## (<%d>) Pasa del estado <EXEC> al estado <EXIT>",proceso->id);

        sem_post(&sem_hay_procesos_en_exit);
    }
}

//EXTRAS PARA SYSCALLS
void esperar_activacion_tiempo_suspencion(pcb_t* proceso){
    sem_wait(&proceso->sem_activar_suspension);
    log_debug(kernel_logger,"se ACTIVO EL TIMER DE SUSPENCION");
}

bool comparador_proceso_mas_chico(void *pcb_a, void* pcb_b){
    pcb_t *proceso_a = (pcb_t *)pcb_a;
    pcb_t *proceso_b = (pcb_t *)pcb_b;
    
    return proceso_a->peso_pseudocodigo < proceso_b->peso_pseudocodigo;
}
bool es_valida_io(char* nombre_io){
    return find_by_name_(ios_conectadas,nombre_io)!=NULL;
}

io_conectada* find_by_name_(t_list* ios_conectadas, char* nombre) {
    bool _same_name(void* ptr) {
        io_conectada* io = (io_conectada*) ptr;
        return strcmp(io->nombre_io,nombre)==0;
    }
    pthread_mutex_lock(&mutex_ios_conectadas);
    io_conectada* io_solicitada = list_find(ios_conectadas, _same_name);
    pthread_mutex_unlock(&mutex_ios_conectadas);

    if(io_solicitada==NULL){
        log_debug(kernel_logger,"la io solicitada no se encuentra en ios_conectadas");
        return;
    }

    return io_solicitada;
}


double calculo_tiempo_proceso_en_estado(pcb_t* proceso){
    struct timespec inicio = proceso->tiempo_entrada_a_estado;
    struct timespec fin;
    clock_gettime(CLOCK_MONOTONIC, &fin);

    double tiempo = (fin.tv_sec - inicio.tv_sec) + (fin.tv_nsec - inicio.tv_nsec) / 1e9;  //hace la dif entre los seg(entre fin - inicio) y lo mismo con los nano(pero los divide por 1e9 ya que lo vamos a mostrar en seg)
    return tiempo;
}

void pasar_proceso_de_estado_a_otro(pcb_t* proceso, EstadoProceso estado_inicial,EstadoProceso estado_final){
    sacar_proceso_de_estado(proceso,estado_inicial);
    agregar_proceso_a_estado(proceso,estado_final);
    
}



void sacar_proceso_de_estado(pcb_t* proceso,EstadoProceso estado){
    switch(estado){
        case(NEW):
            pthread_mutex_lock(&mutex_lista_new);
            //falta verificar
            list_remove_element(procesos_new,proceso);
            proceso->metricas_tiempo[NEW]+=calculo_tiempo_proceso_en_estado(proceso);
            pthread_mutex_unlock(&mutex_lista_new);
            log_debug(kernel_logger,"## (<%d>) deja el estado <NEW> ",proceso->id);
        break;
        case(READY):
            pthread_mutex_lock(&mutex_lista_ready);
            //falta verificar
            list_remove_element(procesos_ready,proceso);
            proceso->metricas_tiempo[READY]+=calculo_tiempo_proceso_en_estado(proceso);
            pthread_mutex_unlock(&mutex_lista_ready);
        break;
        case(EXEC):
            pthread_mutex_lock(&mutex_lista_exec);
            //falta verificar
            list_remove_element(procesos_exec,proceso);

            double tiempo_en_exec = calculo_tiempo_proceso_en_estado(proceso);

            proceso->metricas_tiempo[EXEC]+=tiempo_en_exec;
            
            pthread_mutex_unlock(&mutex_lista_exec);
        break;
        case(BLOCKED):
            pthread_mutex_lock(&mutex_lista_blocked);
            //falta verificar
            list_remove_element(procesos_blocked,proceso);
            proceso->metricas_tiempo[BLOCKED]+=calculo_tiempo_proceso_en_estado(proceso);
            pthread_mutex_unlock(&mutex_lista_blocked);
        break;
        case(EXIT):
            pthread_mutex_lock(&mutex_lista_exit);
            //falta verificar
            list_remove_element(procesos_exit,proceso);
            proceso->metricas_tiempo[EXIT]+=calculo_tiempo_proceso_en_estado(proceso);
            pthread_mutex_unlock(&mutex_lista_exit);
        break;
        case(SUSP_BLOCKED):
            pthread_mutex_lock(&mutex_lista_blocked_suspend);
            //falta verificar
            list_remove_element(procesos_blocked_suspend,proceso);
            proceso->metricas_tiempo[SUSP_BLOCKED]+=calculo_tiempo_proceso_en_estado(proceso);
            pthread_mutex_unlock(&mutex_lista_blocked_suspend);
        break;
        case(SUSP_READY):
            pthread_mutex_lock(&mutex_lista_ready_suspend);
            //falta verificar
            list_remove_element(procesos_ready_suspend,proceso);
            proceso->metricas_tiempo[SUSP_READY]+=calculo_tiempo_proceso_en_estado(proceso);
            pthread_mutex_unlock(&mutex_lista_ready_suspend);
        break;
        default:
            log_error(kernel_logger,"el estado es incorrecto");
        break;
    }
}
void agregar_proceso_a_estado(pcb_t* proceso,EstadoProceso estado){
    // log_debug(kernel_logger, "Se lllega a la funion para pasar de un estado a otro");
    switch(estado){
        case(NEW):
            pthread_mutex_lock(&mutex_lista_new);
            //falta verificar
            agregar_proceso_a_new(proceso);
            proceso->metricas_estado[NEW]++;
            clock_gettime(CLOCK_MONOTONIC, &proceso->tiempo_entrada_a_estado); //tiempo de entrada
            pthread_mutex_unlock(&mutex_lista_new);
        break;
        case(READY):
            pthread_mutex_lock(&mutex_lista_ready);
            list_add(procesos_ready,proceso);
            proceso->metricas_estado[READY]++;
            clock_gettime(CLOCK_MONOTONIC, &proceso->tiempo_entrada_a_estado);
            if(strcmp(ALGORITMO_CORTO_PLAZO,"SJF") == 0 || strcmp(ALGORITMO_CORTO_PLAZO,"SRT") == 0)
                ordenar_por_menor_estimacion();
            pthread_mutex_unlock(&mutex_lista_ready);
            verificarAlgoritmoSRT();
        break;
        case(EXEC):
            pthread_mutex_lock(&mutex_lista_exec);
            //falta verificar
            list_add(procesos_exec,proceso);
            proceso->metricas_estado[EXEC]++;
            clock_gettime(CLOCK_MONOTONIC, &proceso->tiempo_entrada_a_estado);
            pthread_mutex_unlock(&mutex_lista_exec);
        break;
        case(BLOCKED):
            pthread_mutex_lock(&mutex_lista_blocked);
            //falta verificar
            list_add(procesos_blocked,proceso);
            proceso->metricas_estado[BLOCKED]++;
            clock_gettime(CLOCK_MONOTONIC, &proceso->tiempo_entrada_a_estado);
            pthread_mutex_unlock(&mutex_lista_blocked);

            if(proceso->motivoDeBloqueo == IO){
                //aca se implementa el planificador de mediazoPlazo
                pthread_t hilo_planificador_mediano_plazo;
                pthread_create(&hilo_planificador_mediano_plazo,NULL,manejador_de_cambio_de_procesos_a_disco,(void *)proceso);
                pthread_detach(hilo_planificador_mediano_plazo);
            }
        break;
        case(EXIT):
            pthread_mutex_lock(&mutex_lista_exit);
            //falta verificar
            list_add(procesos_exit,proceso);
            proceso->metricas_estado[EXIT]++;
            clock_gettime(CLOCK_MONOTONIC, &proceso->tiempo_entrada_a_estado);
            pthread_mutex_unlock(&mutex_lista_exit);
        break;
        case(SUSP_BLOCKED):
            pthread_mutex_lock(&mutex_lista_blocked_suspend);
            //falta verificar
            //list_add(procesos_blocked_suspend,proceso);
            list_add(procesos_blocked_suspend,proceso);
            proceso->metricas_estado[SUSP_BLOCKED]++;
            clock_gettime(CLOCK_MONOTONIC, &proceso->tiempo_entrada_a_estado);
            pthread_mutex_unlock(&mutex_lista_blocked_suspend);
        break;
        case(SUSP_READY):
            pthread_mutex_lock(&mutex_lista_ready_suspend);
            //falta verificar
            agregar_proceso_a_susp_ready(proceso);
            proceso->metricas_estado[SUSP_READY]++;
            clock_gettime(CLOCK_MONOTONIC, &proceso->tiempo_entrada_a_estado);
            pthread_mutex_unlock(&mutex_lista_ready_suspend);
        break;
        default:
            log_error(kernel_logger,"el estado es incorrecto ESTADO: %d",estado);
         break;
    }
}
void agregar_proceso_a_new(pcb_t* proceso){
    //dependiendo de que algoritmo de plani alla, el ordenamiento va a ser distinto
    if(strcmp(ALGORITMO_INGRESO_A_READY, "FIFO")==0){
        list_add(procesos_new, proceso);
    }else if (strcmp(ALGORITMO_INGRESO_A_READY,"PMCP")==0){
        list_add_sorted(procesos_new, proceso, comparador_proceso_mas_chico);
    }else{
        log_error(kernel_logger,"nombe invalido de ALGORITMO_COLA_NEW");
        return;
    }
}
void agregar_proceso_a_susp_ready(pcb_t* proceso){
    //dependiendo de que algoritmo de plani alla, el ordenamiento va a ser distinto
    if(strcmp(ALGORITMO_INGRESO_A_READY, "FIFO")==0){
        list_add(procesos_ready_suspend, proceso);
    }else if (strcmp(ALGORITMO_INGRESO_A_READY,"PMCP")==0){
        list_add_sorted(procesos_ready_suspend, proceso, comparador_proceso_mas_chico);
    }else{
        log_error(kernel_logger,"nombe invalido de ALGORITMO_COLA_NEW");
        return;
    }
}
//MANEJO DE SENIALES 
//para cuando llegue un control + c por consola termine correctamente la ejecucuion
void manejar_ctrl_c(int sig) {
    switch (sig)
    {
    case SIGINT:
        terminar =1;
        finalizar_kernel();
        log_debug(kernel_logger,"\n[Kernel] Señal SIGINT capturada. Finalizando...\n");
        break;
    
    default:
        log_warning(kernel_logger,"senial no manejada");
        break;
    }
     exit(EXIT_SUCCESS);
}

double calcular_estimacion_proxima_rafaga(pcb_t* proceso){
    // PROXIMA_ESTIMACION = ALFA* (LO_QUE_REALMENTE_EJECUTO) + (1-ALFA)* ESTIMACION_ANTERIOR
    double estimacion = ALFA*(proceso->tiempo_antes_de_syscall) + (1-ALFA)*(proceso->estimacion_proxima_rafaga);

    return estimacion;

}

void* manejador_de_cambio_de_procesos_a_disco(void * arg) {
    pcb_t* proceso_bloc = (pcb_t *) arg;
    log_debug(kernel_logger,"EMPEZO CON EL TIMMERRRRR");

    struct timespec ts, fin , inicio;
    clock_gettime(CLOCK_REALTIME, &inicio);
    
    clock_gettime(CLOCK_REALTIME, &ts);

    ts.tv_sec += (TIEMPO_SUSPENSION / 1000);
    ts.tv_nsec += ((TIEMPO_SUSPENSION % 1000) * 1000000);
    
    // Normalizar los nanosegundos en caso de overflow
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_sec += 1;
        ts.tv_nsec -= 1000000000;
    }

    sem_post(&proceso_bloc->sem_activar_suspension); //para sincronizar hilos

    int resultado = sem_timedwait(&proceso_bloc->sem_cancelar_suspension, &ts); 
    //agrego para verificar cuanto espera 
    clock_gettime(CLOCK_REALTIME, &fin); 

    // Calcular diferencia en milisegundos
    long tiempo_esperado_ms = (fin.tv_sec - inicio.tv_sec) * 1000 + 
                              (fin.tv_nsec - inicio.tv_nsec) / 1000000;

    if (resultado == -1 && errno == ETIMEDOUT) {
        // Se cumplió el tiempo y aún sigue bloqueado
        if (sigue_proceso_bloqueado(proceso_bloc)) {
            pthread_mutex_lock(&proceso_bloc->mutex_esta_siendo_suspendido);
            mensaje_para_memoria(proceso_bloc->id, SUSPENDER_PROCESO);
            pasar_proceso_de_estado_a_otro(proceso_bloc, BLOCKED, SUSP_BLOCKED);
            pthread_mutex_unlock(&proceso_bloc->mutex_esta_siendo_suspendido);
            log_info(kernel_logger, "## (<%d>) Pasa del estado <BLOCKED> al estado <SUSP_BLOCKED>", proceso_bloc->id);

            pthread_mutex_lock(&mutex_agotamiento_memoria);
            bool rta = se_agoto_memoria_alguna_vez;
            pthread_mutex_unlock(&mutex_agotamiento_memoria);
            if(rta){     
                sem_post(&espacio_para_proceso); 
            }
        }
    } else {
        // log_debug(kernel_logger, "## (<%d>) Finalizó antes de suspender (respuesta de I/O)", proceso_bloc->id);
    }
    log_debug(kernel_logger, "## (<%d>) Esperó %ld ms antes de salir de sem_timedwait", proceso_bloc->id, tiempo_esperado_ms);

    return NULL;
}


bool sigue_proceso_bloqueado(pcb_t* proceso_bloc) {
    bool resultado;

    bool tiene_mismo_id(void* elem) {
    pcb_t* proceso = (pcb_t*) elem;
    return proceso->id == proceso_bloc->id;
    }

    pthread_mutex_lock(&mutex_lista_blocked);
    resultado = (list_any_satisfy(procesos_blocked, tiene_mismo_id)); 
    pthread_mutex_unlock(&mutex_lista_blocked);

    return resultado;
    
}

void mensaje_para_memoria(int id, op_code codigo) {

    pthread_mutex_lock(&mutex_conexion_memoria_nuevo);
    int conexion = crear_conexion(IP_MEMORIA,PUERTO_MEMORIA);
    enviar_hand_shake_kernel_mem(HANDSHAKE_MEM_KERNEL, conexion);
    log_info(kernel_logger,"## Se conecto kernel a memoria");
    
    t_buffer* buffer = crear_buffer();
    cargar_entero_al_buffer(buffer, id);
    t_paquete* paquete = crear_paquete(codigo,buffer);
    enviar_paquete(paquete, conexion);
    eliminar_paquete(paquete);
    pthread_mutex_unlock(&mutex_conexion_memoria_nuevo);
}


//PARA EL PLANIFICADOR DE CORTO PLAZO 
void* espera_rta_cpu(void* arg){
    cpu_conectada* cpu = (cpu_conectada *)arg;
    log_debug(kernel_logger,"Se lanza hilo de espera de rta de CPU: %d",cpu->id);

    while(1){
    int codigo = recibir_operacion(cpu->fd_dispatch);
    log_debug(kernel_logger,"Se recibio codigo de operacion: %d de la CPU %d",codigo, cpu->id);
   
    switch (codigo){
        case (SOL_EXIT_CPU_KER):
            t_buffer* buffer_sol_exit_cpu_ker = recibir_todo_un_buffer(cpu->fd_dispatch);
            int pid = extraer_int_del_buffer(buffer_sol_exit_cpu_ker);
            destruir_buffer(buffer_sol_exit_cpu_ker);
            //busca el proceso con ese pid en procesos en EXEC y lo saco de EXEC
            pcb_t* proceso = buscar_proceso_en_exec_con_pid(pid);
            if (proceso==NULL){
                log_debug(kernel_logger,"no se encontro el proceso en EXEC, en SOL_EXIT_CPU_KERNEL");
            }

            SISCALL_EXIT(proceso);

            //SE MARCA LA CPU COMO LIBRE 
            pthread_mutex_lock(&mutex_cpus_conectadas);
            cpu->estado=LIBRE;
            pthread_mutex_unlock(&mutex_cpus_conectadas);
            //se avisa que hay una cpu disponible 
            sem_post(&sem_hay_cpus_disponibles);
        break;

        case(SOL_IO_CPU_KER):
            t_buffer* buffer = recibir_todo_un_buffer(cpu->fd_dispatch);
            int pid_sol_io_cpu_ker = extraer_int_del_buffer(buffer);
            char* nombre_io = extraer_string_del_buffer(buffer);
            double tiempo = extraer_double_del_buffer(buffer);
            int pc_actualizado = extraer_int_del_buffer(buffer);
            destruir_buffer(buffer);

            pcb_t * proceso_sol_io_cpu_ker = buscar_proceso_en_exec_con_pid(pid_sol_io_cpu_ker);
            actualizar_pc_de_proceso(proceso_sol_io_cpu_ker, pc_actualizado);
            double tiempo_ejecutado = temporal_gettime(proceso_sol_io_cpu_ker->cronometro);
            temporal_destroy(proceso_sol_io_cpu_ker->cronometro);
            proceso_sol_io_cpu_ker->tiempo_antes_de_syscall = tiempo_ejecutado;
            proceso_sol_io_cpu_ker->tiempo_restante_rafaga = 0;
            proceso_sol_io_cpu_ker->desalojado = false;
            if (proceso_sol_io_cpu_ker==NULL){
                log_debug(kernel_logger,"no se encontro el proceso en EXEC, en SOL_IO_CPU_KERNEL"); 
            }
            log_debug(kernel_logger,"recibi de cpu el pid %d nombre %s tiempo %f", pid_sol_io_cpu_ker, nombre_io, tiempo);
            SISCALL_IO(proceso_sol_io_cpu_ker,nombre_io,tiempo);

            //SE MARCA LA CPU COMO LIBRE 
            pthread_mutex_lock(&mutex_cpus_conectadas);
            cpu->estado=LIBRE;
            pthread_mutex_unlock(&mutex_cpus_conectadas);
            //se avisa que hay una cpu disponible 
            free(nombre_io);
            sem_post(&sem_hay_cpus_disponibles);
        break;

        case SOL_INIT_PROC_CPU_KER: 
            log_debug(kernel_logger,"Se recibio SOL_INIT_PROC_CPU_KER de la CPU %d", cpu->id);
            t_buffer* buffer_inic_proc_cpu_ker = recibir_todo_un_buffer(cpu->fd_dispatch);
            char* path_pseudocodigo = extraer_string_del_buffer(buffer_inic_proc_cpu_ker);
            int tamanio = extraer_int_del_buffer(buffer_inic_proc_cpu_ker);
            int pid_sol_init_proc = extraer_int_del_buffer(buffer_inic_proc_cpu_ker);
            int pc_actualizado_init_proc = extraer_int_del_buffer(buffer_inic_proc_cpu_ker);

            pcb_t * proceso_sol_init_proc = buscar_proceso_en_exec_con_pid(pid_sol_init_proc);
            actualizar_pc_de_proceso(proceso_sol_init_proc, pc_actualizado_init_proc);

            destruir_buffer(buffer_inic_proc_cpu_ker);

            // log_debug(kernel_logger,"path recibido es %s",path_pseudocodigo);

            SYSCALL_INIT_PROC(path_pseudocodigo,tamanio);
            free(path_pseudocodigo);
            break;

        case(SOL_DUMP_MEMORY_CPU_KER):

            t_buffer* buffer_dump_memory = recibir_todo_un_buffer(cpu->fd_dispatch);
            int pid_proceso_exec = extraer_int_del_buffer(buffer_dump_memory);
            int pc_actualizado_dump = extraer_int_del_buffer(buffer_dump_memory);
            destruir_buffer(buffer_dump_memory);

            pcb_t* proceso_sol_dump_memory = buscar_proceso_en_exec_con_pid(pid_proceso_exec);

            if(proceso_sol_dump_memory==NULL){
                log_debug(kernel_logger,"No se encontro el proceso con PID %d en EXEC, en SOL_DUMP_MEMORY_CPU_KERNEL",proceso_sol_dump_memory->id);
            }
          
            actualizar_pc_de_proceso(proceso_sol_dump_memory,pc_actualizado_dump);
            double tiempo_ejecutado_antes_de_dump = temporal_gettime(proceso_sol_dump_memory->cronometro);
            temporal_destroy(proceso_sol_dump_memory->cronometro);
            proceso_sol_dump_memory->tiempo_antes_de_syscall = tiempo_ejecutado_antes_de_dump;
            proceso_sol_dump_memory->tiempo_restante_rafaga = 0;
            proceso_sol_dump_memory->desalojado = false;
            proceso_sol_dump_memory->motivoDeBloqueo = MEMORY_DUMP;
           

            //SISCALL_DUMP_MEMORY(proceso_sol_dump_memory);
            pthread_mutex_lock(&mutex_cpus_conectadas);
            cpu->estado=LIBRE;
            pthread_mutex_unlock(&mutex_cpus_conectadas);
            //se avisa que hay una cpu disponible 
            sem_post(&sem_hay_cpus_disponibles);
            
            pthread_t hilo_atender_dump_memory;
            pthread_create(&hilo_atender_dump_memory,NULL,SISCALL_DUMP_MEMORY,(void *)proceso_sol_dump_memory);
            pthread_detach(hilo_atender_dump_memory);
        break;
        case(ACTUALIZAR_CONTEXTO): // Se usa solo para el contexto por desalojo
            t_buffer* bufferr = recibir_todo_un_buffer(cpu->fd_dispatch);
            int pc_ac = extraer_int_del_buffer(bufferr);
            int pid_ac = extraer_int_del_buffer(bufferr);
            destruir_buffer(bufferr);

            pthread_mutex_lock(&mutex_cpus_conectadas);
            cpu->estado=LIBRE;
            pthread_mutex_unlock(&mutex_cpus_conectadas);
            sem_post(&sem_hay_cpus_disponibles);

            log_debug(kernel_logger,"PC actualizado en kernel: %d, PID actualizado en kernel: %d",pc_ac,pid_ac);

            pcb_t * proceso_ac = buscar_proceso_en_exec_con_pid(pid_ac);

            actualizar_pc_de_proceso(proceso_ac, pc_ac);
            if(proceso_ac->desalojado){
                pasar_proceso_de_estado_a_otro(proceso_ac,EXEC,READY);
                log_debug(kernel_logger,"Le queda rafaga para ejecutar %f",proceso_ac->tiempo_restante_rafaga);
                temporal_destroy(proceso_ac->cronometro);
                proceso_ac->desalojado=false;
                
                log_info(kernel_logger,"## (<%d>) Pasa del estado <EXEC> al estado <READY>",proceso_ac->id);
                log_info(kernel_logger,"## (<%d>) - Desalojado por algoritmo SJF/SRT", proceso_ac->id);
                sem_post(&sem_hay_procesos_en_ready);
                // pthread_mutex_lock(&mutex_cpus_conectadas);
                // cpu->estado=LIBRE;
                // pthread_mutex_unlock(&mutex_cpus_conectadas);
                // sem_post(&sem_hay_cpus_disponibles);
            }   
        break;   
        case(-1):
            log_error(kernel_logger,"se desconecto CPU");
            break;
        
        default:
            log_info(kernel_logger,"accion desconocida de CPU");
        break;
    }

}
    return NULL;
}


pcb_t* buscar_proceso_en_exec_con_pid(int pid){
    bool mismo_id(void* ptr){
        pcb_t* pcb = (pcb_t*) ptr;
        return pcb->id == pid;
    }

    log_debug(kernel_logger,"El proceso que se esta buscando es: %d", pid);

    pthread_mutex_lock(&mutex_lista_exec);
    pcb_t* pcb = (pcb_t*) list_find(procesos_exec, mismo_id);
    pthread_mutex_unlock(&mutex_lista_exec);
    /*
    if(pcb==NULL){
        log_error(kernel_logger,"no se encontro el proceso que se queria remover");
    }
    */
    return pcb;
}
pcb_t* buscar_proceso_en_bloqued_con_pid(int pid){
    bool mismo_id(void* ptr){
        pcb_t* pcb = (pcb_t*) ptr;
        return pcb->id == pid;
    }
    pthread_mutex_lock(&mutex_lista_blocked);
    pcb_t* pcb = (pcb_t*) list_find(procesos_blocked, mismo_id);
    pthread_mutex_unlock(&mutex_lista_blocked);
    if(pcb==NULL){
        //log_error(kernel_logger,"no se encontro el proceso que se queria remover");
        pthread_mutex_lock(&mutex_lista_blocked_suspend);
        pcb = (pcb_t*) list_find(procesos_blocked_suspend, mismo_id);
        pthread_mutex_unlock(&mutex_lista_blocked_suspend);
    }
    if(pcb==NULL){
            //log_error(kernel_logger,"no se encontro el proceso que se queria remover");
        pthread_mutex_lock(&mutex_lista_exit);
        pcb = (pcb_t*) list_find(procesos_exit, mismo_id);
        pthread_mutex_unlock(&mutex_lista_exit);
    }
    if(pcb==NULL){
        //log_error(kernel_logger,"no se encontro el proceso que se queria remover");
        pthread_mutex_lock(&mutex_lista_ready_suspend);
        pcb = (pcb_t*) list_find(procesos_ready_suspend, mismo_id);
        pthread_mutex_unlock(&mutex_lista_ready_suspend);
    }
    if(pcb==NULL){
        //log_error(kernel_logger,"no se encontro el proceso que se queria remover");
        pthread_mutex_lock(&mutex_lista_ready);
        pcb = (pcb_t*) list_find(procesos_ready, mismo_id);
        pthread_mutex_unlock(&mutex_lista_ready);
    }
    // else
    //     log_debug(kernel_logger,"El proceso que se encontro en EXEC tiene PID : %d",pcb->id);
    return pcb;
}

pcb_t* buscar_proceso_en_ready_con_pid(int pid){
    bool mismo_id(void* ptr){
        pcb_t* pcb = (pcb_t*) ptr;
        return pcb->id == pid;
    }
    pthread_mutex_lock(&mutex_lista_ready);
    pcb_t* pcb = (pcb_t*) list_find(procesos_ready, mismo_id);
    pthread_mutex_unlock(&mutex_lista_ready);
    if(pcb==NULL){
        pthread_mutex_lock(&mutex_lista_exit);
        pcb = (pcb_t*) list_find(procesos_exit, mismo_id);
        pthread_mutex_unlock(&mutex_lista_exit);
    }
    return pcb;
}
    

void actualizar_pc_de_proceso(pcb_t* proceso_en_exec,int pc){
    pthread_mutex_lock(&proceso_en_exec->mutex_pc);
    proceso_en_exec->pc = pc;
    pthread_mutex_unlock(&proceso_en_exec->mutex_pc);
}

void actualizar_pc_de_proceso_bloc(pcb_t* proceso_en_blocked,int pc){
    pthread_mutex_lock(&proceso_en_blocked->mutex_pc);
    proceso_en_blocked->pc = pc;
    pthread_mutex_unlock(&proceso_en_blocked->mutex_pc);
}

void actualizar_pc_de_proceso_ready(pcb_t* proceso_en_ready,int pc){
    pthread_mutex_lock(&proceso_en_ready->mutex_pc);
    proceso_en_ready->pc = pc;
    pthread_mutex_unlock(&proceso_en_ready->mutex_pc);
}

void verificarAlgoritmoSRT(){
    if(strcmp(ALGORITMO_CORTO_PLAZO,"SRT") == 0){
        
        if(cpu_no_disponible()){

            log_debug(kernel_logger,"Entro al comparador de SRT");

            // Busco  el proceso con mayor estimacion
            pcb_t* proceso_a_desalojar = proceso_largo_en_exec();

            if(proceso_a_desalojar != NULL){
                log_debug(kernel_logger,"el proceso encontrado tiene pid %d",proceso_a_desalojar->id);
                desalojar_proceso(proceso_a_desalojar);
                proceso_a_desalojar->desalojado = true;
            }
        }

    }
}

bool cpu_no_disponible(){
    int valor;

    sem_getvalue(&sem_hay_cpus_disponibles,&valor);
    return valor==0;
}

void cancelar_timer_de_supension(pcb_t* proceso_actual){
    if(!esta_suspendido(proceso_actual)){
        sem_post(&proceso_actual->sem_cancelar_suspension);
    }
}


bool esta_suspendido(pcb_t* proceso_actual) {
    bool resultado;

    bool tiene_mismo_id(void* elem) {
    pcb_t* proceso = (pcb_t*) elem;
    return proceso->id == proceso_actual->id;
    }

    pthread_mutex_lock(&mutex_lista_blocked_suspend);
    resultado = (list_any_satisfy(procesos_blocked_suspend, tiene_mismo_id)); 
    pthread_mutex_unlock(&mutex_lista_blocked_suspend);

    return resultado;   
}

void ordenar_por_menor_estimacion(){
    bool menor_estimacion(void* a, void* b) {
        pcb_t* procesoA = (pcb_t*) a;
        pcb_t* procesoB = (pcb_t*) b;
        if(procesoA->estimacion_proxima_rafaga != procesoB->estimacion_proxima_rafaga)
            return procesoA->estimacion_proxima_rafaga < procesoB->estimacion_proxima_rafaga;
        else
            return procesoA->id < procesoB->id;
    };

    list_sort(procesos_ready,menor_estimacion);
}