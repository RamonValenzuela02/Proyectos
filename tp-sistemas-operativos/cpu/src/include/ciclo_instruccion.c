
#include "ciclo_instruccion.h"


volatile sig_atomic_t apagar_cpu = 0;  
pthread_mutex_t mutex_var_global_syscall;
pthread_mutex_t semaforo_pc;

t_proceso* proceso = NULL;
t_proceso* proceso_anterior = NULL;
t_instruccion* instruccion;

bool hay_syscall = false;
bool hay_desalojo = false;
bool instruccion_io = false;
int pid_a_desalojar = -1;

void incializar_proceso_anterior(){
    proceso_anterior = malloc(sizeof(t_proceso));
    proceso_anterior->pid = -20;
}

void atender_cpu_dispatch() {
  
    recibirProceso(); // proceso es = al proceso en ejecucion

    if (proceso == NULL) {
        log_error(cpu_logger, "No se pudo recibir el proceso");
        exit(EXIT_FAILURE);
    }


    log_info(cpu_logger, "PID ejecutando: %d PC ejecutando: %d ",proceso->pid,proceso->pc);

    if(proceso->pid != proceso_anterior->pid){
        if(ENTRADAS_CACHE>0 && proceso_anterior->pid != -20){
        // enviar_paginas_modificadas_a_memoria(proceso_anterior->pid); // lo voy a ejecutar despues antes d ehcaer una io porque sino me suspende el proceso y luego me envia direcciones fisicas que nada que ver
        liberar_cache(cache);
        liberar_tlb(tlb);
        }
    }

    log_debug(cpu_logger, "Iniciando ejecucion del proceso");
    ejecutar_proceso(); 
}



void ejecutar_proceso(){
    while (!apagar_cpu )
    {
        fetch();

        hay_syscall = decode(); // se activa el hay_sysacall 

        if(hay_syscall){
            liberar_instruccion();
            liberar_ejecucion();

            pthread_mutex_lock(&mutex_var_global_syscall);
            hay_syscall = false;
            pthread_mutex_unlock(&mutex_var_global_syscall);  
            log_debug(cpu_logger, "Se detecto una syscall, finalizando ejecucion del proceso ...");
            break;    
        }

        if (instruccion->operacion == NULL)
        {
            log_error(cpu_logger, "No se pudo obtener la instruccion");
            exit(EXIT_FAILURE);
        }

        execute();

        liberar_instruccion();
        log_debug(cpu_logger, "Iniciando check_interrupt");
        if (!checkInterrupt())
        {
            break;
        }
        log_debug(cpu_logger, "Fin de check_interrupt");
    }
    atender_cpu_dispatch();
}

void recibirProceso(){
    
    log_debug(cpu_logger,"esperando proceso");

    int codigo = recibir_operacion(fd_conexion_dispatch);

    log_info(cpu_logger, "Se esta recibiendo el cod op: %d ", codigo);

    if(codigo != EJECUCION_DE_PROCESO_KER_CPU)
    {
        log_error(cpu_logger, "No se recibio EJECUCION_DE_PROCESO_KER_CPU para recibir el proceso a ejecutar");
        // exit(EXIT_FAILURE);
    }

    t_buffer* bufferEjecucion = recibir_todo_un_buffer(fd_conexion_dispatch);
    proceso = malloc(sizeof(t_proceso));
    proceso->pid = extraer_int_del_buffer(bufferEjecucion);
    proceso->pc = extraer_int_del_buffer(bufferEjecucion);
    log_info(cpu_logger, "PID: %d PC:%d ",proceso->pid, proceso->pc);

    eliminar_buffer(bufferEjecucion);
    
}


