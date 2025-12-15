#include "include/memoria.h"
#include <math.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>


int main(int argc, char* argv[]){

    signal(SIGINT, manejar_sigint);

    mem_config = iniciar_config(argv[1]);

    iniciarMemoria();

    fd_server = iniciar_servidor(PUERTO_MEMORIA, mem_logger, "MEMORIA iniciado");
    
    //va ser una servidor multihilo esperando las conexiones de cpu y kernel
    //cad vez que le quieran mandar msj a memoria se van a tener que conectar antes
    while(!apagar_memoria){
        int* fd_conexion = malloc(sizeof(int));
        *fd_conexion = esperar_cliente(fd_server);
        log_info(mem_logger,"se conecto un cliente.");

        pthread_t hilo_cliente;
        pthread_create(&hilo_cliente,NULL,manejar_cliente,(void*) fd_conexion);
        pthread_detach(hilo_cliente);
    }

    return 0;
}


///home/utnso/tp-2025-1c-laEscaloneta/memoria/memoria.config

void iniciarMemoria(){
    tamanio_necesario = 0;
    hay_proceso_suspendido = false;
    sockets_de_cpus = list_create();
    lista_procesos = list_create();
    procesos_suspendidos = list_create();
    TAM_OCUPADO = 0;
    mem_logger = iniciar_logger("memoria.log","MEMORIA_LOG"); 
    //mem_config = iniciar_config("./memoria.config");
    cargar_config();
    iniciar_semaforos();
    memoria_usuario = malloc(TAM_MEMORIA);// se crea la memoria de usuario
    memset(memoria_usuario, 0, TAM_MEMORIA);// se setea con ceros toda la memoria

    CANT_FRAMES = 0; // cantidad de frames que tiene la memoria
    size_t nbytes = ((TAM_MEMORIA/TAM_PAGINA) + 7) / 8;
    char *buffer = malloc(nbytes);
    memset(buffer, 0, nbytes);
    bitarray_frames = bitarray_create_with_mode(buffer, nbytes, LSB_FIRST); // se crea el bitarray de frames
    numTablaCreada = 1; //va a empezar en 1
    numero_de_pagina = 0; // va a empezar en 0
    tabla_pag_nivel1 = malloc(sizeof(t_tabla));
    tabla_pag_nivel1->lista_paginas = list_create();
    tabla_pag_nivel1->nivel = 1;
    tabla_pag_nivel1->ocupadaPorCompleto = false;
    tabla_pag_nivel1->tabla_paginasPadre = NULL;     
    pthread_mutex_lock(&numeroDeTablaCreada);
   tabla_pag_nivel1->numTablaCreada = numTablaCreada++;// la tabla de nivel 1 es la primera tabla que se crea asi que lleva el num 1, uso un postincrementador
    pthread_mutex_unlock(&numeroDeTablaCreada);
    log_debug(mem_logger, "NIVEL %d",CANT_NIVELES);
    log_debug(mem_logger, "Se crea la primera tabla, numero %d ", tabla_pag_nivel1->numTablaCreada);
    crear_paginacion_jerarquica_nivel_n(); // se crean los n tnivelese de tablas de paginacion
    crearArchivoSwap();
}

void cargar_config(){
    PUERTO_MEMORIA = config_get_string_value(mem_config, "PUERTO_MEMORIA");
    TAM_MEMORIA = config_get_int_value(mem_config, "TAM_MEMORIA");
    TAM_PAGINA = config_get_int_value(mem_config, "TAM_PAGINA");
    ENTRADAS_POR_TABLA = config_get_int_value(mem_config, "ENTRADAS_POR_TABLA"); 
    CANT_NIVELES = config_get_int_value(mem_config, "CANTIDAD_NIVELES");
    RETARDO_MEMORIA = config_get_int_value(mem_config, "RETARDO_MEMORIA"); 
    RETARDO_SWAP = config_get_int_value(mem_config, "RETARDO_SWAP"); 
    PATH_SWAP = config_get_string_value(mem_config, "PATH_SWAPFILE");
    LOG_LEVEL = config_get_string_value(mem_config, "LOG_LEVEL"); 
    DUMP_PATH = config_get_string_value(mem_config, "DUMP_PATH");
    PATH_INSTRUCCIONES = config_get_string_value(mem_config, "PATH_INSTRUCCIONES");
}
void crearArchivoSwap(){
    archivoSwapFile = open("/home/utnso/swapfile.bin", O_RDWR | O_CREAT, 0666);
    if (archivoSwapFile == -1) {
        perror("open");
        log_error(mem_logger, "ocurrio un error al crear el archivo swap");
        
    }

    // double cant = pow((ENTRADAS_POR_TABLA * 3), CANT_NIVELES);; // TODO CAPAZ DEBA CAMBIARLO
 
    TAM_SWAP = TAM_MEMORIA *20;

    log_debug(mem_logger, "Tamaño Swap %d",TAM_SWAP);
    off_t tam = (off_t) TAM_SWAP; // lo agrando para obtener mas espacio 
    ftruncate(archivoSwapFile, tam); // tamaño de archivo a mapear

    if (ftruncate(archivoSwapFile, tam) == -1) {
        perror("ftruncate");
        log_error(mem_logger, "Error al truncar el archivo swap");
        close(archivoSwapFile);
        return;
    }

    // mapear el archivo a memoria
    swapFileMapeado = mmap(NULL, (size_t)TAM_SWAP, PROT_READ | PROT_WRITE, MAP_SHARED, archivoSwapFile, 0);
    if (swapFileMapeado == MAP_FAILED) {
        perror("mmap");
        log_error(mem_logger, "ocurrio un erro al mapear el archivo swap");
        exit(1);
    }

    entradasSwap = list_create(); // para saber que entradas estan ocupdas y por quien

    rellenarCamposDeEntradasASwap();
}

void finalizarSwapMapeado(void* mapa, size_t tam, int fd){
    munmap(mapa, tam);
    close(fd);
}


void* manejar_cliente(void* arg){
   
    int fd_conexion = *(int*)arg;
    free(arg);

    // while (1){
    op_code cod_op = recibir_operacion(fd_conexion);
  

    switch (cod_op) {
		case HANDSHAKE_MEM_KERNEL:  
            t_buffer* bufferR = recibir_todo_un_buffer(fd_conexion);
            int hash = extraer_int_del_buffer(bufferR);
            if(hash == 1){
                log_info(mem_logger,"## Kernel Conectado - FD del socket: <%d>", fd_conexion);
            }
            destruir_buffer(bufferR);

            int* fd_copiaK = malloc(sizeof(int));
            *fd_copiaK = fd_conexion;
            pthread_t hilo_cliente_kernel;
            pthread_create(&hilo_cliente_kernel,NULL,manejar_cliente_kernel,fd_copiaK);
            pthread_join(hilo_cliente_kernel, NULL);
            // pthread_detach(hilo_cliente_kernel);
			break;
		case HANDSHAKE_MEM_CPU:
            t_buffer* buffer = crear_buffer();
            buffer = recibir_todo_un_buffer(fd_conexion);
            int identificador = extraer_int_del_buffer(buffer);
            destruir_buffer(buffer);
            if(identificador){
                log_info(mem_logger,"se conecto CPU %d a Memoria", identificador);
            }
            int* fd_copiaC = malloc(sizeof(int));
            *fd_copiaC = fd_conexion;
            list_add(sockets_de_cpus,fd_copiaC); //guardo cada socket de cpu que se conectó
            pthread_t hilo_cliente_cpu;
            pthread_create(&hilo_cliente_cpu,NULL,manejar_cliente_cpu,fd_copiaC);
            pthread_join(hilo_cliente_cpu, NULL);
            // pthread_detach(hilo_cliente_cpu);
         
			break;
		default:
			log_warning(mem_logger,"Operacion desconocida.");
            log_debug(mem_logger, "la operacion desconocida es esta %d ", cod_op);
			break;
		}

    // }

        return NULL;
}

