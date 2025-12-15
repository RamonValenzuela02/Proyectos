#include "include/inicializacion.h"

void iniciarKernel(){
    //seniales
    terminar = 0;
    //loggers
    kernel_logger = iniciar_logger("kernel.log","KERNEL_LOG");
    // kernel_logger = kernel_logger = log_create("kernel.log", "KERNEL_LOG", true, LOG_LEVEL_DEBUG); // comentado por las dudas
    //config
    //kernel_config = iniciar_config("./kernel.config");
    cargarConfigs();
    //listas
    cpus_conectadas = list_create();
    ios_conectadas = list_create();
    procesos_new = list_create();
    procesos_ready = list_create();
    procesos_exit = list_create();  
    procesos_exec = list_create();
    procesos_blocked = list_create();
    procesos_esperando_ios = list_create(); 
    procesos_blocked_suspend =list_create();
    procesos_ready_suspend = list_create(); 
    //servidores
    fd_server_dispatch = iniciar_servidor(PUERTO_ESCUCHA_DISPATCH, kernel_logger,"para DISPACTH iniciado");
    fd_server_interrupt =iniciar_servidor(PUERTO_ESCUCHA_INTERRUPT,kernel_logger,"para INTERRUPT iniciado");
    fd_server_io = iniciar_servidor(PUERTO_ESCUCHA_IO,kernel_logger,"para IOS iniciado");
    //variables
    id_pcb_generador=0;
    se_agoto_memoria_alguna_vez=false;
    // seNecesitaSeguirPreguntandoAMem = 0;
    //inicar mutex y semaforos
    sem_init(&sem_iniciar_planificacion,0,0);
    pthread_mutex_init(&mutex_lista_new,NULL);
    pthread_mutex_init(&mutex_lista_ready,NULL);
    pthread_mutex_init(&mutex_id_pcb_generador,NULL);
    pthread_mutex_init(&mutex_agotamiento_memoria,NULL);
    sem_init(&espacio_para_proceso,0,0);
    sem_init(&sem_proceso_bloqueado_por_io,0,0); 
    pthread_mutex_init(&mutex_procesos_esperando_ios,NULL);
    pthread_mutex_init(&mutex_ios_conectadas,NULL); 
    sem_init(&sem_hay_cpus_disponibles,0,0);   
    sem_init(&sem_hay_procesos_en_ready,0,0);  
    pthread_mutex_init(&mutex_lista_exec,NULL); 
    pthread_mutex_init(&mutex_cpus_conectadas,NULL); 
    pthread_mutex_init(&mutex_lista_exit,NULL); 
    pthread_mutex_init(&mutex_lista_blocked,NULL);
    sem_init(&sem_hay_procesos_esp_ser_ini_en_mem,0,0);
    sem_init(&sem_hay_procesos_en_exit,0,0); 
    sem_init(&se_conecta_cpu,0,0);
    pthread_mutex_init(&mutex_lista_blocked_suspend,NULL);
    pthread_mutex_init(&mutex_lista_ready_suspend,NULL);
    sem_init(&esperoAqueSeConectenTodasLasCpus,0,0);
    pthread_mutex_init(&mutex_conexion_memoria_nuevo, NULL);

}

void cargarConfigs(){
    IP_MEMORIA = config_get_string_value(kernel_config,"IP_MEMORIA"); 
    ALGORITMO_CORTO_PLAZO = config_get_string_value(kernel_config,"ALGORITMO_CORTO_PLAZO"); 
    PUERTO_MEMORIA = config_get_string_value(kernel_config,"PUERTO_MEMORIA");
    TIEMPO_SUSPENSION = config_get_int_value(kernel_config,"TIEMPO_SUSPENSION");
    LOG_LEVEL= config_get_string_value(kernel_config,"LOG_LEVEL");
    PUERTO_ESCUCHA_DISPATCH = config_get_string_value(kernel_config,"PUERTO_ESCUCHA_DISPATCH");
    PUERTO_ESCUCHA_INTERRUPT = config_get_string_value(kernel_config,"PUERTO_ESCUCHA_INTERRUPT");
    PUERTO_ESCUCHA_IO = config_get_string_value(kernel_config,"PUERTO_ESCUCHA_IO");
    ALGORITMO_INGRESO_A_READY = config_get_string_value(kernel_config,"ALGORITMO_INGRESO_A_READY");
    ALFA= config_get_double_value(kernel_config,"ALFA");
    ESTIMACION_INICIAL=config_get_int_value(kernel_config,"ESTIMACION_INICIAL");
}