void fetch(){
    instruccion = malloc(sizeof(t_instruccion));
    instruccion->args = malloc(sizeof(t_argumentos_genericos));
    instruccion->args->dispositivo = NULL;
    instruccion->args->archivo = NULL;
    instruccion->args->datos = NULL;
    if (instruccion->args == NULL) {
        log_error(cpu_logger, "INSTRUCCION->ARGS ES NULL AL ENTRAR A asignarParametros");
    }    
    log_info(cpu_logger, "## PID:%d - FETCH - Program Counter: %d", proceso->pid, proceso->pc); // LOG OBLIGATORIO 
    solicitarinstruccionAMemoria(proceso->pid,proceso->pc);
// t_instruccion* instruccionAEjecutar = recibirinstruccionAMemoria(proceso->pc,proceso->pid);
}

void execute(){
    if(strcmp(instruccion->operacion,"NOOP") == 0){
        // usleep(1000 * 1000); lo comento porque el tiempo de ejecucion seria hacer todo el ciclo de ejecucion, no un retardo
        log_info(cpu_logger, "## PID:%d - Ejecutando: NOOP", proceso->pid);
        proceso->pc++;
    }
    else if (strcmp(instruccion->operacion,"WRITE") == 0){

        log_info(cpu_logger, "## PID:%d - Ejecutando: WRITE - %d %s",
             proceso->pid,
             instruccion->args->direccion_logica,
             instruccion->args->datos);
   
        direccion_fisica = traducir_direccion_logica(proceso->pid, instruccion);
        
        if(direccion_fisica != -1){
            
            log_info(cpu_logger, "PID: %d - Acción: ESCRIBIR - Dirección Física: %d - Valor: %s",
                proceso->pid,
                direccion_fisica,
                instruccion->args->datos);
        }
      
       //  valor_leido_memoria = instruccion->args->datos; lo comente porque esto es lo mismo que usar directamente instruccion->args->datos

        proceso->pc++;

    }else if (strcmp(instruccion->operacion,"READ") == 0){

        log_info(cpu_logger, "## PID:%d - Ejecutando: READ - %d %d",
                proceso->pid,
                instruccion->args->direccion_logica,
                instruccion->args->tamanio);

        direccion_fisica = traducir_direccion_logica(proceso->pid, instruccion);

        if(direccion_fisica != -1){
            log_info(cpu_logger, "PID: %d - Acción: LEER - Dirección Física: %d - Valor: %s",
                proceso->pid,
                direccion_fisica,
                valor_leido_memoria);
        }
            proceso->pc++;

    }else if (strcmp(instruccion->operacion,"GOTO") == 0){

        proceso->pc = instruccion->args->valor;
        log_info(cpu_logger, "## PID:%d - EXECUTE - GOTO %d", proceso->pid, instruccion->args->valor);

    }
}

bool checkInterrupt() {
    pthread_mutex_lock(&mutex_var_global_syscall);
    bool hay_desalojo_aux = hay_desalojo;
    pthread_mutex_unlock(&mutex_var_global_syscall);

    if (hay_desalojo_aux && proceso->pid==pid_a_desalojar) {
        enviar_contexto_a_kernel();
        pthread_mutex_lock(&mutex_var_global_syscall);
        hay_desalojo = false;
        pid_a_desalojar = -1;
        pthread_mutex_unlock(&mutex_var_global_syscall);

        liberar_ejecucion();
        atender_cpu_dispatch();
        return false;

    }else{
        pid_a_desalojar = -1;
        return true;
    }
     
}


void* atender_cpu_interrupt(void* args) {
    while (!apagar_cpu) {
        // int res = recv(fd_conexion_interrupt, &motivo, sizeof(int), MSG_WAITALL);
        int res = recibir_operacion(fd_conexion_interrupt);
        t_buffer* buffer=recibir_todo_un_buffer(fd_conexion_interrupt);
        pid_a_desalojar = extraer_int_del_buffer(buffer);
        destruir_buffer(buffer);
        if (res <= 0) {
            log_error(cpu_logger, "Se cerró la conexión con el Kernel.");
            exit(EXIT_FAILURE);
        }

        log_warning(cpu_logger, "## Llega interrupción al puerto Interrupt!!");
        log_warning(cpu_logger, "Se va a desalojar al PID %d !!",pid_a_desalojar);
        log_debug(cpu_logger,"Se recibio el codigo %d",res);

        pthread_mutex_lock(&mutex_var_global_syscall);
        hay_desalojo = true;
        pthread_mutex_unlock(&mutex_var_global_syscall);
    }
}