void* manejar_cliente_kernel(void* arg){

    int fd_conexion_kernel = *(int*) arg;
    free(arg);
   
    // while (1){

    op_code cod_op = recibir_operacion(fd_conexion_kernel);

    log_info(mem_logger, "cod_op: %d ", cod_op);

    switch (cod_op) {
		case CREATE_PROCESS:
            usleep(RETARDO_MEMORIA*1000); // Simula el retardo de memoria // 1 microsegundo = 1000 milisegundos
            //recibe el path del proceso y tamaño del proceso
            t_proceso* nuevo_proceso = malloc(sizeof(t_proceso));
            nuevo_proceso->lista_pag_del_proceso = list_create();
            nuevo_proceso->colaDeCaminoAPag = queue_create();
            nuevo_proceso->colaDECaminoNumTP = queue_create();
            
    
            t_buffer* buffer1 = recibir_todo_un_buffer(fd_conexion_kernel);

            pthread_mutex_lock(&mutex_pid);
            nuevo_proceso->pid_proceso = extraer_int_del_buffer(buffer1);
            pthread_mutex_unlock(&mutex_pid);

            char* path_proceso = extraer_string_del_buffer(buffer1);
            nuevo_proceso->tamanio_proceso = extraer_int_del_buffer(buffer1);

            log_debug(mem_logger,"se recibio el path del proceso %s y su tamanio %d",path_proceso, nuevo_proceso->tamanio_proceso);
            char rutaAlScript[100];
            strcpy(rutaAlScript,PATH_INSTRUCCIONES);
            strcat(rutaAlScript, path_proceso);
            
        
            pthread_mutex_lock(&mutex_add_lista);
            nuevo_proceso->lista_instrucciones_proceso = crear_lista_de_instrucciones(rutaAlScript);
            list_add(lista_procesos,nuevo_proceso);
            pthread_mutex_unlock(&mutex_add_lista);

           
            asignarTablasDePagAlProceso(nuevo_proceso->tamanio_proceso, nuevo_proceso->lista_pag_del_proceso, nuevo_proceso->pid_proceso);
          

            pthread_mutex_lock(&mutex_contador);
            cant_procesosMem++;
            pthread_mutex_unlock(&mutex_contador);

            pthread_mutex_lock(&mutex_inicializador_metricas);
            nuevo_proceso->cantAccesosTP = 0;
            nuevo_proceso->cantInstruccionesSolicitadas = 0;
            nuevo_proceso->cantBajadasSwap = 0; 
            nuevo_proceso->cantSubidasMP = 0;
            nuevo_proceso->cantLecturasMem = 0;
            nuevo_proceso->cantEscriturasMem = 0;
            pthread_mutex_unlock(&mutex_inicializador_metricas);

            log_info(mem_logger,"Creación de Proceso: ## PID: <%d> - Proceso Creado - Tamaño: <%d>", nuevo_proceso->pid_proceso, nuevo_proceso->tamanio_proceso);//LOG OBLIGATORIO
            destruir_buffer(buffer1);
            free(path_proceso);

            t_buffer* bufferCreate = crear_buffer();
            int rtaCreate = 0;
            cargar_entero_al_buffer(bufferCreate, rtaCreate);
            t_paquete* paqueteCreate = crear_paquete(RTA_CREATE_PROCESS,bufferCreate);
            enviar_paquete(paqueteCreate,fd_conexion_kernel);
            close(fd_conexion_kernel);
            pthread_exit(NULL);
			break;
		case ESPACIO_DISPONIBLE_PARA_PROCESO: 
            usleep(RETARDO_MEMORIA*1000);   
            t_buffer* buffer2 = recibir_todo_un_buffer(fd_conexion_kernel);
            int id_proceso = extraer_int_del_buffer(buffer2);
            int tam_proceso = extraer_int_del_buffer(buffer2);//no necesito el pseudocodigo porque ya lo tengo en la memoria
            destruir_buffer(buffer2);

            int espacioDisponibleConEseProceso = chequear_y_actualizar_suspensiones(tam_proceso, INGRESO);   

            log_info(mem_logger, "Tam proceso %d y quedaría ESPACIO DISPONIBLE %d ",tam_proceso,espacioDisponibleConEseProceso);

            int rta =  -1;
            if(espacioDisponibleConEseProceso < 0){
                rta = 1;
                log_debug(mem_logger,"no hay espacio suficiente para el proceso %d porque sino la memoria queda en %d ", id_proceso, espacioDisponibleConEseProceso);
            } else{
                rta = 0;
                log_debug(mem_logger,"si hay espacio suficiente para el proceso en memoria ");
            }
            
            pthread_mutex_lock(&mutex_conexion_con_kernel);
            t_buffer* buffer3 = crear_buffer();
            cargar_entero_al_buffer(buffer3, rta);
            t_paquete* paquete_espacio_disponible = crear_paquete(RTA_ESPACIO_DISPONIBLE_PARA_PROCESO, buffer3);
            enviar_paquete(paquete_espacio_disponible, fd_conexion_kernel);
            log_info(mem_logger,"se envio el tamanio de memoria disponible para el PID %d ", id_proceso);
            pthread_mutex_unlock(&mutex_conexion_con_kernel);

            eliminar_paquete(paquete_espacio_disponible);
            close(fd_conexion_kernel);
            pthread_exit(NULL);
			break;
        case ELIMINAR_PROCESO_DE_MEMORIA:
            usleep(RETARDO_MEMORIA*1000); 
            t_buffer* buffer4 = recibir_todo_un_buffer(fd_conexion_kernel);
            int pid_proceso = extraer_int_del_buffer(buffer4);
            destruir_buffer(buffer4);

            pthread_mutex_lock(&mutex_add_lista);
            t_proceso* un_proceso = buscar_proceso_y_eliminar(pid_proceso);  
            pthread_mutex_unlock(&mutex_add_lista);
            
          
            int tam_final = chequear_y_actualizar_suspensiones(un_proceso->tamanio_proceso, SE_MUERE);
            
            log_debug(mem_logger,"Eliminando proceso con PID: <%d> - Tamaño: <%d> ESPACIO_OCUPADO_FINAL: %d", un_proceso->pid_proceso, un_proceso->tamanio_proceso,tam_final);

           
            eliminarEstructurasDelProcesoEnMemoria(un_proceso);
           

            pthread_mutex_lock(&mutex_contador);
            cant_procesosMem--;
            pthread_mutex_unlock(&mutex_contador);

            t_buffer* buffer5 = crear_buffer();  
            int rtaEliminacion = 0;
            cargar_entero_al_buffer(buffer5, rtaEliminacion);
            pthread_mutex_lock(&mutex_conexion_con_kernel);
            t_paquete* paquete_eliminar_proc_mem = crear_paquete(RTA_ELIMINAR_PROCESO_DE_MEMORIA, buffer5);
            enviar_paquete(paquete_eliminar_proc_mem, fd_conexion_kernel);
            log_debug(mem_logger,"se elimino el proceso %d de memoria", pid_proceso);
            pthread_mutex_unlock(&mutex_conexion_con_kernel);
            eliminar_paquete(paquete_eliminar_proc_mem);
            close(fd_conexion_kernel);
            pthread_exit(NULL);
            break;
        case DUMP_MEMORY:   
            usleep(RETARDO_MEMORIA*1000); 
            t_buffer* bufferDump = recibir_todo_un_buffer(fd_conexion_kernel);
            int pidDump = extraer_int_del_buffer(bufferDump);
            destruir_buffer(bufferDump);

            log_info(mem_logger,"Memory Dump: ## PID: <%d> - Memory Dump solicitado",pidDump);//LOG OBLIGATORIO
        
         
            t_proceso* procesoDump = buscar_proceso(pidDump);

            
            generarDumpMemory(procesoDump);
            

            log_debug(mem_logger, "Se realizó el DumpMemory para el PID: %d ", pidDump);

            t_buffer* bufferDumpRta = crear_buffer();
            int confirmacionDump = 1; // 1 indica que el dump se realizó correctamente
            cargar_entero_al_buffer(bufferDumpRta, confirmacionDump); 
            pthread_mutex_lock(&mutex_conexion_con_kernel);
            t_paquete* paqueteDumpRta = crear_paquete(RTA_DUMP_MEMORY, bufferDumpRta);
            enviar_paquete(paqueteDumpRta, fd_conexion_kernel);
            log_debug(mem_logger, "Se envió la respuesta de DumpMemory");
            pthread_mutex_unlock(&mutex_conexion_con_kernel);
            close(fd_conexion_kernel);
            pthread_exit(NULL);
            break;
		case SUSPENDER_PROCESO:
            usleep(RETARDO_SWAP*1000); 
           
            t_buffer* bufferSuspender = recibir_todo_un_buffer(fd_conexion_kernel);
            int pidDeSuspendido = extraer_int_del_buffer(bufferSuspender);
            log_debug(mem_logger, "Se recibe solicitud de suspender proceso para el PID %d",pidDeSuspendido);

            escribirPaginasEnSwap(pidDeSuspendido); // aca adentro se incrementa la cant de bajadas a SWAP
          
            close(fd_conexion_kernel);
            pthread_exit(NULL);
			break;
        case DESSUSPENDER_PROCESO:
            usleep(RETARDO_MEMORIA*1000); 
            t_buffer* bufferDesSuspender = recibir_todo_un_buffer(fd_conexion_kernel);
            int pidDeDessuspendido = extraer_int_del_buffer(bufferDesSuspender);

            log_debug(mem_logger, "Se recibe solicitud de dessuspender proceso para el PID %d",pidDeDessuspendido);
            escribirPaginasDeSwapAMP(pidDeDessuspendido); // aca adentro se incrementa la cant de bajadas a MP
            t_proceso* procesoDes = buscar_proceso(pidDeDessuspendido);
            log_info(mem_logger, "El proceso  %d después de volver de SWAP TIENE %d PAGINAS", procesoDes->pid_proceso, list_size(procesoDes->lista_pag_del_proceso));
         
            // pthread_mutex_lock(&mutex_send_ok_kernel);
            pthread_mutex_lock(&mutex_conexion_con_kernel);
            log_debug(mem_logger, "se va a enviar OK-DESUSPENSIÓN a Kernel");
            op_code cod = OK;
            send(fd_conexion_kernel, &cod, sizeof(op_code), 0); // responde OK a Kernel
            close(fd_conexion_kernel);
            log_debug(mem_logger, "se envió OK-DESUSPENSIÓN a Kernel!!");
            pthread_mutex_unlock(&mutex_conexion_con_kernel);
            // pthread_mutex_unlock(&mutex_send_ok_kernel);
           
            pthread_exit(NULL);
			break;
        case SOYKERNEL:
            log_info(mem_logger,"entra por el soykernel");
            log_info(mem_logger,"## Kernel Conectado - FD del socket: <%d>", fd_conexion_kernel);//LOG OBLIGATORIO
			break;
        case -1:
            log_info(mem_logger, "Se desconecta KERNEL porque termino su petición");
            close(fd_conexion_kernel);
            pthread_exit(NULL);
            break;
		default:
			log_warning(mem_logger,"Se desconecto.");
			break;
		}

    // }

        return NULL;
}


