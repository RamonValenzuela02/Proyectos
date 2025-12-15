#include "include/io.h"
#include <signal.h>
#include <sys/socket.h>

int main(int argc, char* argv[]){
    signal(SIGINT, manejar_sigint);
    if (argc < 2) {
        printf("ERROR: Falta argumento (nombre de la io)");
        return 1;
    }
    nombre = argv[1];
    iniciarIo();

    //conexion con kernel
    fd_conexion_kernel = crear_conexion(IP_KERNEL,PUERTO_KERNEL);
    enviar_handshake();
    esperar_peticiones_kernel();
    
    return 0;
}


void iniciarIo(){
    io_logger = iniciar_logger("io.log","IO_LOG");
    io_config = iniciar_config("./io.config");
    cargarConfigs();
    log_info(io_logger, "IO INICIADA");
}

void cargarConfigs(){
    IP_KERNEL = config_get_string_value(io_config,"IP_KERNEL"); 
    PUERTO_KERNEL = config_get_string_value(io_config,"PUERTO_KERNEL");
    LOG_LEVEL= config_get_string_value(io_config,"LOG_LEVEL");
}

void enviar_handshake(){
    t_buffer* buffer= crear_buffer();
    cargar_string_al_buffer(buffer, nombre);
    t_paquete* paquete = crear_paquete(HANDSHAKE_IO_KERNEL, buffer);
    enviar_paquete(paquete,fd_conexion_kernel);

    eliminar_paquete(paquete);
}
void* esperar_peticiones_kernel(){
    while(!apagar_io){
        int code = recibir_operacion(fd_conexion_kernel);

        log_debug(io_logger,"codigo recibido: %d", code);

        switch(code) {
        case SOLICITUD_IO_DE_KERNEL:
        //recibimos peticion de usleep del kernel
            t_buffer* buffer = recibir_todo_un_buffer(fd_conexion_kernel);
            double tiempo_io = extraer_double_del_buffer(buffer);
            int pid = extraer_int_del_buffer(buffer);
            destruir_buffer(buffer);
            log_info(io_logger,"## PID: <%d> - Inicio de IO - Tiempo: <%f>",pid,tiempo_io);
        //hacemos el usleep 
            usleep(tiempo_io*1000);
            log_info(io_logger,"## PID: <%d> - Fin de IO",pid);
            t_buffer* buffer_rta= crear_buffer();
            cargar_string_al_buffer(buffer_rta, nombre);
        //respuesta al kernel de finalizado el usleep    
            t_paquete* paquete = crear_paquete(RTA_SOLICITUD_IO_DE_KERNEL, buffer_rta);
            enviar_paquete(paquete,fd_conexion_kernel);
            eliminar_paquete(paquete);
            break;

        case -1:
			log_error(io_logger, "el kernel se desconecto");
			// return EXIT_FAILURE;
		default:
			log_warning(io_logger,"Operacion desconocida");
			break;

        }

    }
}
//manejo de señales

void manejar_sigint(int senial){
    printf("\n[IO] Señal SIGINT recibida. Terminando...\n");
    apagar_io = 1;
    cerrar_conexiones();
    liberar_recursos();
    exit(0); // Salida limpia
}
void liberar_recursos(){
    printf("\nLiberando recursos...\n");

    free(IP_KERNEL);
    free(PUERTO_KERNEL);
    free(LOG_LEVEL);
    free(nombre);
}   


void cerrar_conexiones(){

    if (fd_conexion_kernel != -1) {
        close(fd_conexion_kernel);
        printf("Socket de conexión con Kernel cerrado.\n");
    }

    if (io_config != NULL) {
        config_destroy(io_config);
        printf("Configuración de IO destruida.\n");
    }

    if (io_logger != NULL) {
        log_destroy(io_logger);
        printf("Logger de IO destruido.\n");
    }
}