void liberar_ejecucion(){

free(proceso);

};

void liberar_instruccion(){

        if (instruccion == NULL) return;
        if (instruccion->args != NULL) {
            
            if (instruccion->args->dispositivo != NULL) {
              
                free(instruccion->args->dispositivo);
                instruccion->args->dispositivo = NULL;
            }
          

            if (instruccion->args->archivo != NULL) {
                free(instruccion->args->archivo);
                instruccion->args->archivo = NULL;
            }
            if (instruccion->args->datos != NULL) {
                free(instruccion->args->datos);
                instruccion->args->datos = NULL;
            }
    
            free(instruccion->args);
            instruccion->args = NULL;
        }
    
        if (instruccion->operacion != NULL) {
            free(instruccion->operacion);
            instruccion->operacion = NULL;
        }
    
        free(instruccion);
        instruccion = NULL;
}

void enviar_contexto_a_kernel(){
   

    t_buffer* buffer = crear_buffer();

    cargar_entero_al_buffer(buffer,proceso->pc);
    cargar_entero_al_buffer(buffer,proceso->pid);
    
    t_paquete* paquete = crear_paquete(ACTUALIZAR_CONTEXTO,buffer);

    log_debug(cpu_logger,"Enviando paquete para ACTUALIZAR CONTEXTO a Kernel");

    enviar_paquete(paquete, fd_conexion_dispatch);
    // destruir_buffer(buffer);
    eliminar_paquete(paquete);

    proceso_anterior->pid = proceso->pid; // una vez se envia el proceso 

    log_debug(cpu_logger, "Enviando contexto del proceso a Kernel: PID: %d PC actualizado: %d", proceso->pid, proceso->pc);
}





bool decode(){

    int flag_syscall;
 
    t_buffer* buffer;
    int cod_op = recibir_operacion(fd_conexion_memoria);

    if(cod_op == OBTENER_INSTRUCCION_RTA){
        log_debug(cpu_logger, "Se recibe la instruccion de Memoria ... ");
        // buffer = recibir_todo_un_buffer(fd_conexion_memoria);
    }else{
        log_debug(cpu_logger, "Se recibio una operacion incorrecta %d ", cod_op);
        exit(EXIT_FAILURE);
    }

    buffer = recibir_todo_un_buffer(fd_conexion_memoria);
    if (!buffer){
        perror("No se pudo recibir el buffer");
        return NULL;
    }
   
    char* codigoDeOperacion = extraer_string_del_buffer(buffer);

    if(strcmp(codigoDeOperacion,"IO")==0)
        instruccion_io = true;

    instruccion->operacion = codigoDeOperacion; 

    log_debug(cpu_logger, "Se recibio instruccion: %s", instruccion->operacion);

    flag_syscall = mandar_syscall_a_ker(instruccion->operacion,buffer);

    // log_debug(cpu_logger, "Se detecto syscall: %d", flag_syscall);

    if(!flag_syscall){
        log_debug(cpu_logger, "No se detecto syscall, se asignan los parametros de la instruccion");
        asignarParametros(instruccion, buffer);
    }
        
    return flag_syscall;

}

void solicitarinstruccionAMemoria(int pid, int pc){
    t_buffer* buffer = crear_buffer();
    cargar_entero_al_buffer(buffer, pid);
    cargar_entero_al_buffer(buffer, pc);
    t_paquete* paquete = crear_paquete(GET_INSTRUCCION, buffer);
    enviar_paquete(paquete, fd_conexion_memoria);
    eliminar_paquete(paquete);

    // log_debug(cpu_logger, "Se solicita instruccion a memoria para PID: %d PC: %d", pid, pc);
}