void* manejar_cliente_cpu(void* arg){

    int fd_conexion_cpu = *(int*) arg;
    free(arg);

    while(1){
       
        op_code cod_op = recibir_operacion(fd_conexion_cpu);    

        log_debug(mem_logger, "Se recibio un codigo de operacion: <%d>", cod_op);
        
        switch (cod_op) {
            case SOLICITAR_CONFIG_MEMORIA:
                usleep(RETARDO_MEMORIA*1000);
                t_buffer* bufferConfig = recibir_todo_un_buffer(fd_conexion_cpu);
                int numeroRandomConfig = extraer_int_del_buffer(bufferConfig);
                if(numeroRandomConfig){
                    log_debug(mem_logger, "Cpu solicita saber cantidad de niveles, entradas por tabla y tamanio de pagina");
                }
                t_buffer* bufferConfigMem = crear_buffer();
                cargar_entero_al_buffer(bufferConfigMem, CANT_NIVELES);
                cargar_entero_al_buffer(bufferConfigMem, ENTRADAS_POR_TABLA);
                cargar_entero_al_buffer(bufferConfigMem, TAM_PAGINA);
                t_paquete* paqueteConfigMem = crear_paquete(SOLICITAR_CONFIG_MEMORIA_RTA, bufferConfigMem);
                enviar_paquete(paqueteConfigMem, fd_conexion_cpu);
                log_debug(mem_logger,"se envio la configuracion de memoria a CPU");
                eliminar_paquete(paqueteConfigMem);
                break;
            case GET_INSTRUCCION:   
                usleep(RETARDO_MEMORIA*1000);          
                t_buffer* buffer = recibir_todo_un_buffer(fd_conexion_cpu);
                int pid_proceso = extraer_int_del_buffer(buffer);
                int pc_instruccion = extraer_int_del_buffer(buffer);
                destruir_buffer(buffer);

                log_debug(mem_logger, "CPU  solicita la instruccion de PID: %d PC: %d", pid_proceso, pc_instruccion);

                // pthread_mutex_lock(&mutex_get_listaProcesos);
                log_debug(mem_logger,"tamaño de lista de procesos : %d",list_size(lista_procesos));
                // t_proceso* un_proceso = list_get(lista_procesos,pid_proceso);
                t_proceso* un_proceso = buscar_proceso(pid_proceso);
                // pthread_mutex_unlock(&mutex_get_listaProcesos);

                if(un_proceso==NULL)
                    log_debug(mem_logger,"No hay proceso para solicitar instrucciones");

                pthread_mutex_lock(&mutex_get_listaInstrucciones);
                t_instruccion* instruccion = list_get(un_proceso->lista_instrucciones_proceso, pc_instruccion);
                pthread_mutex_unlock(&mutex_get_listaInstrucciones);
                
                pthread_mutex_lock(&incrementarCantSolicitudesInstr);
                un_proceso->cantInstruccionesSolicitadas++;
                pthread_mutex_unlock(&incrementarCantSolicitudesInstr);

                loguear_instruccion(pid_proceso,instruccion);
                
                t_buffer* buffer1 = crear_buffer();
                cargar_instruccion_al_buffer(buffer1,instruccion);
                t_paquete* paquete = crear_paquete(OBTENER_INSTRUCCION_RTA, buffer1);
                enviar_paquete(paquete, fd_conexion_cpu);
                log_debug(mem_logger, "se envio la instruccion con  PID : %d PC: %d a CPU", pid_proceso, pc_instruccion);
                destruir_buffer(buffer1);
                break;
            case SOLICITAR_VALOR:
                usleep(RETARDO_MEMORIA*1000);
                t_buffer* pedidoDeLectura = recibir_todo_un_buffer(fd_conexion_cpu);
                int pidProcesoQueLee = extraer_int_del_buffer(pedidoDeLectura); 
                int numeroPagina = extraer_int_del_buffer(pedidoDeLectura);
                int direccionFisicaEnMem= extraer_int_del_buffer(pedidoDeLectura);
                int tamanio = extraer_int_del_buffer(pedidoDeLectura);
                destruir_buffer(pedidoDeLectura);

                log_debug(mem_logger,"se recibio la pág local %d la direccion fisica %d y el tamanio %d",numeroPagina  ,direccionFisicaEnMem, tamanio);

                pthread_mutex_lock(&solicitar_valor);

                t_proceso* procesoQueLeyo = buscar_proceso(pidProcesoQueLee);

                char* valor = pedir_valor_a_memoria(direccionFisicaEnMem, tamanio, procesoQueLeyo);
                if (valor == NULL) {
                    log_error(mem_logger, "Error al obtener el valor  que se quería leer de memoria, me da NULL porque la DF es negativa o mayor al tam de memoria");
                    break;
                }else{
                    log_debug(mem_logger, "Valor obtenido de memoria: %s", valor);
                }
                pthread_mutex_lock(&incrementarCantLecturas);
                procesoQueLeyo->cantLecturasMem++;
                pthread_mutex_unlock(&incrementarCantLecturas);
                t_buffer* datoLeido= crear_buffer();
                cargar_string_al_buffer(datoLeido, valor);
                t_paquete* paqueteValor = crear_paquete(SOLICITAR_VALOR_RTA, datoLeido);
                enviar_paquete(paqueteValor, fd_conexion_cpu);  

                pthread_mutex_unlock(&solicitar_valor);
                //LOG OBLIGATORIO
                log_info(mem_logger, " LECTURA en espacio de usuario: ## PID: <%d> - <Lectura> - Dir. Física: <%d> - Tamaño: <%d>",pidProcesoQueLee,direccionFisicaEnMem,tamanio);
                break;
            case ESCRIBIR_VALOR:
                usleep(RETARDO_MEMORIA*1000);
                t_buffer* pedidoDeEscritura = recibir_todo_un_buffer(fd_conexion_cpu);
                int pidProcesoQueVaEscribir = extraer_int_del_buffer(pedidoDeEscritura); 
                int direccionFisicaWrite= extraer_int_del_buffer(pedidoDeEscritura);
                char* datoAEScribir = extraer_string_del_buffer(pedidoDeEscritura);
                destruir_buffer(pedidoDeEscritura);

                pthread_mutex_lock(&escribir_valor);
                int tam = strlen(datoAEScribir);
                log_debug(mem_logger, "Se recibio el dato %s para escribir en la dir fisica %d ", datoAEScribir,direccionFisicaWrite);

                escribir_valor_en_memoria(datoAEScribir,direccionFisicaWrite);

                // t_proceso* procesoQueEscribio = list_get(lista_procesos, pidProcesoQueVaEscribir);
                t_proceso* procesoQueEscribio = buscar_proceso(pidProcesoQueVaEscribir);      
                marcarPaginasEscritas(procesoQueEscribio,datoAEScribir, direccionFisicaWrite);
                
                pthread_mutex_lock(&incrementarCantEscriturasMem);
                procesoQueEscribio->cantEscriturasMem ++;
                pthread_mutex_unlock(&incrementarCantEscriturasMem);

                t_buffer* bufferEscritura = crear_buffer();
                int verificado = 1;
                cargar_entero_al_buffer(bufferEscritura,verificado); // el 1 simula el OK
                t_paquete* paqueteEscritura = crear_paquete(ESCRIBIR_VALOR_RTA, bufferEscritura);
                enviar_paquete(paqueteEscritura,fd_conexion_cpu);
                eliminar_paquete(paqueteEscritura);

                pthread_mutex_unlock(&escribir_valor);
                 //LOG OBLIGATORIO
                log_info(mem_logger, "ESCRITURA en espacio de usuario: ## PID: <%d> - <Escritura> - Dir. Física: <%d> - Tamaño: <%d>",pidProcesoQueVaEscribir,direccionFisicaWrite,tam);
                break;
            case PEDIR_TABLA_NIVEL: 
                usleep(RETARDO_MEMORIA*1000);
                t_buffer* bufferDePedidioDeTabla = recibir_todo_un_buffer(fd_conexion_cpu);
                int pid_procesoTB = extraer_int_del_buffer(bufferDePedidioDeTabla);
                int entrada = extraer_int_del_buffer(bufferDePedidioDeTabla);
                int nivel = extraer_int_del_buffer(bufferDePedidioDeTabla);
                destruir_buffer(bufferDePedidioDeTabla);

                pthread_mutex_lock(&pedir_tabla);
                int numTablaSignivel = 0;

                if(ENTRADAS_POR_TABLA != 8){//si no es la de estabilidad se va ir por aca
                    numTablaSignivel = buscarTablaDelSigNivel(nivel,entrada, pid_procesoTB);
                }else{
                    //voy a recibir las entradas y el nivel y el pid pero no voy a hacer nada con eso
                    numTablaSignivel = buscarTablaSigNivelparaEStabilidad(pid_procesoTB,entrada, nivel);
                }

                  
                t_proceso* proceso = buscar_proceso(pid_procesoTB);
                
                pthread_mutex_lock(&cant_AccesosTP);
                proceso->cantAccesosTP++; // 
                pthread_mutex_unlock(&cant_AccesosTP);

                t_buffer* nivelDeTablaQueSigue = crear_buffer();
                cargar_entero_al_buffer(nivelDeTablaQueSigue, pid_procesoTB);
                cargar_entero_al_buffer(nivelDeTablaQueSigue, numTablaSignivel);       
                t_paquete* paqueteTablaNivel = crear_paquete(PEDIR_TABLA_NIVEL_RTA, nivelDeTablaQueSigue);
                enviar_paquete(paqueteTablaNivel, fd_conexion_cpu);    
                log_debug(mem_logger,"se envio la tabla %d  para el proceso %d", numTablaSignivel, pid_procesoTB);
                eliminar_paquete(paqueteTablaNivel);

                pthread_mutex_unlock(&pedir_tabla);
                break;
            case PEDIR_MARCO:
                usleep(RETARDO_MEMORIA*1000*CANT_NIVELES); // simula el retardo por cada acceso a un nivel de tabla de paginas
                t_buffer* bufferMarco = recibir_todo_un_buffer(fd_conexion_cpu);
                int pidProcesoQuePideMarco = extraer_int_del_buffer(bufferMarco);
                int tablaDeUltimoNivel = extraer_int_del_buffer(bufferMarco);
                int entradaPagina = extraer_int_del_buffer(bufferMarco);
                destruir_buffer(bufferMarco);

                log_debug(mem_logger,"se recibio el pid %d, la tabla %d de ultimo nivel y entrada de pagina %d", pidProcesoQuePideMarco, tablaDeUltimoNivel, entradaPagina);

                pthread_mutex_lock(&pedir_marco);
                t_proceso* procesoMarco = buscar_proceso(pidProcesoQuePideMarco);

                int marco = 0;
                if(ENTRADAS_POR_TABLA != 8){
                    marco = obtenerMarco(CANT_NIVELES,entradaPagina,pidProcesoQuePideMarco);
                }else{
                    pthread_mutex_lock(&mutex_buscando_num_pag_global);
                    marco = buscarMarcoParaEstabilidad(pidProcesoQuePideMarco, entradaPagina, procesoMarco);
                    pthread_mutex_unlock(&mutex_buscando_num_pag_global);
                }

                if(marco == -1){
                    log_error(mem_logger, "El marco es -1, porque no se encontró para el PID %d", pidProcesoQuePideMarco);
                }
                   
                pthread_mutex_lock(&cant_AccesosTP);
                procesoMarco->cantAccesosTP++; // por acceder a la Tabla de nivel 1
                pthread_mutex_unlock(&cant_AccesosTP);

                t_buffer* bufferConMarco = crear_buffer();
                cargar_entero_al_buffer(bufferConMarco, marco);   
                t_paquete* paqueteMarco = crear_paquete(PEDIR_MARCO_RTA, bufferConMarco);
                enviar_paquete(paqueteMarco, fd_conexion_cpu);
             
                log_debug(mem_logger,"se envio el marco  N° %d del proceso %d", marco, pidProcesoQuePideMarco);
                eliminar_paquete(paqueteMarco);

                pthread_mutex_unlock(&pedir_marco);
                break;

            case LEER_PAGINA_COMPLETA:
                usleep(RETARDO_MEMORIA*1000);
                t_buffer* bufferLeerPag = recibir_todo_un_buffer(fd_conexion_cpu);
                int pidProcesoPaginaALeer = extraer_int_del_buffer(bufferLeerPag);
                int direccionFisicaPag = extraer_int_del_buffer(bufferLeerPag);
                destruir_buffer(bufferLeerPag);
                
                log_debug(mem_logger, "Se recibio el PID %d direccion fisica %d ", pidProcesoPaginaALeer,direccionFisicaPag);

                pthread_mutex_lock(&mutex_leer_pagina);

                t_proceso* procesoQueLeyoPag = buscar_proceso(pidProcesoPaginaALeer);
                //me va a decir el numero de pagina global que corresponde

                log_debug(mem_logger, "DIRECCION FISICA QUE RECIBE DE CPU %d para el PID %d",direccionFisicaPag, pidProcesoPaginaALeer);
                int numPagGlobal = buscarNroPaginaAtravesDeDirFisicaGlobal(direccionFisicaPag, pidProcesoPaginaALeer);

                if(numPagGlobal == -1){
                    log_debug(mem_logger, "EL numero de página global es -1");
                    pthread_mutex_unlock(&mutex_leer_pagina);
                    break;
                }
                void* valorPagina = (void*)pedirPaginaCompetaAMemoria(direccionFisicaPag, numPagGlobal, TAM_PAGINA, procesoQueLeyoPag);

                if(valorPagina == NULL){
                    valorPagina = "\0";
                    pthread_mutex_unlock(&mutex_leer_pagina);
                    break;
                }
        
                t_buffer* bufferContenidoPag = crear_buffer();
                // cargar_string_al_buffer(bufferContenidoPag, valorPagina);   
                cargar_choclo_al_buffer(bufferContenidoPag,valorPagina, TAM_PAGINA);
                t_paquete* paqueteContenido = crear_paquete(LEER_PAGINA_COMPLETA_RTA, bufferContenidoPag);
                enviar_paquete(paqueteContenido, fd_conexion_cpu);
                eliminar_paquete(paqueteContenido);
              
                
                pthread_mutex_lock(&incrementarCantLecturas);
                procesoQueLeyoPag->cantLecturasMem ++;
                pthread_mutex_unlock(&incrementarCantLecturas);
                log_debug(mem_logger,"Se envio el contenido de la página N° %d para el PID %d", numPagGlobal, pidProcesoPaginaALeer);
                //LOG OBLIGATORIO
                log_info(mem_logger, " LEER en espacio de usuario: ## PID: <%d> - <Lectura> - Dir. Física: <%d> - Tamaño: <%d>",pidProcesoPaginaALeer,direccionFisicaPag,TAM_PAGINA);
                
                pthread_mutex_unlock(&mutex_leer_pagina);
              
                break;
            case ACTUALIZAR_PAGINA_COMPLETA:
                usleep(RETARDO_MEMORIA*1000);

                t_buffer* bufferEscribirPag = recibir_todo_un_buffer(fd_conexion_cpu);
                int pidProcesoPaginaAEscribir = extraer_int_del_buffer(bufferEscribirPag);
                int dirFisicaPag = extraer_int_del_buffer(bufferEscribirPag);
                char* datoAEscribirEnPag =(char*)extraer_choclo_del_buffer(bufferEscribirPag); //siempre tiene que ser la direccion fisica de la página global

                t_proceso* procesoQueEscribioPag = buscar_proceso(pidProcesoPaginaAEscribir);

                pthread_mutex_lock(&mutex_actualizar_pagina);
                int numPagGlobalEscrita = buscarNroPaginaAtravesDeDirFisicaGlobal(dirFisicaPag, pidProcesoPaginaAEscribir);
                if(numPagGlobalEscrita == -1){
                    log_debug(mem_logger, "EL numero de página global es -1 porque la dreccion fisica %d no esta entre las pág del PID %d ", dirFisicaPag, procesoQueEscribioPag->pid_proceso);
                    for(int i; i<list_size(procesoQueEscribioPag->lista_pag_del_proceso);i++){
                        t_pagina* pagina = list_get(procesoQueEscribioPag->lista_pag_del_proceso,i);
                        log_debug(mem_logger, "PID %d - Pagina Global %d Pag local %d - Dir. Fisica: Marco base %d marco limite %d", procesoQueEscribioPag->pid_proceso, pagina->num_pagina,pagina->num_pag_local_pid, pagina->frame.base, pagina->frame.limite);               
                    }
                    pthread_mutex_unlock(&mutex_actualizar_pagina);
                    log_debug(mem_logger, "No se actializo (envio send a CPU)la pag con DF %d del PId %d",dirFisicaPag, pidProcesoPaginaAEscribir);
                    break;
                }
                escribirPaginaCompleta(datoAEscribirEnPag, dirFisicaPag, numPagGlobalEscrita,procesoQueEscribioPag);

                pthread_mutex_lock(&incrementarCantEscriturasMem);
                procesoQueEscribioPag->cantEscriturasMem ++;
                pthread_mutex_unlock(&incrementarCantEscriturasMem);

                marcarPaginasEscritas(procesoQueEscribioPag,datoAEscribirEnPag, dirFisicaPag); // Aca dentro se marca la pagina como escrita


                log_debug(mem_logger, "Se actualizo la pagina %d ", numPagGlobalEscrita);
                        
                pthread_mutex_lock(&send_ok_cpu);
                op_code cod = OK;
                send(fd_conexion_cpu,&cod,sizeof(int), 0); // envia el OK
                pthread_mutex_unlock(&send_ok_cpu);
                //LOG OBLIGATORIO
                log_info(mem_logger, "ESCRITURA en espacio de usuario: ## PID: <%d> - <Escritura> - Dir. Física: <%d> - Tamaño: <%d>",pidProcesoPaginaAEscribir,dirFisicaPag,TAM_PAGINA);
                pthread_mutex_unlock(&mutex_actualizar_pagina);
                break;
            
             case -1:
                log_info(mem_logger,"se desconecta CPU");
            
                break;
            default:
                log_warning(mem_logger,"Operacion desconocida.");
                break;
        }
    }
        return NULL;
}

t_proceso* buscar_proceso_y_eliminar(int pid){

    t_proceso* proceso = buscar_proceso(pid);

    if(proceso==NULL)
        log_error(mem_logger,"No se encontro el proceso");
    pthread_mutex_lock(&mutex_get_listaProcesos);
    log_debug(mem_logger,"tamaño de lista de procesos : %d",list_size(lista_procesos));
    list_remove_element(lista_procesos,proceso);
    pthread_mutex_unlock(&mutex_get_listaProcesos);

    return proceso;
}

t_proceso* buscar_proceso(int pid){

    bool mismo_proceso(void* ptr){
        t_proceso* proceso = (t_proceso*)ptr;
        return proceso->pid_proceso == pid;
    }

    pthread_mutex_lock(&mutex_get_listaProcesos);
    log_debug(mem_logger,"tamaño de lista de procesos : %d",list_size(lista_procesos));
    t_proceso* proceso = (t_proceso*)list_find(lista_procesos,mismo_proceso);
    pthread_mutex_unlock(&mutex_get_listaProcesos);

    if(proceso==NULL){
        log_error(mem_logger,"No se encontro el proceso");
    }

    return proceso;
        
    
}

 
t_list* crear_lista_de_instrucciones(char* path_proceso){
    
    FILE* archivo = fopen(path_proceso, "r");

    if (archivo == NULL) {
        log_error(mem_logger, "Error al abrir el archivo: %s", path_proceso);
        exit(EXIT_FAILURE);
        return NULL;
    }
    t_list* lista_instrucciones = list_create();
   
    int id_inicial_ins = 0;
   
    char linea[100];

    while(fgets(linea, sizeof(linea), archivo) != NULL){
        // Eliminar el salto de línea al final de la línea
        linea[strcspn(linea, "\n")] = '\0';
        t_instruccion* instruccion = malloc(sizeof(t_instruccion));
        
        // log_info(mem_logger,"se leyo la siguiente instruccion: %s", linea);
        
      
        char* token = strtok(linea, " ");
        
        verificarOperacion(token, instruccion);
        
    
        list_add(lista_instrucciones,instruccion);
        instruccion->id = id_inicial_ins;

        // log_info(mem_logger,"se guardo la instruccion n° %d", instruccion->id );

        id_inicial_ins++;


    }

    fclose(archivo);
   
    return lista_instrucciones;
    
}

