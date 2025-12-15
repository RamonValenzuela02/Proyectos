#include "include/kernel.h"

int main(int argc, char* argv[]) {
    signal(SIGINT, manejar_ctrl_c); 
    kernel_config = iniciar_config(argv[3]);
    iniciarKernel();
    /*
    if (argc < 3){
        log_error(kernel_logger,"ERROR: Falta argumento");
        return 1;
    }
    */
    //para cuando reciba la config
    if (argc < 4){
        log_error(kernel_logger,"ERROR: Falta argumento");
        return 1;
    }
    log_info(kernel_logger,"recibio las config el path: %s",argv[3]);
    

    //CONEXIONES CON MODULOS  
    
    //CONEXIONES CON CPU
    pthread_t hilo_conexiones_cpus;
    pthread_create(&hilo_conexiones_cpus,NULL,conexiones_cpus,NULL);

   
    //HILO A LA ESPERA DEL ENTER 
    pthread_t hilo_consola;
    pthread_create(&hilo_consola,NULL,consola_interactiva,NULL);

    //CONEXIONES CON IOS 
    pthread_t hilo_escucha_ios;
    pthread_create(&hilo_escucha_ios,NULL,esperar_ios,NULL);


    //PLANIFICACION LARGO PLAZO 
    SYSCALL_INIT_PROC(argv[1],atoi(argv[2]));
    // SYSCALL_INIT_PROC("PLANI_CORTO_PLAZO",0);
    pthread_join(hilo_consola,NULL);
    pthread_join(hilo_conexiones_cpus,NULL);

    pthread_t planificacion_largo_plazo;
    pthread_create(&planificacion_largo_plazo,NULL,planificador_largo_plazo,NULL);

    //PLANIFICACION CORTO PLAZO 
    pthread_t hilo_planificacion_corto_plazo;
    pthread_create(&hilo_planificacion_corto_plazo,NULL,planificacion_corto,NULL);

    //MANEJO DE IOS
    pthread_t hilo_manejo_de_io;
    pthread_create(&hilo_manejo_de_io,NULL,conexion_con_io,NULL);

    
    //pthread_join(hilo_consola,NULL);
    //pthread_join(hilo_conexiones_cpus,NULL);
    pthread_join(planificacion_largo_plazo,NULL);
    pthread_join(hilo_escucha_ios,NULL);
    pthread_join(hilo_manejo_de_io,NULL);
    pthread_join(hilo_planificacion_corto_plazo,NULL);

    //finalizar_kernel();
    close(fd_memoria);
     
    return 0;
}