//////////////FINALIZADOR 

void finalizar_kernel(){
    log_destroy(kernel_logger);
    // log_destroy(kernel_logger);
    //config
    config_destroy(kernel_config);
    //listas
    list_destroy_and_destroy_elements(cpus_conectadas,destruir_cpu_conectada);
    list_destroy_and_destroy_elements(ios_conectadas,destruir_io_conectada);
    list_destroy_and_destroy_elements(procesos_new,destruir_pcb);
    list_destroy_and_destroy_elements(procesos_ready,destruir_pcb);
    list_destroy_and_destroy_elements(procesos_exit,destruir_pcb);
    list_destroy_and_destroy_elements(procesos_exec,destruir_pcb);
    list_destroy_and_destroy_elements(procesos_blocked,destruir_pcb);
    list_destroy_and_destroy_elements(procesos_blocked_suspend,destruir_pcb);
    list_destroy_and_destroy_elements(procesos_ready_suspend,destruir_pcb);
    list_destroy_and_destroy_elements(procesos_esperando_ios,destruir_proceso_esperado_io);
    //servidores
    close(fd_server_dispatch);
    close(fd_server_interrupt);
    close(fd_server_io);
    //inicar mutex y semaforos
    sem_destroy(&sem_iniciar_planificacion);
    pthread_mutex_destroy(&mutex_lista_new);
    pthread_mutex_destroy(&mutex_lista_ready);
    pthread_mutex_destroy(&mutex_id_pcb_generador);
    pthread_mutex_destroy(&mutex_agotamiento_memoria);
    sem_destroy(&espacio_para_proceso);
    sem_destroy(&sem_proceso_bloqueado_por_io); 
    pthread_mutex_destroy(&mutex_procesos_esperando_ios);
    pthread_mutex_destroy(&mutex_ios_conectadas); 
    sem_destroy(&sem_hay_cpus_disponibles);   
    sem_destroy(&sem_hay_procesos_en_ready);  
    pthread_mutex_destroy(&mutex_lista_exec); 
    pthread_mutex_destroy(&mutex_cpus_conectadas); 
    pthread_mutex_destroy(&mutex_lista_exit); 
    pthread_mutex_destroy(&mutex_lista_blocked);
    sem_destroy(&sem_hay_procesos_esp_ser_ini_en_mem);
    sem_destroy(&sem_hay_procesos_en_exit); 
}
void destruir_proceso_esperado_io(void* proceso_wait){
    proceso_esperando_io* p_esperado = (proceso_esperando_io *) proceso_wait;
    free(p_esperado->nombre_io);
    free(p_esperado);
}
void destruir_io_conectada(void* io_conec){
    io_conectada* io = (io_conectada *)io_conec;
    free(io->nombre_io);
    close(io->fd);
    //list_destroy_and_destroy_elements(io->procesos_esperando_io,destruir_proceso_esperado_io);
    pthread_mutex_destroy(&io->mutex_io);
    free(io);
}
void destruir_pcb(void* proceso){
    pcb_t* pcb = (pcb_t *)proceso;
    free(pcb->archivo_en_pseudoCodigo);
    free(pcb);
}
void destruir_cpu_conectada(void* cpu){
    cpu_conectada* cpu_conec = (cpu_conectada*) cpu;
    close(cpu_conec->fd_dispatch);
    close(cpu_conec->fd_interrupt);
    free(cpu_conec);
}