void verificarOperacion(char* token, t_instruccion* instruccion){
    if(strcmp(token,"NOOP") == 0){
        instruccion->operacion = malloc(strlen(token)+1);
        strcpy(instruccion->operacion,token);
        instruccion->parametros = malloc(sizeof(t_parametros));
        instruccion->parametros->primerParamNum = NULL;
        instruccion->parametros->primerParamString = NULL;
        instruccion->parametros->segundoParamNum = NULL;
        instruccion->parametros->segundoParamString = NULL;
        // log_info(mem_logger,"se guardo la instruccion %s %p %p %p %p", instruccion->operacion, instruccion->parametros->primerParamNum, instruccion->parametros->primerParamString, instruccion->parametros->segundoParamNum, instruccion->parametros->segundoParamString);

    }
    else if(strcmp(token,"READ") == 0){
        instruccion->operacion = malloc(strlen(token)+1);
        strcpy(instruccion->operacion,token);
        instruccion->parametros = malloc(sizeof(t_parametros));
        instruccion->parametros->primerParamNum = malloc(sizeof(int));
        *(instruccion->parametros->primerParamNum) = atoi(strtok(NULL, " "));
        instruccion->parametros->segundoParamNum = malloc(sizeof(int));
        *(instruccion->parametros->segundoParamNum) = atoi(strtok(NULL, " "));
        instruccion->parametros->primerParamString = NULL;
        instruccion->parametros->segundoParamString = NULL;
        // log_info(mem_logger, "se guardo la instruccion %s %d %d %p %p",instruccion->operacion,*(instruccion->parametros->primerParamNum),*(instruccion->parametros->segundoParamNum), instruccion->parametros->primerParamString,instruccion->parametros->segundoParamString);

    }
    else if(strcmp(token,"WRITE") == 0){
        instruccion->operacion = malloc(strlen(token)+1);
        strcpy(instruccion->operacion,token);
        instruccion->parametros = malloc(sizeof(t_parametros));
        instruccion->parametros->primerParamNum = malloc(sizeof(int));
        *(instruccion->parametros->primerParamNum) = atoi(strtok(NULL, " "));
        instruccion->parametros->segundoParamNum = NULL;
        instruccion->parametros->primerParamString = NULL;
        instruccion->parametros->segundoParamString = strdup(strtok(NULL, " "));
        // log_info(mem_logger,"se guardo la instruccion %s  %d  %s %p  %p", instruccion->operacion,*(instruccion->parametros->primerParamNum),instruccion->parametros->segundoParamString, instruccion->parametros->segundoParamNum, instruccion->parametros->primerParamString);
    }
    else if(strcmp(token,"GOTO") == 0){
        instruccion->operacion = malloc(strlen(token)+1);
        strcpy(instruccion->operacion,token);
        instruccion->parametros = malloc(sizeof(t_parametros));
        instruccion->parametros->primerParamNum = malloc(sizeof(int));
        *(instruccion->parametros->primerParamNum) = atoi(strtok(NULL, " "));
        instruccion->parametros->primerParamString = NULL;
        instruccion->parametros->segundoParamNum = NULL;
        instruccion->parametros->segundoParamString = NULL;
        // log_info(mem_logger,"se guardo la instruccion %s %d %p %p %p", instruccion->operacion,  *(instruccion->parametros->primerParamNum), instruccion->parametros->primerParamString, instruccion->parametros->segundoParamNum, (void*)instruccion->parametros->segundoParamString);
    }
    else if(strcmp(token,"IO") == 0){
        instruccion->operacion = malloc(strlen(token)+1);
        strcpy(instruccion->operacion,token);
        instruccion->parametros = malloc(sizeof(t_parametros));
        instruccion->parametros->primerParamString =strdup(strtok(NULL, " "));
        instruccion->parametros->segundoParamNum = malloc(sizeof(int));
        *(instruccion->parametros->segundoParamNum) = atoi(strtok(NULL, " "));
        instruccion->parametros->primerParamNum = NULL;
        instruccion->parametros->segundoParamString = NULL;
        // log_info(mem_logger,"se guardo la instruccion %s %s %d %p %p " , instruccion->operacion, instruccion->parametros->primerParamString, *(instruccion->parametros->segundoParamNum), instruccion->parametros->primerParamNum, instruccion->parametros->segundoParamString);
    }
    else if(strcmp(token,"INIT_PROC") == 0){
        instruccion->operacion = malloc(strlen(token)+1);
        strcpy(instruccion->operacion,token);
        instruccion->parametros = malloc(sizeof(t_parametros));
        instruccion->parametros->primerParamString = strdup(strtok(NULL, " "));
        instruccion->parametros->segundoParamNum = malloc(sizeof(int));
        *(instruccion->parametros->segundoParamNum) = atoi(strtok(NULL, " "));
        instruccion->parametros->primerParamNum = NULL;
        instruccion->parametros->segundoParamString = NULL;
        // log_info(mem_logger,"se guardo la instruccion %s %s %d %p %p ", instruccion->operacion, instruccion->parametros->primerParamString, *(instruccion->parametros->segundoParamNum), instruccion->parametros->primerParamNum , instruccion->parametros->segundoParamString);
    } 
    else if(strcmp(token,"DUMP_MEMORY") == 0){
        instruccion->operacion = malloc(strlen(token)+1);
        strcpy(instruccion->operacion,token);
        instruccion->parametros = malloc(sizeof(t_parametros));
        instruccion->parametros->primerParamNum = NULL;
        instruccion->parametros->primerParamString = NULL;
        instruccion->parametros->segundoParamNum = NULL;
        instruccion->parametros->segundoParamString = NULL;
        // log_info(mem_logger,"se guardo la instruccion %s %p %p %p %p", instruccion->operacion, instruccion->parametros->primerParamNum, instruccion->parametros->primerParamString, instruccion->parametros->segundoParamNum, instruccion->parametros->segundoParamString);

    }
    else if(strcmp(token,"EXIT") == 0){
        instruccion->operacion = malloc(strlen(token)+1);
        strcpy(instruccion->operacion,token);
        instruccion->parametros = malloc(sizeof(t_parametros));
        instruccion->parametros->primerParamNum = NULL;
        instruccion->parametros->primerParamString = NULL;
        instruccion->parametros->segundoParamNum = NULL;
        instruccion->parametros->segundoParamString = NULL;
        // log_info(mem_logger,"se guardo la instruccion %s %p %p %p %p", instruccion->operacion, instruccion->parametros->primerParamNum, instruccion->parametros->primerParamString, instruccion->parametros->segundoParamNum, instruccion->parametros->segundoParamString);

    }
    else{
        // log_error(mem_logger,"Instruccion no valida");
    }

}

void cargar_instruccion_al_buffer(t_buffer* buffer, t_instruccion* instruccion){

    // log_debug(mem_logger,"se va a cargar la instruccion  n° %d - %s %p %p %p %p al buffer", instruccion->id,instruccion->operacion, instruccion->parametros->primerParamNum, 
    //     instruccion->parametros->segundoParamNum, instruccion->parametros->primerParamString, instruccion->parametros->segundoParamString);

    if(instruccion != NULL){
        cargar_string_al_buffer(buffer,instruccion->operacion);
    }
    
    if(instruccion->parametros->primerParamNum != NULL){
        cargar_entero_al_buffer(buffer, *(instruccion->parametros->primerParamNum));
        // log_debug(mem_logger,"se cargo el primer parametro de la instruccion %s con el valor %d", instruccion->operacion, *(instruccion->parametros->primerParamNum));
    }
    if(instruccion->parametros->primerParamString != NULL){
        cargar_string_al_buffer(buffer, instruccion->parametros->primerParamString);
        // log_debug(mem_logger,"se cargo el primer parametro de la instruccion %s con el valor %s", instruccion->operacion, instruccion->parametros->primerParamString);
    }
    if (instruccion->parametros->segundoParamNum != NULL){
        log_debug(mem_logger,"entra al if del segundo parametro como int");
        cargar_entero_al_buffer(buffer, *(instruccion->parametros->segundoParamNum));
        // log_debug(mem_logger,"se cargo el segundo parametro de la instruccion %s con el valor %d", instruccion->operacion, *(instruccion->parametros->segundoParamNum));
    }
    if(instruccion->parametros->segundoParamString != NULL){
        cargar_string_al_buffer(buffer, instruccion->parametros->segundoParamString);
        // log_debug(mem_logger,"se cargo el segundo parametro de la instruccion %s con el valor %s", instruccion->operacion, instruccion->parametros->segundoParamString);
    }
    else{
        // log_debug(mem_logger,"Instruccion sin parametros");
    }
}

void loguear_instruccion(int pid_proceso, t_instruccion* instruccion){

    if(instruccion->parametros->primerParamNum == NULL && instruccion->parametros->segundoParamNum == NULL){
        log_info(mem_logger,"## PID: <%d> - Obtener instrucción: <%d> - Instrucción: <%s> ", 
        pid_proceso,
        instruccion->id,
        instruccion->operacion);
    }
    else if(instruccion->parametros->primerParamString != NULL && instruccion->parametros->segundoParamNum != NULL){
        log_info(mem_logger,"## PID: <%d> - Obtener instrucción: <%d> - Instrucción: <%s> <%s> <%d> ", 
        pid_proceso,
        instruccion->id,
        instruccion->operacion,
        instruccion->parametros->primerParamString, 
        *(instruccion->parametros->segundoParamNum));
    }
    else if(instruccion->parametros->primerParamNum != NULL && instruccion->parametros->segundoParamNum != NULL){
    log_info(mem_logger,"## PID: <%d> - Obtener instrucción: <%d> - Instrucción: <%s> <%d> <%d> ", 
        pid_proceso,
        instruccion->id,
        instruccion->operacion,
        *(instruccion->parametros->primerParamNum), 
        *(instruccion->parametros->segundoParamNum));
    }
    else if(instruccion->parametros->primerParamNum != NULL && instruccion->parametros->segundoParamString!= NULL){
            log_info(mem_logger,"## PID: <%d> - Obtener instrucción: <%d> - Instrucción: <%s> <%d>  <%s> ", 
                pid_proceso,
                instruccion->id,
                instruccion->operacion,
                *(instruccion->parametros->primerParamNum), 
                instruccion->parametros->segundoParamString);
    }

    else{
        log_info(mem_logger,"## PID: <%d> - Obtener instrucción: <%d> - Instrucción: <%s> <%d>", 
            pid_proceso,
            instruccion->id,
            instruccion->operacion,
            *(instruccion->parametros->primerParamNum));
    }

}

void eliminarEstructurasDelProcesoEnMemoria(t_proceso* un_proceso){
    pthread_mutex_lock(&mutex_eliminar_estructuras);
    liberarPaginasAsignadasAlProceso(un_proceso);
    eliminarColaProceso(un_proceso);
    eliminarInstrucciones(un_proceso);
    imprimirMetricas(un_proceso);
    free(un_proceso);
    pthread_mutex_unlock(&mutex_eliminar_estructuras);
}

