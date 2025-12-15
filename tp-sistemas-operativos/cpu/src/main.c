#include "include/cpu.h"
#include <signal.h>
#include <sys/socket.h>
int fd_conexion_dispatch;
int fd_conexion_interrupt;
int fd_conexion_memoria;



t_log* cpu_logger;
t_log* logger_debug;
t_config* cpu_config;

char* IP_MEMORIA;
char*PUERTO_MEMORIA;
char* PUERTO_DISPATCH; 
char* PUERTO_INTERRUPT; 
int ENTRADAS_TLB;
char* REEMPLAZO_TLB;
int ENTRADAS_CACHE;
char* REEMPLAZO_CACHE;
int RETARDO_CACHE;
char* LOG_LEVEL;
char* IP_KERNEL;

int main(int argc, char* argv[]) {
    signal(SIGINT, manejar_sigint);

    if (argc < 3) {
        printf("ERROR: Falta argumento (identificador de cpu)");
        return 1;
    }

    int identificador = atoi(argv[1]);
    
    // printf("id: %d ", identificador);

    cpu_config = iniciar_config(argv[2]);

    iniciarCpu(identificador);

    log_debug(cpu_logger,"cpu id: %d", identificador);
    log_debug(cpu_logger,"## Entradas de TLB : %d con reemplazo por %s, Entradas de CACHE %d con reemplazo por %s", ENTRADAS_TLB, REEMPLAZO_TLB, ENTRADAS_CACHE, REEMPLAZO_CACHE);

    //la cpu se va a conectar por dos puertos a kernel(cliente)
    fd_conexion_dispatch = crear_conexion(IP_KERNEL,PUERTO_DISPATCH);
    log_info(cpu_logger,"cpu %d se conecto a kernel dispath", identificador);

    fd_conexion_interrupt = crear_conexion(IP_KERNEL,PUERTO_INTERRUPT);
    log_info(cpu_logger,"cpu %d se conecto a kernel interrupt", identificador);

    enviar_hand_shake_cpu(HANDSHAKE_CPU_KERNEL,fd_conexion_dispatch,identificador); //para pasarle el identificador del cpu a kernel
    


    //hilo para esperar peteciones de kernel por interrupt
    pthread_t hilo_peticiones_kernel_interrupt;
    pthread_create(&hilo_peticiones_kernel_interrupt,NULL,atender_cpu_interrupt,NULL);

    //la cpu se va a conectar a la memoria 1 puerto (cliente)
    fd_conexion_memoria = crear_conexion(IP_MEMORIA,PUERTO_MEMORIA);
    enviar_hand_shake_cpu(HANDSHAKE_MEM_CPU,fd_conexion_memoria,identificador);//le pasa el hanshake a memoria y su id 
    log_info(cpu_logger,"cpu %d se conecto a memoria", identificador);

    obtener_config_de_memoria(fd_conexion_memoria);  // Esto deberías llamarlo solo una vez al inicio del programa->listo
    incializar_proceso_anterior();
    //poner un wait de semaforo global 
    atender_cpu_dispatch();
    // finalizar_cpu();


    pthread_join(hilo_peticiones_kernel_interrupt,NULL);
    return 0;
}


// void finalizar_cpu()
// {
//     log_info(cpu_logger, "Finalizando CPU");
//     log_destroy(cpu_logger);
//     cerrar_conexiones_cpu();
// }


// void cerrar_conexiones_cpu()
// {
//     close(fd_conexion_dispatch);
//     close(fd_conexion_interrupt);  
//     close(fd_conexion_memoria);
// }



void iniciarCpu(int identificador){
    char nombreLogger[30];
    char nombreModulo[30];
    sprintf(nombreLogger,"cpu_%d.log",identificador);
    sprintf(nombreModulo,"cpu_%d.log",identificador);
    cpu_logger =iniciar_logger(nombreLogger,nombreModulo);
    //cpu_config = iniciar_config("./cpu.config");
    cargarConfigs();
    // instruccion = malloc(sizeof(t_instruccion));
    // instruccion->args = malloc(sizeof(t_argumentos_genericos));
    
    //inicializo tlb y cache
    tlb = list_create();
    cache = list_create();


}
void cargarConfigs(){
    IP_MEMORIA= config_get_string_value(cpu_config,"IP_MEMORIA");
    PUERTO_MEMORIA = config_get_string_value(cpu_config,"PUERTO_MEMORIA");
    PUERTO_DISPATCH= config_get_string_value(cpu_config,"PUERTO_DISPATCH");
    PUERTO_INTERRUPT= config_get_string_value(cpu_config,"PUERTO_INTERRUPT");
    ENTRADAS_TLB= config_get_int_value(cpu_config,"ENTRADAS_TLB");
    REEMPLAZO_TLB = config_get_string_value(cpu_config,"REEMPLAZO_TLB");
    ENTRADAS_CACHE= config_get_int_value(cpu_config,"ENTRADAS_CACHE"); 
    REEMPLAZO_CACHE= config_get_string_value(cpu_config,"REEMPLAZO_CACHE");
    RETARDO_CACHE= config_get_int_value(cpu_config,"RETARDO_CACHE");
    LOG_LEVEL= config_get_string_value(cpu_config,"LOG_LEVEL");
    IP_KERNEL= config_get_string_value(cpu_config,"IP_KERNEL");
}


void manejar_sigint(int senial){
    printf("\n[CPU] Señal SIGINT recibida. Terminando...\n");
    apagar_cpu = 1; // Indica que la CPU debe apagarse
    cerrar_conexiones_cpu();
    // finalizar_cpu();
    exit(0); // Salida limpia
}

// void finalizar_cpu() {
//     log_info(cpu_logger, "Finalizando CPU");
//     log_destroy(cpu_logger);
//     cerrar_conexiones_cpu();
//     free(proceso);
//     free(instruccion->args);
//     free(instruccion);
//     // free(proceso_anterior);
//     free(IP_MEMORIA);
//     free(PUERTO_MEMORIA);
//     free(PUERTO_DISPATCH);
//     free(PUERTO_MEMORIA);  
//     free(REEMPLAZO_TLB);
//     list_destroy_and_destroy_elements(tlb, liberarEntrada_tlb);
//     list_destroy_and_destroy_elements(cache, liberaraEntrada_cache);
//     config_destroy(cpu_config);
// }

void cerrar_conexiones_cpu(){
    if(fd_conexion_dispatch != -1){
        close(fd_conexion_dispatch);
    }
    if(fd_conexion_interrupt!=-1){
        close(fd_conexion_interrupt);
    }
    if(fd_conexion_memoria != -1){
        close(fd_conexion_memoria);
    }
 
}