void enviar_paginas_modificadas_a_memoria(int pid){
    for (int i = 0; i < list_size(cache); i++) {
        entrada_cache* entrada = list_get(cache, i);

        if(entrada->bit_modificado){
            log_info(cpu_logger, "Página modificada. PID: %d - Dirección Física: %d - Valor: %s",
                     proceso_anterior->pid, entrada->direccion_fisica, entrada->valor);

            enviar_valor_a_memoria(entrada->direccion_fisica, entrada->valor,proceso->pid);
        }
    }
}


void enviar_valor_a_memoria(int direccion_fisica, char* valor, int pid_de_pagina_a_actualizar){
    t_buffer* buffer = crear_buffer();
    cargar_entero_al_buffer(buffer, pid_de_pagina_a_actualizar);
    cargar_entero_al_buffer(buffer, direccion_fisica);
    cargar_choclo_al_buffer(buffer, valor,TAM_PAGINA);

    t_paquete* paquete = crear_paquete(ACTUALIZAR_PAGINA_COMPLETA, buffer);
    enviar_paquete(paquete, fd_conexion_memoria);
    eliminar_paquete(paquete);

    log_debug(cpu_logger, "Me quedo esperando OK de Memoria..");
    op_code cod_op = recibir_operacion(fd_conexion_memoria);
    if (cod_op == OK) {
        log_debug(cpu_logger,"se actualizo la pagina en memoria, recibie el OK de Memoria");
    }
        
}


const char* op_codeAstring(e_tipo_instruccion operacion) {
    switch (operacion) {
    case NOOP:
        return "NOOP";
    case WRITE:
        return "WRITE";
    case READ:
        return "READ";
    case GOTO:
        return "GOTO";
    case IO:
        return "IO";
    case INIT_PROC:
        return "INIT_PROC";
    case INST_DUMP_MEMORY:
        return "DUMP_MEMORY";
    case EXIT:
        return "EXIT";
    default:
        return "INSTRUCCION_DESCONOCIDA";
    }
}

void asignarParametros(t_instruccion* instruccion, t_buffer* buffer){

    if(strcmp(instruccion->operacion,"NOOP") == 0){
        instruccion->args = NULL;
    }else if(strcmp(instruccion->operacion,"WRITE") == 0){
        int direccion_logica = extraer_int_del_buffer(buffer);
        char* datos = extraer_string_del_buffer(buffer);

        instruccion->args->direccion_logica = direccion_logica;
        instruccion->args->datos= datos;
        instruccion->args->archivo = NULL;
        instruccion->args->dispositivo = NULL;
    }else if(strcmp(instruccion->operacion,"READ") == 0){
        int direccion_logicaR = extraer_int_del_buffer(buffer);
        int tamanioR = extraer_int_del_buffer(buffer);

        instruccion->args->direccion_logica = direccion_logicaR;
        instruccion->args->tamanio = tamanioR;
        instruccion->args->datos = NULL;
        instruccion->args->archivo = NULL;
        instruccion->args->dispositivo = NULL;

    }else if(strcmp(instruccion->operacion,"GOTO") == 0){
            int valor = extraer_int_del_buffer(buffer);

            instruccion->args->valor = valor;
            instruccion->args->datos = NULL;
            instruccion->args->archivo = NULL;
            instruccion->args->dispositivo = NULL;
        } 
    
        destruir_buffer(buffer);
}