void liberarPaginasAsignadasAlProceso(t_proceso* un_proceso){
  for(int i = 0; i<list_size(un_proceso->lista_pag_del_proceso);i++){
        t_pagina* pagina = list_get(un_proceso->lista_pag_del_proceso,i);
        bool resultado = marcarPaginaComoLibreEnTablaPag(pagina,tabla_pag_nivel1);
        if(resultado){
            log_debug(mem_logger,"se seteo como libre la pagina del PID %d ", un_proceso->pid_proceso); // capaz deba comentarlo
        }
   }
   list_destroy(un_proceso->lista_pag_del_proceso);
}
// void destruirPagina(void* ptr) {
//     t_pagina* pag = (t_pagina*) ptr;
//     free(pag);
// }
void eliminarColaProceso(t_proceso* un_proceso){
        free(un_proceso->colaDeCaminoAPag);
        free(un_proceso->colaDECaminoNumTP); 
}
void eliminarInstrucciones(t_proceso* un_proceso){

    for(int i=0; i<list_size(un_proceso->lista_instrucciones_proceso); i++){
        t_instruccion* instruccion = list_remove(un_proceso->lista_instrucciones_proceso,i);
        free(instruccion->operacion);
        free(instruccion->parametros->primerParamNum);
        free(instruccion->parametros->primerParamString);
        free(instruccion->parametros->segundoParamNum);
        free(instruccion->parametros->segundoParamString);
        free(instruccion->parametros);
        free(instruccion);
    }
    list_destroy(un_proceso->lista_instrucciones_proceso);
}
//LOG OBLIGATORIO
void imprimirMetricas(t_proceso* un_proceso){
    log_info(mem_logger, "Destrucción de Proceso: ## PID: <%d>- Proceso Destruido - Métricas - Acc.T.Pag: <%d>; Inst.Sol.: <%d.>; SWAP: <%d>; Mem.Prin.: <%d>; Lec.Mem.: <%d>; Esc.Mem.: <%d>",
    un_proceso->pid_proceso,un_proceso->cantAccesosTP,un_proceso->cantInstruccionesSolicitadas,un_proceso->cantBajadasSwap,un_proceso->cantSubidasMP, un_proceso->cantLecturasMem,un_proceso->cantEscriturasMem);
}
void iniciar_semaforos(){
    pthread_mutex_init(&mutex_pc,NULL);
    pthread_mutex_init(&mutex_pid,NULL);
    pthread_mutex_init(&mutex_add_lista,NULL);
    pthread_mutex_init(&mutex_contador,NULL);
    pthread_mutex_init(&mutex_calculoTamMem,NULL);
    pthread_mutex_init(&mutex_remove_lista,NULL);
    pthread_mutex_init(&mutex_get_listaProcesos,NULL);
    pthread_mutex_init(&mutex_get_listaInstrucciones,NULL);
    pthread_mutex_init(&incrementarCantLecturas,NULL);
    pthread_mutex_init(&incrementarCantSolicitudesInstr,NULL);
    pthread_mutex_init(&incrementarCantEscriturasMem,NULL);
    pthread_mutex_init(&cant_AccesosTP,NULL);
    pthread_mutex_init(&cant_bajadas_swap,NULL);
    pthread_mutex_init(&cant_bajadas_MP, NULL);
    pthread_mutex_init(&numeroDeTablaCreada,NULL);
    pthread_mutex_init(&mutexParaNumTP,NULL);
    pthread_mutex_init(&mutex_lista_paginas_proceso,NULL);
    pthread_mutex_init(&mutex_entradas_swap,NULL);
    pthread_mutex_init(&mutex_eliminar_estructuras,NULL);
    pthread_mutex_init(&mutex_para_saber_si_hay_procesos_suspendidos,NULL);
    pthread_mutex_init(&mutex_lista_procesos_suspendidos,NULL);
    pthread_mutex_init(&mutex_send_ok_kernel,NULL);
    sem_init(&proceso_escribiendo,0,1);
    sem_init(&modificar_tamanio_ocupado_en_memoria,0,0);
    pthread_mutex_init(&mutex_buscando_num_pag_global,NULL);
    pthread_mutex_init(&send_ok_cpu,NULL);
    pthread_mutex_init(&mutex_leer_pagina,NULL);
    pthread_mutex_init(&mutex_actualizar_pagina,NULL);
    pthread_mutex_init(&escribir_valor,NULL);
    pthread_mutex_init(&solicitar_valor,NULL);
    pthread_mutex_init(&pedir_marco,NULL);
    pthread_mutex_init(&pedir_tabla,NULL);
    pthread_mutex_init(&mutex_conexion_con_kernel, NULL);
}



void crear_paginacion_jerarquica_nivel_n(){
    
    if(CANT_NIVELES != 1){
         //creo n punteros para cada tabla del siguiente nivel, limitado por la cantidad de entradas
        for(int i = 0; i < ENTRADAS_POR_TABLA; i++){
            t_tabla* tabla_pagSigNivel = malloc(sizeof(t_tabla));
            tabla_pagSigNivel->lista_paginas = list_create();
            tabla_pagSigNivel->ocupadaPorCompleto = false;
            tabla_pagSigNivel->nivel = 2;
            tabla_pagSigNivel->tabla_paginasPadre = tabla_pag_nivel1; 
            pthread_mutex_lock(&numeroDeTablaCreada);
            tabla_pagSigNivel->numTablaCreada  =  numTablaCreada++; // empieza 
            log_debug(mem_logger, "Se crea la tabla %d nivel %d ", tabla_pagSigNivel->numTablaCreada, tabla_pagSigNivel->nivel);
            pthread_mutex_unlock(&numeroDeTablaCreada);

            list_add_in_index(tabla_pag_nivel1->lista_paginas, i, tabla_pagSigNivel);// en la primera iteración ( i = 0), se crea la primera tabla de Pag de 2do nivel

            if(tabla_pagSigNivel->nivel == CANT_NIVELES){ // si el nivel es 2 entonces aca terminaria todo
                rellenarTablaDeUltimoNivel(tabla_pag_nivel1->lista_paginas, i);
            //     log_debug(mem_logger, "El nivel alcanzado es %d" ,tabla_pagSigNivel->nivel);
            }
           
            else{//pero si el nivel > 2, se le pasa por cada iteracion (por cada tabla creada del segundo nivel), se le pasa la tabla del nivel 3 asociada a las tablas de nivel 2
             
                int nivelActual = tabla_pagSigNivel->nivel; // 2
                // t_list* tabla_pag_nivel_alcanzado = tabla_pagSigNivel->lista_paginas; //primera tabla de nivel 2

                nivelActual = agregarUnNivelMas(nivelActual, CANT_NIVELES,tabla_pagSigNivel); // (2,5,primera tabla de nivel 2, 3)
                // log_debug(mem_logger, "El nivel alcanzado es %d" ,nivelActual);
    
            }
        }
    }
    else {

        for(int i = 0; i < ENTRADAS_POR_TABLA; i++){
            //por cada entrada hay una pagina 
            t_pagina* unaPagina = malloc(sizeof(t_pagina));
            pthread_mutex_lock(&mutexParaNumTP);
            unaPagina->num_pagina = numero_de_pagina++; 
            pthread_mutex_unlock(&mutexParaNumTP);
             //numero de pagina, al principio todos los frames estan libres, por lo que se asigna el primer frame libre y asi con todas las paginas, o sea van a tener el indice de los frames secuencialmente
            unaPagina->frame.base = -1; //  0 para el primer frame que se asigne
            unaPagina->frame.limite = -1; // 63 para el primer frame que se asigne
            unaPagina->bit_ocupado = 0;
            unaPagina->num_frame = -1; //inicializo el numero de frame en -1 porque no se le asigno ningun frame todavia
            unaPagina->bit_modificado = 0;
            list_add(tabla_pag_nivel1->lista_paginas, unaPagina);
        }

    }
}
//aca vendría la tabla de paginas que correspende al nivel N-1
void rellenarTablaDeUltimoNivel(t_list* tablaPagQueContieneAlAnteUltimoNivel, int i){
    t_tabla* tablaPagUltima = list_get(tablaPagQueContieneAlAnteUltimoNivel, i);
    log_debug(mem_logger, "Se creo la tabla de paginas %d nivel %d  ", tablaPagUltima->numTablaCreada, tablaPagUltima->nivel);
    for(int j = 0; j < ENTRADAS_POR_TABLA; j++){
        t_pagina* unaPagina = malloc(sizeof(t_pagina));
        pthread_mutex_lock(&mutexParaNumTP);
        unaPagina->num_pagina = numero_de_pagina++;
        pthread_mutex_unlock(&mutexParaNumTP);
         //numero de pagina, al principio todos los frames estan libres, por lo que se asigna el primer frame libre y asi con todas las paginas, o sea van a tener el indice de los frames secuencialmente
        unaPagina->frame.base = -1; //  0 para el primer frame que se asigne
        unaPagina->frame.limite = -1; // 63 para el primer frame que se asigne
        unaPagina->bit_ocupado = 0;
        unaPagina->num_frame = -1; //inicializo el numero de frame en -1 porque no se le asigno ningun frame todavia
        unaPagina->bit_modificado = 0;
        list_add_in_index(tablaPagUltima->lista_paginas, j, unaPagina);
    }
}
//(2,5,1era Lista de la TN1, 3) //(3,5,TablaN3Subi,2)//(4,5,TablaN4Subi,1)
int agregarUnNivelMas(int nivelActual, int nivelALlegar,t_tabla* tablaDePagDelUltimoNivelAlcanzado){
    
    int nivelAumentado = nivelActual + 1;// nivel 3 // nivel 4 //nivel 5

    for(int i = 0; i < ENTRADAS_POR_TABLA; i++){
        t_tabla* tabla_pagSig = malloc(sizeof(t_tabla));

        if (!tabla_pagSig) {
            log_error(mem_logger, "Fallo al reservar memoria para tabla de nivel %d", nivelAumentado);
             return -1;
        }
        tabla_pagSig->lista_paginas = list_create(); //aca se crean las tablas nivel 3//aca se agregan las de nivel 4 //aca se agregan las de nivel 5
        tabla_pagSig->ocupadaPorCompleto = 0;
        tabla_pagSig->nivel = nivelAumentado;
        tabla_pagSig->tabla_paginasPadre = tablaDePagDelUltimoNivelAlcanzado; //asocio la tabla de paginas padre con la tabla de paginas del ultimo nivel alcanzado
        pthread_mutex_lock(&numeroDeTablaCreada);
        tabla_pagSig->numTablaCreada = numTablaCreada++; 
        log_debug(mem_logger, "Se crea la tabla %d nivel %d", tabla_pagSig->numTablaCreada,tabla_pagSig->nivel);
        pthread_mutex_unlock(&numeroDeTablaCreada);
        list_add_in_index(tablaDePagDelUltimoNivelAlcanzado->lista_paginas, i, tabla_pagSig);//(tablaN2,subi,TablaN3)//(tablaN3,subi,Tabla4)//(tabalN4,i,Tabla5)
        
    }
    
    t_tabla* tabla_reciente = list_get(tablaDePagDelUltimoNivelAlcanzado->lista_paginas, 0);

    if(tabla_reciente->nivel == nivelALlegar){// 3 != 5 //4!=5// 5==5
        for(int k = 0;k<ENTRADAS_POR_TABLA;k++){
            rellenarTablaDeUltimoNivel(tablaDePagDelUltimoNivelAlcanzado->lista_paginas, k);
            
        }
        return tabla_reciente->nivel;
    }
    else{
          for(int z = 0; z< ENTRADAS_POR_TABLA; z++){
                t_tabla* primeraTablaPagDelUltimoNivel = list_get(tablaDePagDelUltimoNivelAlcanzado->lista_paginas,z);//(TablaPagN2,subz)-> en la primera iteracion agarro una T3 sub z//
               int nivelAlcanzado = agregarUnNivelMas(primeraTablaPagDelUltimoNivel->nivel,CANT_NIVELES,primeraTablaPagDelUltimoNivel);//(3,5,TablaN3Subi,2)//(4,5,TablaN4Subi,1) => ese indiceFinal ya no lo voy a usar
                // log_debug(mem_logger, "El nivel alcanzado es %d", nivelAlcanzado);
            }   
          
            return tabla_reciente->nivel;
    }
           
}

bool paginasOcupadas(void* ptr){
    t_pagina* pagina = (t_pagina*) ptr;
    return pagina->bit_ocupado == 1;
}

bool verificarSiTodasLasPagEstanOcupadas(t_list* lista){
    return list_all_satisfy(lista, paginasOcupadas);
}



int asignarNumDePrimerFrameLIbre(){
    for (int i = 0; i < bitarray_get_max_bit(bitarray_frames); i++) {
        if (!bitarray_test_bit(bitarray_frames, i)) {
            bitarray_set_bit(bitarray_frames, i); // Marca el frame como ocupado
            return i; // le asigno del 0 al 63 frame
            break;
        }
    }
    return -1; // Si no hay frames libres
}

void asignarTablasDePagAlProceso(int tamanioProceso, t_list* listaDePaginasAsociadasAlPid, int pid){ // primera iteracion =>(4096, listaPag)  
    double cant_paginas = ceil((double)tamanioProceso / (double)TAM_PAGINA); // 64 pag
    int cantPagParaElProceso = (int)cant_paginas;// 64 pag
    // sem_post(&modificar_tamanio_ocupado_en_memoria);
    log_debug(mem_logger, "El proceso va a tener %d paginas",cantPagParaElProceso);

    int tamanioOcupado = chequear_y_actualizar_suspensiones(tamanioProceso,CREATE);

    log_info(mem_logger, "TAM OCUPADO %d con PROCESO NUEVO ",tamanioOcupado);

    int asignadas = 0;

 
    asignar_paginas_recursivo(tabla_pag_nivel1, tabla_pag_nivel1->nivel, &asignadas, cantPagParaElProceso, listaDePaginasAsociadasAlPid, pid);//(T1,1,0,64,listaPag)
    
}

void asignar_paginas_recursivo(t_tabla* tabla_actual, int nivel_actual, int* paginas_asignadas, int total_paginas, t_list* paginas_asociadas, int pid){//(T1,1,0,64,listaPag)//(T2,2,0,64,listaPag)//(T3,3,0,64,listaPag) 
    if (*paginas_asignadas == total_paginas) return; //0>= 64 -> Falso

    if (nivel_actual == CANT_NIVELES) { // 1 != 3// 2!=3// 3 == 3 ENTRA AL FOR
        // Estamos en el último nivel, donde están las páginas reales
        for (int i = 0; i < list_size(tabla_actual->lista_paginas); i++){
            t_pagina* pagina = list_get(tabla_actual->lista_paginas, i);
            if (pagina->bit_ocupado == 0) {
                pagina->bit_ocupado = 1;
                pagina->num_pag_local_pid = *paginas_asignadas;//0/1//2//3//4//5//6//7
                pagina->num_frame = asignarNumDePrimerFrameLIbre();
                pagina->frame.base = pagina->num_frame * TAM_PAGINA;// 0 // 64
                pagina->frame.limite = pagina->frame.base + TAM_PAGINA - 1; //63 //
                list_add(paginas_asociadas, pagina);
                (*paginas_asignadas)++;                       //1//2//3//4//5//6//7//8
                log_info(mem_logger, "Se le asigno la pagina %d, pag  %d del PID %d",pagina->num_pagina,pagina->num_pag_local_pid,pid);                  
                if (*paginas_asignadas == total_paginas) return;
            }
        }

        if(verificarSiTodasLasPagEstanOcupadas(tabla_actual->lista_paginas )){ // si todas las paginas de la subtabla estan ocupadas, entonces marco la tabla de paginas como ocupada por completo
            tabla_actual->ocupadaPorCompleto = true;
            log_debug(mem_logger,"Se lleno la tabla de paginas de nivel %d", tabla_actual->nivel);

        }
    } else {
        // Estamos en un nivel intermedio
        for (int i = 0; i < list_size(tabla_actual->lista_paginas); i++){ // tomo la lista de T1 // tomo la lista de T2
            t_tabla* subtabla = list_get(tabla_actual->lista_paginas, i);// (T1,0) tomo la primera T2(empiezo por la 0, luego la 1 y asi hasta la sub 3) // (T2,0) tomo la primera T3
            asignar_paginas_recursivo(subtabla, subtabla->nivel, paginas_asignadas, total_paginas, paginas_asociadas,pid);//(T2,2,0,64,listaPag)// (T3,3,0,64,listaPag)
            if (*paginas_asignadas >= total_paginas) return;
        }
    }
}

char* pedir_valor_a_memoria(int direccionFisica, int tamanio, t_proceso* proceso){
    if (direccionFisica < 0 || direccionFisica + tamanio > TAM_MEMORIA) {
        log_error(mem_logger, "Acceso fuera del rango de memoria");
        return NULL;

    }

    char* destino = malloc(tamanio + 1);  // +1 para usarlo como string
    if (!destino) {
        perror("malloc");
        return NULL;
    }
    memcpy(destino, (char*)memoria_usuario + direccionFisica, tamanio);
    destino[tamanio] = '\0';  
    log_debug(mem_logger, "La pagina del PID %d a enviar tiene este contenido %s ",proceso->pid_proceso,destino);
    return destino;
}

char* pedirPaginaCompetaAMemoria(int direccionFisica, int numPag, int tamanio, t_proceso* proceso){
    if (direccionFisica < 0 || direccionFisica + tamanio > TAM_MEMORIA) {
        log_error(mem_logger, "Acceso fuera del rango de memoria");
         return NULL;
    }
    if (numPag == -1) {
        log_error(mem_logger, "La página es -1 en pedir página completa en Memoria");
        return NULL;
    }

    char* contenidoPagina = malloc(tamanio);

    if(!contenidoPagina){
        perror("malloc");
        log_error(mem_logger, "No se pudo asignar memoria para enviar el contenido de página a CPU");
        return NULL;
    }
    if(ENTRADAS_POR_TABLA != 8){
        memcpy(contenidoPagina, (char*)memoria_usuario + numPag*TAM_PAGINA, tamanio);
        contenidoPagina[TAM_PAGINA-1]='\0';
        log_debug(mem_logger,"El contenido de la pagina antes de ir a CPU es %s", contenidoPagina);
        return contenidoPagina;
    }else{
        int numMarco = buscarPaginaSegunPag(numPag,proceso);

        // if (direccionBaseFisica < 0 || direccionBaseFisica + tamanio > TAM_MEMORIA) {
        //     log_error(mem_logger, "Acceso fuera del rango de memoria");
        //     exit(EXIT_FAILURE);
        // }

        if(numMarco == -1){
            log_debug(mem_logger,"marco -1 porque no se encontro el num pag global para el pid %d",proceso->pid_proceso);
        }
        memcpy(contenidoPagina, (char*)memoria_usuario + (numMarco*TAM_PAGINA), tamanio);
        contenidoPagina[TAM_PAGINA-1]='\0';
        log_debug(mem_logger,"El contenido de la pagina %d del PID %d antes de ir a CPU es %s", numPag, proceso->pid_proceso,contenidoPagina);
        return contenidoPagina;
    }
 
}

int buscarPaginaSegunPag(int numPag, t_proceso*  proceso){
    for(int i = 0; i<list_size(proceso->lista_pag_del_proceso);i++){
            t_pagina* pagina = list_get(proceso->lista_pag_del_proceso,i);
            if(pagina->num_pagina== numPag){
                return pagina->num_frame;
                break;
            }
    } 
    return -1;
}

int buscarTablaDelSigNivel(int nivelActualTabla, int entrada, int pid_proceso){

    t_tabla* tablaQueEsDelSigNivel = tabla_pag_nivel1;
    // t_proceso* proceso = list_get(lista_procesos, pid_proceso);
    t_proceso* proceso = buscar_proceso(pid_proceso);

    //tengo que buscar el numero de tabla
  
    if(tablaQueEsDelSigNivel->nivel != nivelActualTabla){
        while(tablaQueEsDelSigNivel->nivel != nivelActualTabla){ // 1 != 2  

            int* entradaAux;

            while(!queue_is_empty(proceso->colaDECaminoNumTP)){
                entradaAux = queue_pop(proceso->colaDECaminoNumTP); //[1]
                int entradaAuxi = *entradaAux;// aca se guardó la entrada incial
                tablaQueEsDelSigNivel = list_get(tablaQueEsDelSigNivel->lista_paginas, entradaAuxi); //saco la tabla 2
            }
            
            if(tablaQueEsDelSigNivel == NULL){
                log_error(mem_logger, "No se encontró la tabla del siguiente nivel para el nivel %d y entrada %d", nivelActualTabla, entrada);
                return -1; // Error al buscar la tabla del siguiente nivel
            }

            if(nivelActualTabla  ==  (CANT_NIVELES-1)){
                queue_clean_and_destroy_elements(proceso->colaDECaminoNumTP, free);
            }else{

                int* entrada_nueva = malloc(sizeof(int));
                *entrada_nueva = entrada;
                queue_push(proceso->colaDECaminoNumTP, entrada_nueva);
            }
           
        }
            if(proceso!=NULL){
                int *entrada_ptr = malloc(sizeof(int));
                *entrada_ptr = entrada;
                queue_push(proceso->colaDeCaminoAPag, entrada_ptr);
                log_debug(mem_logger, "Valor de la entrada recibida: %d de nivel %d", *entrada_ptr,tablaQueEsDelSigNivel->nivel);
                tablaQueEsDelSigNivel = list_get(tablaQueEsDelSigNivel->lista_paginas, entrada); // list_get(listaTP2,1)-> Tabla 4
                return tablaQueEsDelSigNivel->numTablaCreada; //tabla num 4
                
            }else{
                log_error(mem_logger, "No se encontró el proceso con PID %d al buscar la tabla del siguiente nivel", pid_proceso);
                return -1; // Error al buscar el proceso    
            }      
    }
    else{
          
        if(proceso!=NULL){
            int *entrada_ptr2 = malloc(sizeof(int));
            *entrada_ptr2 = entrada;
            queue_push(proceso->colaDeCaminoAPag, entrada_ptr2);
            log_debug(mem_logger, "Valor de la entrada recibida: %d del nivel %d, se procede a guardar", *entrada_ptr2,tablaQueEsDelSigNivel->nivel);

        }else{
            log_error(mem_logger, "No se encontró el proceso con PID %d al buscar la tabla del siguiente nivel", pid_proceso);
            return -1; // Error al buscar el proceso    
        }
        tablaQueEsDelSigNivel = list_get(tablaQueEsDelSigNivel->lista_paginas, entrada);
        int *entradaAuxNum = malloc(sizeof(int));
        *entradaAuxNum = entrada;
        queue_push(proceso->colaDECaminoNumTP,entradaAuxNum); // se guarda la entrada 0
        return tablaQueEsDelSigNivel->numTablaCreada; // ya no Retorna el nivel de la tabla encontrada sino el num de tabla creada --> la primera tabla de la tabla 1 es la 2

    }
 
}

int obtenerMarco(int ultimoNivel, int entradaPagina, int pid_proceso){

    if(CANT_NIVELES != 1){

        t_tabla* tablaActual = tabla_pag_nivel1;
        // t_proceso* proceso = list_get(lista_procesos, pid_proceso);
        t_proceso* proceso = buscar_proceso(pid_proceso);
            while(tablaActual->nivel < ultimoNivel && tablaActual != NULL && proceso != NULL){
                int* entrada_ptr2 = queue_pop(proceso->colaDeCaminoAPag);
                int entrada2 = *entrada_ptr2;
                tablaActual = list_get(tablaActual->lista_paginas, entrada2);
                if(tablaActual == NULL){
                    log_error(mem_logger, "No se encontró la tabla del nivel %d y entrada %d", ultimoNivel, entradaPagina);
                    return -1; // Error al buscar la tabla del siguiente nivel
                }
                free(entrada_ptr2);  // Liberar la memoria del entero que se sacó de la cola
            }

        t_pagina* pagina = list_get(tablaActual->lista_paginas, entradaPagina);
            if(!pagina){
                log_error(mem_logger, "No se encontró la página en la tabla del PID %d", pid_proceso);
                return -1;
            }
        log_debug(mem_logger, "Se va enviar el marco %d  de la pagina %d ", pagina->num_frame, pagina->num_pagina);

        return pagina->num_frame; // Retorna el número de marco asociado a la página solicitada
    
    }
    else if(CANT_NIVELES == 1){
        t_pagina* pagina = list_get(tabla_pag_nivel1->lista_paginas, entradaPagina);
        log_debug(mem_logger, "Se obtuvo el marco %d de la pagina %d  para el PID %d", pagina->num_frame, pagina->num_pagina, pid_proceso);
        return pagina->num_frame; // Retorna el número de marco asociado a la página solicitada
    }
    else{
        return -1; // Error 
    }
    
   
}

void escribir_valor_en_memoria(char* datoAEScribir, int direccionFisica){
    int tamanioTexto = strlen(datoAEScribir);  

    if (direccionFisica < 0 || direccionFisica + tamanioTexto > TAM_MEMORIA){
        log_error(mem_logger, "Escritura fuera del rango de memoria");
        return;
    }
    memcpy((char*)memoria_usuario + direccionFisica, datoAEScribir, tamanioTexto);
}

void escribirPaginaCompleta(char* datoAEScribir, int direccionFisica, int numPagEscrita, t_proceso* proceso){
    //me entraga el num de pagina global

    if(ENTRADAS_POR_TABLA !=8){
        if (direccionFisica < 0 || direccionFisica + TAM_PAGINA > TAM_MEMORIA){
            log_error(mem_logger, "Escritura fuera del rango de memoria");
            return;
        }
        char* contenido = datoAEScribir;
        contenido[TAM_PAGINA-1] = '\0';
        log_debug(mem_logger,"El contenido de la pagina a actualizar porque viene de CPU es %s ",contenido);
        memcpy((char*)memoria_usuario + numPagEscrita*TAM_PAGINA, datoAEScribir, TAM_PAGINA); // esto es cierto para los primeros casos
    
    }else{
        for(int i= 0; i<list_size(proceso->lista_pag_del_proceso);i++){
            t_pagina* pag = list_get(proceso->lista_pag_del_proceso, i);
    
            if(pag->num_pagina == numPagEscrita){
                char* contenido = datoAEScribir;
                contenido[TAM_PAGINA-1] = '\0';
                log_debug(mem_logger,"El contenido de la pagina %d a actualizar porque viene de CPU es %s ",pag->num_pagina,contenido);
                memcpy((char*)memoria_usuario +(pag->frame.base) , datoAEScribir, TAM_PAGINA);
            }
        }
    }
  
}

void marcarPaginasEscritas(t_proceso* proceso,char* datoAEScribir, int dirFisica){ //la direccion fisica siempre es la correcta
    if (proceso == NULL || proceso->lista_pag_del_proceso == NULL) {
        log_error(mem_logger, "Proceso o lista de Pags nulas en marcarPaginasEscritas");
        return;
    }

    for(int i= 0; i<list_size(proceso->lista_pag_del_proceso);i++){
        t_pagina* pag = list_get(proceso->lista_pag_del_proceso, i);

        if(pag->frame.base <= dirFisica && pag->frame.limite > dirFisica){
                pag->bit_modificado = 1;
                break;
        }
    }
}