bool mandar_syscall_a_ker(char* syscall, t_buffer* buffer){
    // log_debug(cpu_logger, "Se detecta syscall: %s", syscall);

    t_buffer* buffer_syscall = crear_buffer();

    if(strcmp(syscall,"IO") == 0){

        enviar_paginas_modificadas_a_memoria(proceso->pid);
        
        char* dispositivo = extraer_string_del_buffer(buffer);
        int tiempo = extraer_int_del_buffer(buffer);
        destruir_buffer(buffer);

        instruccion->args->dispositivo = dispositivo;
        instruccion->args->tiempo = tiempo;

        cargar_entero_al_buffer(buffer_syscall,proceso->pid);
        cargar_string_al_buffer(buffer_syscall,dispositivo);
        cargar_entero_al_buffer(buffer_syscall,tiempo);

        pthread_mutex_lock(&semaforo_pc);
        proceso->pc++;
        pthread_mutex_unlock(&semaforo_pc);

        cargar_entero_al_buffer(buffer_syscall,proceso->pc);

        t_paquete* paquete = crear_paquete(SOL_IO_CPU_KER,buffer_syscall);

        log_debug(cpu_logger,"Enviando syscall IO, PID %d, dispositivo %s, tiempo %d y pc actualizado %d",proceso->pid,dispositivo,tiempo,proceso->pc);

        enviar_paquete(paquete,fd_conexion_dispatch);
        eliminar_paquete(paquete);
        // enviar_contexto_a_kernel();

        proceso_anterior->pid = proceso->pid;

        log_info(cpu_logger, "## PID:%d - EXECUTE - IO %s %d ", proceso->pid, instruccion->args->dispositivo, instruccion->args->tiempo);

        return true;

    }else if(strcmp(syscall,"INIT_PROC") == 0){


        char* archivo = extraer_string_del_buffer(buffer);
    
        int tamanioArchivo = extraer_int_del_buffer(buffer);
  

        //destruir_buffer(buffer); erorr por doble free 

        instruccion->args->archivo = archivo;
        instruccion->args->tamanio = tamanioArchivo;



        cargar_string_al_buffer(buffer_syscall,archivo);
        cargar_entero_al_buffer(buffer_syscall,tamanioArchivo);
       
        
        pthread_mutex_lock(&semaforo_pc);
        proceso->pc++;
        pthread_mutex_unlock(&semaforo_pc);

        cargar_entero_al_buffer(buffer_syscall,proceso->pid);
        cargar_entero_al_buffer(buffer_syscall,proceso->pc);
        

        t_paquete* paquete = crear_paquete(SOL_INIT_PROC_CPU_KER,buffer_syscall);
        enviar_paquete(paquete,fd_conexion_dispatch);
        eliminar_paquete(paquete);

        
        // enviar_contexto_a_kernel();

        log_info(cpu_logger, "## PID:%d - EXECUTE - INIT_PROC %s %d ", proceso->pid,instruccion->args->archivo, instruccion->args->tamanio );
        //return true; 
        return false; //cambio para que siga ejecutando las instrucciones 

    }else if(strcmp(syscall,"DUMP_MEMORY") == 0){
        destruir_buffer(buffer);
        cargar_entero_al_buffer(buffer_syscall,proceso->pid);
        pthread_mutex_lock(&semaforo_pc);
        proceso->pc++;
        pthread_mutex_unlock(&semaforo_pc);
        cargar_entero_al_buffer(buffer_syscall,proceso->pc);
        t_paquete* paquete = crear_paquete(SOL_DUMP_MEMORY_CPU_KER,buffer_syscall);
        enviar_paquete(paquete,fd_conexion_dispatch);
        eliminar_paquete(paquete);
        // enviar_contexto_a_kernel();
        proceso_anterior->pid = proceso->pid;
        log_info(cpu_logger, "## PID:%d - EXECUTE - DUMP_MEMORY", proceso->pid);
        return true;

    }else if(strcmp(syscall,"EXIT") == 0){
        destruir_buffer(buffer);

        cargar_entero_al_buffer(buffer_syscall,proceso->pid);

        t_paquete* paquete = crear_paquete(SOL_EXIT_CPU_KER,buffer_syscall);
        enviar_paquete(paquete,fd_conexion_dispatch);
        eliminar_paquete(paquete);
        proceso_anterior->pid = proceso->pid;
        log_info(cpu_logger, "## PID:%d - EXECUTE - EXIT", proceso->pid);
        return true;
    }
    destruir_buffer(buffer_syscall);
    return false;
}
// void liberarInstruccionAnterior(){
//     if(proceso_anterior!=NULL){
//         free(proceso_anterior);
//     }
// }