int buscarNroPaginaAtravesDeDirFisicaGlobal(int direccionFisicaPag, int pid){
    // t_proceso* proceso = list_get(lista_procesos, pid);
    t_proceso* proceso = buscar_proceso(pid);

    int numPag = -1;

    if(ENTRADAS_POR_TABLA != 8){
        for(int i=0; i<list_size(proceso->lista_pag_del_proceso);i++){
            t_pagina* unaPagina = list_get(proceso->lista_pag_del_proceso, i);
            if(unaPagina->frame.base <= direccionFisicaPag && unaPagina->frame.limite > direccionFisicaPag){
                numPag = unaPagina->num_pagina;
                break;
            }
        }
    }else{
        for(int i=0; i<list_size(proceso->lista_pag_del_proceso);i++){
            t_pagina* unaPagina = list_get(proceso->lista_pag_del_proceso, i);
            if(unaPagina->frame.base<= direccionFisicaPag && unaPagina->frame.limite > direccionFisicaPag){
                numPag = unaPagina->num_pagina;
                break;
            }
        }
    }

   
    if(numPag != -1){
        return numPag;
    }else{
        log_error(mem_logger, "La página es -1 porque no se encontro la pág en la lista de páginas del pid %d",pid);
        return -1;
    }  
}

void escribirPaginasEnSwap(int pidDeSuspendido){
    
    t_proceso* procesoSuspendido = buscar_proceso(pidDeSuspendido); // uso buscar_proceso porque rompe en la prueba de corto plazo con fifo
    if(procesoSuspendido == NULL){
        return;
    }
    pthread_mutex_lock(&cant_bajadas_swap);
    procesoSuspendido->cantBajadasSwap ++;
    pthread_mutex_unlock(&cant_bajadas_swap);

    log_info(mem_logger, "Hay %d paginas asignadas al PID %d",list_size(procesoSuspendido->lista_pag_del_proceso),pidDeSuspendido);
    pthread_mutex_lock(&mutex_entradas_swap);
    while(!list_is_empty(procesoSuspendido->lista_pag_del_proceso)){
        t_pagina* unaPagina = list_get(procesoSuspendido->lista_pag_del_proceso,0);
        log_debug(mem_logger, "Se va a enviar el contenido de la pagina %d a SWAP", unaPagina->num_pagina);
        
        int posicionEntradaSwap = 0;
            if(procesoSuspendido->tamanio_proceso !=1 && procesoSuspendido->tamanio_proceso !=64 && procesoSuspendido->tamanio_proceso !=128 && procesoSuspendido->tamanio_proceso !=32 && ENTRADAS_POR_TABLA == 8){
                 posicionEntradaSwap = seteoPaginaOcupadaEnSwap(unaPagina->num_pagina, pidDeSuspendido);  // retorna la base de la entrada en swap

                if(posicionEntradaSwap == -1){
                    log_error(mem_logger, "no se encontró posicion de entrada Swap para PID %d",pidDeSuspendido);
                    return;
                }
                if(unaPagina->bit_modificado){ // si la pagina tiene algo escrito 
                    log_debug(mem_logger,"Sigue la suspension del proceso linea 1415");
                    char* contenido = buscarContenidoDePagina(unaPagina);
                        if(posicionEntradaSwap * TAM_PAGINA >= TAM_SWAP){
                            log_warning(mem_logger, "Posición de entrada Swap %d excede el tamaño de SWAP", posicionEntradaSwap);
                        }
                    log_debug(mem_logger,"El contenido de la pagina %d del PID %d a enviar a SWAP va ir a la entrada %d", unaPagina->num_pagina, pidDeSuspendido,  posicionEntradaSwap);
                    memcpy((char*)swapFileMapeado + posicionEntradaSwap, contenido, TAM_PAGINA);
                    free(contenido);
                }
                else{//para las paginas vacias
                    log_debug(mem_logger,"Sigue la suspension del proceso linea 1421");
                    memset((char*)swapFileMapeado + posicionEntradaSwap, 0, TAM_PAGINA);
                }
            }
       
        bitarray_clean_bit(bitarray_frames, unaPagina->num_frame);
        unaPagina->bit_ocupado = 0;
        unaPagina->frame.base = -1;
        unaPagina->frame.limite = -1;
        unaPagina->num_frame = -1;  
        unaPagina->num_pag_local_pid = -1;   
        // if(marcarPaginaComoLibreEnTablaPag(unaPagina, tabla_pag_nivel1)){
        //     log_debug(mem_logger,"Se libero la página %d del PID: %d ", unaPagina->num_pagina,pidDeSuspendido);
        // }else{
        //     log_debug(mem_logger, "No se pudo liberar la página %d del PID: %d", unaPagina->num_pagina, pidDeSuspendido);
        // }
        list_remove(procesoSuspendido->lista_pag_del_proceso, 0); // no se libera la pagina porque todavia pertenece la tabla de paginas
     
    }
    pthread_mutex_unlock(&mutex_entradas_swap);

    int tam = chequear_y_actualizar_suspensiones(procesoSuspendido->tamanio_proceso, SE_SUSPENDE);
    log_info(mem_logger, "TAM OCUPADO PORQUE SE SUSPENDIÓ AL PID %d ES %d",pidDeSuspendido,tam);
 

}

int chequear_y_actualizar_suspensiones(int tam_proceso_que_quiere_ingresar, tipo_consulta tipo_modificacion){
  
    int cant_marcos = 0;
    int espacio_nuevo_ocupado = 0;

    if(tipo_modificacion == INGRESO){

        pthread_mutex_lock(&mutex_calculoTamMem);
        int tamanioDisponibleConProcesoNuevo = TAM_MEMORIA - (TAM_OCUPADO + tam_proceso_que_quiere_ingresar); //para saber si el espacio del nuevo proceso entra en memoria
        pthread_mutex_unlock(&mutex_calculoTamMem);
        // sem_post(&modificar_tamanio_ocupado_en_memoria);

        return tamanioDisponibleConProcesoNuevo;
    }
    else if (tipo_modificacion == CREATE){

        pthread_mutex_lock(&mutex_calculoTamMem);    
        cant_marcos = (int)(ceil((double)tam_proceso_que_quiere_ingresar/ TAM_PAGINA));
        espacio_nuevo_ocupado = cant_marcos * TAM_PAGINA;
        TAM_OCUPADO+= espacio_nuevo_ocupado;
        pthread_mutex_unlock(&mutex_calculoTamMem);
        // sem_post(&modificar_tamanio_ocupado_en_memoria);


        return TAM_OCUPADO;

    }else if (tipo_modificacion == SE_SUSPENDE){
        
        pthread_mutex_lock(&mutex_calculoTamMem);    
        cant_marcos = (int)(ceil((double)tam_proceso_que_quiere_ingresar/ TAM_PAGINA));
        espacio_nuevo_ocupado = cant_marcos * TAM_PAGINA;
        TAM_OCUPADO-= espacio_nuevo_ocupado;
        pthread_mutex_unlock(&mutex_calculoTamMem);
        // sem_post(&modificar_tamanio_ocupado_en_memoria);


        return TAM_OCUPADO;

    }else if (tipo_modificacion == SE_DESUSPENDE){
        pthread_mutex_lock(&mutex_calculoTamMem);    
        cant_marcos = (int)(ceil((double)tam_proceso_que_quiere_ingresar/ TAM_PAGINA));
        espacio_nuevo_ocupado = cant_marcos * TAM_PAGINA;
        TAM_OCUPADO+= espacio_nuevo_ocupado;
        pthread_mutex_unlock(&mutex_calculoTamMem);
        // sem_post(&modificar_tamanio_ocupado_en_memoria);


        return TAM_OCUPADO;
    }else if (tipo_modificacion == SE_MUERE){
        pthread_mutex_lock(&mutex_calculoTamMem);    
        cant_marcos = (int)(ceil((double)tam_proceso_que_quiere_ingresar/ TAM_PAGINA));
        espacio_nuevo_ocupado = cant_marcos * TAM_PAGINA;
        TAM_OCUPADO-= espacio_nuevo_ocupado;
        pthread_mutex_unlock(&mutex_calculoTamMem);
        // sem_post(&modificar_tamanio_ocupado_en_memoria);


        return TAM_OCUPADO;
    }

    // pthread_mutex_unlock(&mutex_lista_procesos_suspendidos);

    // sem_post(&modificar_tamanio_ocupado_en_memoria);
    return -2;
}


bool seteoAlFrameComoLibre(t_pagina* unaPagina, t_tabla* tablaDeUltimoNivel){
    for (int j = 0; j < list_size(tablaDeUltimoNivel->lista_paginas); j++) {
        t_pagina* pagina = list_get(tablaDeUltimoNivel->lista_paginas, j);
        if (unaPagina->num_pagina == pagina->num_pagina) {
            bitarray_clean_bit(bitarray_frames, pagina->num_frame);
            pagina->bit_ocupado = 0;
            pagina->frame.base = -1;
            pagina->frame.limite = -1;
            pagina->num_frame = -1;
            return true;
        }
    }
    return false;
}

bool marcarPaginaComoLibreEnTablaPag(t_pagina* unaPagina, t_tabla* tablaActual){ 
    if (tablaActual->nivel == CANT_NIVELES) {
        return seteoAlFrameComoLibre(unaPagina, tablaActual);
    } else {
        for (int i = 0; i < list_size(tablaActual->lista_paginas); i++) {
            t_tabla* subtabla = list_get(tablaActual->lista_paginas, i);
            if (marcarPaginaComoLibreEnTablaPag(unaPagina, subtabla)) {
                return true; // cortar recursión si ya la encontró
            }
        }
    }
    return false;
}
void escribirPaginasDeSwapAMP(int pidDelDes_Suspendido){
    t_proceso* procesoAdesuspender = buscar_proceso(pidDelDes_Suspendido);
    pthread_mutex_lock(&cant_bajadas_MP);
    procesoAdesuspender->cantSubidasMP ++;
    pthread_mutex_unlock(&cant_bajadas_MP);

    liberarPaginasEnSwap(pidDelDes_Suspendido);


    log_info(mem_logger, "Hay %d paginas asignadas al PID %d DESPUES DE VOLVER DE SWAP",list_size(procesoAdesuspender->lista_pag_del_proceso),pidDelDes_Suspendido);
   
    int tam = chequear_y_actualizar_suspensiones(procesoAdesuspender->tamanio_proceso,SE_DESUSPENDE);
    log_info(mem_logger, "TAM OCUPADO DESPUES DE VOLVER DE SUSPEND %d porque regresa PID %d",tam,pidDelDes_Suspendido);
    
}

void liberarPaginasEnSwap(int pidDelDes_Suspendido){ // me busca todas las paginas en Swap que le pertenecen al pid
    pthread_mutex_lock(&mutex_entradas_swap);
    for(int i=0;i<list_size(entradasSwap);i++){
        t_entradaSwap* unaPaginaSwap = list_get(entradasSwap, i);
        if(unaPaginaSwap->pid == pidDelDes_Suspendido && unaPaginaSwap->base != -1){
            asignarFrameALaPagina(unaPaginaSwap); // siempre asumiendo que lo que se copio en una pag del swap, era todo lo tenia el proceso y lo puedo cargar a un SOLO frame
        }
    }
    pthread_mutex_unlock(&mutex_entradas_swap);
}

void asignarFrameALaPagina(t_entradaSwap* unaPaginaSwap){
    char* contenidoADevolverAMP = malloc(TAM_PAGINA);
        if (!contenidoADevolverAMP) {
            log_error(mem_logger, "No se pudo reservar memoria para contenidoADevolverAMP");
            return;
        }
        
    // memcpy(contenidoADevolverAMP,swapFileMapeado + unaPaginaSwap->base*TAM_PAGINA, TAM_PAGINA); //TODO REVISAR
    memcpy(contenidoADevolverAMP,swapFileMapeado + unaPaginaSwap->base, TAM_PAGINA); //TODO REVISAR
    buscarFrameLibreParaProcesoDessuspendido(contenidoADevolverAMP, unaPaginaSwap->pid);
    unaPaginaSwap->ocupado = 0;
    unaPaginaSwap->base = -1;
    unaPaginaSwap->limite = -1;
    unaPaginaSwap->pid = -1;
 
}

void buscarFrameLibreParaProcesoDessuspendido(char* contenidoADevolverAMP, int pid){
    int numFrameLibre = asignarNumDePrimerFrameLIbre();
    if(numFrameLibre != -1){
        t_proceso* proceso = buscar_proceso(pid);
        bool sePudoAsignarFrameAPag = buscarPrimeraPagLibre(tabla_pag_nivel1, numFrameLibre, proceso); //aca s ele vuelven a asignar nuevas paginas al proceso
        int cantPags = list_size(proceso->lista_pag_del_proceso);
    
        if(sePudoAsignarFrameAPag){
            log_debug(mem_logger, "Se volvieron a asignar las %d paginas  para el PID: %d que vuelve de Swap", cantPags,pid);
        }else{
            log_error(mem_logger, "Ocurrio un error al asignar páginas para el proceso que vuelve de Swap");
        }
    }else{
        log_error(mem_logger, "NO se encontró ningun frame libre");
        return;
    }
    return;
   
}

bool buscarPrimeraPagLibre(t_tabla* tablaActual, int numFrameLibre, t_proceso* proceso){
    if (tablaActual->nivel == CANT_NIVELES) {
        for(int i=0;i<list_size(tablaActual->lista_paginas);i++){
            t_pagina* unaPagina = list_get(tablaActual->lista_paginas, i);
            if(unaPagina->bit_ocupado == 0){
                unaPagina->bit_ocupado = 1;
                unaPagina->num_frame = numFrameLibre;
                unaPagina->frame.base = numFrameLibre*TAM_PAGINA;
                unaPagina->frame.limite = (numFrameLibre*TAM_PAGINA + TAM_PAGINA)-1;
                unaPagina->num_pag_local_pid = list_size( proceso->lista_pag_del_proceso);
                list_add(proceso->lista_pag_del_proceso,unaPagina);
                log_info(mem_logger, "Se asigno la pág golbal n° %d - pág local %d con el marco %d para el PID %d",
                    unaPagina->num_pagina,unaPagina->num_pag_local_pid,unaPagina->num_frame, proceso->pid_proceso);
                return true;
            }
        }
        
    } 
    else{
        for (int i = 0; i < list_size(tablaActual->lista_paginas); i++) {
            t_tabla* subtabla = list_get(tablaActual->lista_paginas, i);
            if (buscarPrimeraPagLibre(subtabla,numFrameLibre,proceso)) {
                return true; // cortar recursión si ya la encontró
            }
        }
    }
    return false;   
}

char* buscarContenidoDePagina(t_pagina* unaPagina){
    char* valorLeido = malloc(TAM_PAGINA);
    if (!valorLeido) {
        log_error(mem_logger, "No se pudo reservar memoria para buscarContenidoDePagina");
        return NULL;
    }
    if (unaPagina->frame.base < 0) {
        log_error(mem_logger, "Frame base inválido para la página porque es negativo");
        free(valorLeido);
        return NULL;
    }
    memcpy(valorLeido, (char*)memoria_usuario+unaPagina->frame.base,TAM_PAGINA);
    valorLeido[TAM_PAGINA-1] = '\0';
    log_debug(mem_logger, "CONTENIDO: %s ",valorLeido);
    return valorLeido;
}

void rellenarCamposDeEntradasASwap(){
    // int cantPaginas = TAM_MEMORIA / TAM_PAGINA; // TODO CAPAZ DEBA CAMBIARLO
    // double cant = pow((ENTRADAS_POR_TABLA * 3), CANT_NIVELES);// TODO CAPAZ DEBA CAMBIARLO
    int cantPaginas = TAM_SWAP/ TAM_PAGINA; 

    for(int i = 0; i < cantPaginas; i++){
        t_entradaSwap* unaEntrada = malloc(sizeof(t_entradaSwap));
        unaEntrada->base = -1;
        unaEntrada->limite = -1;
        unaEntrada->numPag = i; // empiezo desde 0
        unaEntrada->pid = -1;
        unaEntrada->ocupado = 0;
        pthread_mutex_lock(&mutex_entradas_swap);
        list_add(entradasSwap,unaEntrada);
        pthread_mutex_unlock(&mutex_entradas_swap);
    }
}

int  seteoPaginaOcupadaEnSwap(int num_pagina, int pid){
    for(int i = 0; i<list_size(entradasSwap);i++){
        t_entradaSwap* unaEntrada = list_get(entradasSwap, i);
        if(unaEntrada->ocupado == 0){
            log_debug(mem_logger,"pasa mutex de seteoPaginaOcupada");
            // unaEntrada->base = num_pagina*TAM_PAGINA;
            unaEntrada->base = i *TAM_PAGINA;
            unaEntrada->limite = unaEntrada->base + TAM_PAGINA - 1;
            unaEntrada->pid = pid;
            unaEntrada->ocupado = 1;
            log_debug(mem_logger, "se pone la pag %d del PID %d en la entrada de Swap %d",num_pagina,pid,i);
            return unaEntrada->base;
        }
    }
    log_error(mem_logger, "No se encontro ninguna entrada disponible en Swap");
    return -1;
 
}

//se copia cada PALABRA que hay en una sola pagina -> por cada pagina asignada al proceso -> bueno en realidad copia 64 bytes

void generarDumpMemory(t_proceso* procesoDump){
    char* timestamp = temporal_get_string_time("%H:%M:%S:%MS");
    char nombreArchivoDump[100];
    sprintf(nombreArchivoDump, "<PID %d>-<%s>.dmp", procesoDump->pid_proceso, timestamp);
    char nombreArchivoDumpCompleto[150];
    sprintf(nombreArchivoDumpCompleto,"/home/utnso/dump_files/%s",nombreArchivoDump);
    free(timestamp);

    int fdDump= truncarArchivo(procesoDump->tamanio_proceso,nombreArchivoDumpCompleto);
    void* dumpMapeado = mapearArchivoDump(fdDump,procesoDump->tamanio_proceso);
    
    // Limpio toda la región de memoria mapeada
    memset(dumpMapeado, 0, procesoDump->tamanio_proceso);

    int offset = 0;

    for(int i=0; i<list_size(procesoDump->lista_pag_del_proceso);i++){
        t_pagina* pag = list_get(procesoDump->lista_pag_del_proceso,i);
        memcpy(dumpMapeado + offset,memoria_usuario + pag->frame.base, TAM_PAGINA);
        offset += TAM_PAGINA;
    }

    msync(dumpMapeado, procesoDump->tamanio_proceso, MS_SYNC);
    munmap(dumpMapeado, procesoDump->tamanio_proceso);
    close(fdDump);
}

void* mapearArchivoDump(int fdArchivoDump, int tamanioProceso){
   
    // mapear el archivo a memoria
    void* archivoDumpMapeado = mmap(NULL, (size_t)tamanioProceso, PROT_READ | PROT_WRITE, MAP_SHARED, fdArchivoDump, 0);
    if (archivoDumpMapeado == MAP_FAILED) {
            perror("mmap");
            log_error(mem_logger, "ocurrio un erro al mapear el archivo dump del proceso");
            close(fdArchivoDump); 
            exit(1);
    }
    return archivoDumpMapeado;
    
}

int truncarArchivo(int tamanioProceso, char* path){
    int fdArchivoDump = open(path, O_RDWR | O_CREAT, 0644);
    if (fdArchivoDump == -1) {
        perror("Error al crear  el fd del archivo");
        return 0;
    }

    // Setea el tamaño del archivo
    if (ftruncate(fdArchivoDump, tamanioProceso) == -1) {
        perror("Error al cambiar el tamaño del archivo");
        close(fdArchivoDump);
        return 0 ;
    }

    return fdArchivoDump;
}

void chequearQueNoSeaNUll(void* unPuntero){
    if(unPuntero == NULL){
        return;
    }
}

int buscarTablaSigNivelparaEStabilidad(int pid_procesoTB,int entrada, int nivel){
    t_proceso* procesoQueBuscaNumTabla = buscar_proceso(pid_procesoTB);
        int* numEntrada = malloc(sizeof(int));
        *numEntrada = entrada;
        queue_push(procesoQueBuscaNumTabla->colaDECaminoNumTP,numEntrada);// 0 0 

        return 1; //por default pero no significa nada  
}

int buscarMarcoParaEstabilidad(int pid, int ultimaEntrada, t_proceso* procesoQueBuscaMarco){
 
    int* valorE1Ptr = queue_pop(procesoQueBuscaMarco->colaDECaminoNumTP);
    int valor1 =  *valorE1Ptr;
    free(valorE1Ptr);
    int* valorE2Ptr= queue_pop(procesoQueBuscaMarco->colaDECaminoNumTP);
    int valorE2 = *valorE2Ptr;
    free(valorE2Ptr);
        if(valor1 == 0 && valorE2 == 0){
            for(int i = 0; i<list_size(procesoQueBuscaMarco->lista_pag_del_proceso);i++){
                t_pagina* pag = list_get(procesoQueBuscaMarco->lista_pag_del_proceso,i);
                if(pag->num_pag_local_pid == ultimaEntrada){
                    log_debug(mem_logger, "Se le envia a memoria el MARCO %d del PID %d",pag->num_frame,procesoQueBuscaMarco->pid_proceso);
                    return pag->num_frame;
                    break;
                }
            }
        }
        if(valor1 == 0 && valorE2 == 1){
            int i = 0;
            if(ultimaEntrada == 0){
                i = 4;
            }
            if(ultimaEntrada == 1){
                i = 5;
            }
            if(ultimaEntrada == 2){
                i = 6;
            }
            if(ultimaEntrada == 3){
                i = 7;
            }
           
            t_pagina* pag = list_get(procesoQueBuscaMarco->lista_pag_del_proceso,i);
            log_debug(mem_logger, "Se le envia a memoria el MARCO %d del PID %d",pag->num_frame,procesoQueBuscaMarco->pid_proceso);
            return pag->num_frame;    
        }

        log_error(mem_logger, "Algo salió mal al buscar el marco para el pid %d", pid);
        return -1;
}

//SEÑALES
void manejar_sigint(int senial){
    printf("\n[Memoria] Señal SIGINT recibida. Terminando...\n");
    apagar_memoria = 1;
    liberar_recursos();
    exit(0); // Salida limpia
}

void liberar_recursos() {
    printf("\nLiberando recursos...\n");

    if (fd_server != -1) {
        close(fd_server);
        printf("Socket de escucha Memoria cerrado.\n");
    }


    for (int i = 0; i < list_size(sockets_de_cpus); i++) {
        int* socketCpu = list_get(sockets_de_cpus, i);
        if((*socketCpu)!= -1){
            close(*socketCpu);
           
        }
        free(socketCpu);
    }

    list_destroy(sockets_de_cpus);

    // Liberar memoria dinámica, mutexes, hilos, logs, etc.
    if (memoria_usuario != NULL) {
        free(memoria_usuario);
        printf("Memoria de usuario liberada.\n");
    }
    if (swapFileMapeado != NULL) {
        munmap(swapFileMapeado, TAM_MEMORIA);
        printf("Memoria de Swap liberada.\n");
    }
    if (bitarray_frames != NULL) {
        bitarray_destroy(bitarray_frames);
        printf("Bitarray de frames liberado.\n");
    }
    if(mem_logger != NULL){
        log_destroy(mem_logger);
        printf("Logger de Memoria liberado.\n");
    }

    if(mem_config != NULL){
        config_destroy(mem_config);
        printf("Configuración de Memoria liberada.\n");

    }
    if (tabla_pag_nivel1 != NULL) {
        destruirTablasPaginas(tabla_pag_nivel1);
    }
    if (entradasSwap != NULL) {
      list_destroy(entradasSwap);
    }
    if(lista_procesos != NULL){
        list_destroy_and_destroy_elements(lista_procesos, destruirProcesos);    
    }
}

void destruirTpaginas(void* pagina) {
    t_pagina* tablaPag = (t_pagina*)pagina;
    if (tablaPag != NULL){
        free(tablaPag);
    } 
}

void destruirTablasPaginas(void* tabla) {
    t_tabla* tablaPag = (t_tabla*)tabla;
    

        if(tablaPag->nivel == CANT_NIVELES){ 
            list_clean_and_destroy_elements(tablaPag->lista_paginas, (void*)destruirTpaginas);
            if(tablaPag != NULL){
                free(tablaPag);
                return;//termina recursividad
            }
        }else{
            for(int i = 0; i < list_size(tablaPag->lista_paginas); i++) {
                t_tabla* subTabla = list_get(tablaPag->lista_paginas, i);
                destruirTablasPaginas(subTabla);
            }
        }
     
}
void eliminarElemColas(void* entrada){
    if(entrada == NULL){
        return;
    }
    int* entradaAux = (int*)entrada;
    free(entradaAux);
}

void destruir_instruccion(void* unaInstruccion){
    t_instruccion* instruccion = (t_instruccion*)unaInstruccion;

    if(instruccion->operacion != NULL){
        free(instruccion->operacion);
    }
    if(instruccion->parametros != NULL){
        if(instruccion->parametros->primerParamNum != NULL){
            free(instruccion->parametros->primerParamNum);
        }
        if(instruccion->parametros->segundoParamNum != NULL){
            free(instruccion->parametros->segundoParamNum);

        }
        if(instruccion->parametros->primerParamString!= NULL){
            free(instruccion->parametros->primerParamString);
        }
        if (instruccion->parametros->segundoParamString != NULL) {
            free(instruccion->parametros->segundoParamString);

        } 
        
    }
    if(instruccion != NULL){
        free(instruccion);
    }
    
}

void destruirProcesos(void* unProceso){
    t_proceso* proceso = (t_proceso*)unProceso;

    if(proceso->colaDECaminoNumTP != NULL){
        queue_destroy_and_destroy_elements(proceso->colaDECaminoNumTP, eliminarElemColas);
    }
    if(proceso->colaDeCaminoAPag != NULL){
        queue_destroy_and_destroy_elements(proceso->colaDeCaminoAPag, eliminarElemColas);
    }
    if(proceso->lista_pag_del_proceso != NULL){
        list_destroy(proceso->lista_pag_del_proceso);
    }
    if(proceso->lista_instrucciones_proceso != NULL){
        list_destroy_and_destroy_elements(proceso->lista_instrucciones_proceso, destruir_instruccion);
    }
    
    free(proceso);
}