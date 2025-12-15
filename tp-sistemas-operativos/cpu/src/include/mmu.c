#include "mmu.h"

char* valor_leido_memoria;

t_list* cache;
t_list* tlb;
int direccion_fisica = -1;

// Luego en una función:
void inicializar_estructuras() {
    cache = list_create();
    tlb = list_create();
}
entrada_cache* entrada_cache_actual = NULL;
entrada_tlb * entrada_tlb_actual = NULL;

int puntero_clock = 0;  
int puntero_clock_m = 0;
int CANTIDAD_NIVELES = 0;
int ENTRADAS_POR_TABLA = 0;
int TAM_PAGINA = 0;

int nro_marco = -1; //incializo como -1 para que no de error al usarlo en la MMU

int traducir_direccion_logica(int pid, t_instruccion* instruccion) {
    

    int nro_pagina = floor(instruccion->args->direccion_logica / TAM_PAGINA);

    log_debug(cpu_logger, "nro_pagina obtenido por la formula: %d", nro_pagina);
    
    int desplazamiento = instruccion->args->direccion_logica % TAM_PAGINA;

    log_debug(cpu_logger, "Desplazamiento obtenido por la formula: %d", desplazamiento);

    // CACHE
    if (ENTRADAS_CACHE > 0 && list_size(cache) > 0){
        if (verificar_cache(nro_pagina)){ 

            log_info(cpu_logger, "## PID : <%d> - Cache HIT - Página %d", pid, nro_pagina);

                if(strcmp(instruccion->operacion,"WRITE") == 0){
                    log_info(cpu_logger, "## Escribiendo en Cache - Página %d", nro_pagina);
                    escribirEnCache(instruccion->args->datos, nro_pagina,desplazamiento);
                    if(entrada_cache_actual != NULL){
                        entrada_cache_actual->bit_uso = true;
                        entrada_cache_actual->bit_modificado = true;
                    }else{
                        log_debug(cpu_logger,"## La entrada_cache_actual esta en NULL");
                    }
                   
                    return -1; // indica que se escribió en cache, no se necesita direccion fisica

                }else if(strcmp(instruccion->operacion,"READ") == 0){
                    // log_info(cpu_logger, "Leyendo de cache - Página %d", nro_pagina);
                    entrada_cache_actual->bit_uso = true;
                    valor_leido_memoria =  leerEnCache(instruccion->args->tamanio, nro_pagina,desplazamiento);
                    return -1; // indica que se leyó de cache, no se necesita direccion fisica
                }
                    
        }      
        else {
            log_info(cpu_logger, "## PID : <%d> - Cache MISS - Página %d", pid, nro_pagina);
        }
    }else{
        if(ENTRADAS_CACHE > 0){
            log_info(cpu_logger, "## PID : <%d> - Cache MISS - Página %d", pid, nro_pagina);
        }
    }

    // TLB
    if (ENTRADAS_TLB > 0 && list_size(tlb) > 0){
        if (verificar_tlb( nro_pagina)){
            log_info(cpu_logger, "## PID : <%d> - TLB HIT - Página %d", pid, nro_pagina);

            // Si hubo HIT y el algoritmo de TLB es LRU entonces esa pagina se referencio recientemente
            if(strcmp(REEMPLAZO_TLB,"LRU")==0){
                entrada_tlb_actual->timestamp = get_timestamp();
            }

            direccion_fisica = entrada_tlb_actual->nro_marco * TAM_PAGINA + desplazamiento; // se va a guardar en cada entrada de la cache

                if(ENTRADAS_CACHE>0){
                    int confirmacion = procedimientoParaCacheHabilitada(instruccion->args->direccion_logica,direccion_fisica, nro_pagina, desplazamiento);
                    return confirmacion;
                }
                else{
                    if(strcmp(instruccion->operacion, "READ") == 0){
                        valor_leido_memoria = pedir_valor_a_memoria(direccion_fisica, instruccion->args->tamanio, proceso->pid, nro_pagina);
                        return direccion_fisica; // devuelve la direccion fisica porque se leyó de memoria
            
                    }else{
                        escribir_memoria(proceso->pid, direccion_fisica, instruccion->args->datos);
                        return direccion_fisica; // devuelve la direccion fisica porque se escribio en memoria
                    }
                }

        }     
        else{
            log_info(cpu_logger, "TLB MISS - Página %d", nro_pagina);
            direccion_fisica = procedimientoParaObtenerMarcoDeMemoria(nro_pagina, desplazamiento,proceso->pid); //internamente me actualiza la tlb

                if(ENTRADAS_CACHE >0){
                    int confirmacion = procedimientoParaCacheHabilitada(instruccion->args->direccion_logica,direccion_fisica, nro_pagina, desplazamiento);
                    return confirmacion;
                }
                else{
                    if(strcmp(instruccion->operacion, "READ") == 0){
                        valor_leido_memoria = pedir_valor_a_memoria(direccion_fisica, instruccion->args->tamanio, proceso->pid, nro_pagina);
                        return direccion_fisica; // devuelve la direccion fisica porque se leyó de memoria
            
                    }else{
                        escribir_memoria(proceso->pid, direccion_fisica, instruccion->args->datos);
                        return direccion_fisica; // devuelve la direccion fisica porque se escribio en memoria
                    }
                }
            
        }
                      
    }else{
        if(ENTRADAS_TLB > 0){
            log_info(cpu_logger, "TLB MISS - Página %d", nro_pagina);
        }
    }

    // Se solicita a memoria (Hay solo TLB o CACHE)

    if((ENTRADAS_TLB == 0 && ENTRADAS_CACHE > 0 )|| (ENTRADAS_CACHE == 0 && ENTRADAS_TLB > 0)){
        direccion_fisica = procedimientoParaObtenerMarcoDeMemoria(nro_pagina,desplazamiento,proceso->pid);

        if(ENTRADAS_CACHE > 0){
            int confirmacionCache = procedimientoParaCacheHabilitada(instruccion->args->direccion_logica,direccion_fisica,nro_pagina,desplazamiento);
            return confirmacionCache;
        }else{
            if(strcmp(instruccion->operacion, "READ") == 0){
                valor_leido_memoria = pedir_valor_a_memoria(direccion_fisica, instruccion->args->tamanio, proceso->pid, nro_pagina);
                return direccion_fisica;
               }else{
                   escribir_memoria(proceso->pid, direccion_fisica, instruccion->args->datos);
                   return direccion_fisica;
            }
        }
       
    }

    // Se solocita a Memoria (hay TLB y CACHE)

    if((ENTRADAS_TLB > 0 && ENTRADAS_CACHE > 0)){

        direccion_fisica = procedimientoParaObtenerMarcoDeMemoria(nro_pagina,desplazamiento,proceso->pid);

        if(ENTRADAS_CACHE > 0){
            int confirmacionCache = procedimientoParaCacheHabilitada(instruccion->args->direccion_logica,direccion_fisica,nro_pagina,desplazamiento);
            return confirmacionCache;
        }else{
            if(strcmp(instruccion->operacion, "READ") == 0){
                valor_leido_memoria = pedir_valor_a_memoria(direccion_fisica, instruccion->args->tamanio, proceso->pid, nro_pagina);
                return direccion_fisica;
            }else{
                escribir_memoria(proceso->pid, direccion_fisica, instruccion->args->datos);
                return direccion_fisica;
            }
        }
    }

    if(ENTRADAS_TLB == 0 && ENTRADAS_CACHE == 0){
        int direccion_fisica = procedimientoParaObtenerMarcoDeMemoria(nro_pagina,desplazamiento,proceso->pid);

        
        if(strcmp(instruccion->operacion, "READ") == 0){
            valor_leido_memoria = pedir_valor_a_memoria(direccion_fisica, instruccion->args->tamanio, proceso->pid, nro_pagina);
            return direccion_fisica;
        }else{
           escribir_memoria(proceso->pid, direccion_fisica, instruccion->args->datos);
           return direccion_fisica;
        }
       
    }

    return -2;
          
}

  
int procedimientoParaObtenerMarcoDeMemoria(int nro_pagina, int desplazamiento, int pid){
     // MMU -> Accede a tablas de paginas en Memoria
     nro_marco = obtener_marco_mmu(pid, nro_pagina);

     log_info(cpu_logger, "EL MARCO QUE ME ENVIA MEM ES %d para el PID %d",nro_marco, pid);
     direccion_fisica = nro_marco * TAM_PAGINA + desplazamiento;

    log_info(cpu_logger, "Direccion fisica %d del PID %d",direccion_fisica,pid);
     if (ENTRADAS_TLB > 0) {
        actualizar_tlb(nro_pagina, nro_marco);
     }


     return direccion_fisica;
    
}



bool verificar_cache(int nro_pagina) {
    for (int i = 0; i < list_size(cache); i++) {
        entrada_cache* entrada = list_get(cache, i);
        if (entrada->nro_pagina == nro_pagina) {
            entrada_cache_actual = entrada;  // Asignación válida dentro de una función
            log_debug(cpu_logger, "puntero de la entrada de cache actual %p",entrada_cache_actual);
            return true;
        }
    }
    return false;
}


bool verificar_tlb(int nro_pagina){
    for (int i = 0; i < list_size(tlb); i++) {
        entrada_tlb* entrada = list_get(tlb, i);
        if (entrada->nro_pagina == nro_pagina) {
            entrada_tlb_actual = entrada;  // se guarda globalmente
            return true;
        }
    }
    return false;

}


int obtener_marco_mmu(int pid, int nro_pagina) {
    int tabla_actual = 1; 
    int entrada = -1;
    
    // Recorrer niveles 1 hasta N-1
    for(int nivel = 1; nivel < CANTIDAD_NIVELES; nivel++){
        int divisor = pow(ENTRADAS_POR_TABLA, CANTIDAD_NIVELES - nivel);
        entrada = (nro_pagina / divisor) % ENTRADAS_POR_TABLA;
        // función que pide la tabla del siguiente nivel

        log_debug(cpu_logger, "PID: %d - OBTENER TABLA - Nivel: %d - Entrada: %d", pid, nivel, entrada);
        tabla_actual = pedir_tabla_nivel(pid, entrada, nivel);
        log_debug(cpu_logger, "Tabla actual recibida de memoria : %d", tabla_actual);
    }

    // Último nivel: obtenés el marco
    int entrada_final = nro_pagina % ENTRADAS_POR_TABLA; //  = pow(ENTRADAS_POR_TABLA, nivel - nivel);
    log_debug(cpu_logger, "entrada final: %d ",entrada_final);
    int nro_marco_retorno = pedir_marco(pid, tabla_actual, entrada_final);

    log_info(cpu_logger, "PID: %d - OBTENER MARCO - Página: %d - Marco: %d", pid, nro_pagina, nro_marco_retorno);

    return nro_marco_retorno;
}


void escribir_memoria(int pid, int direccion_fisica, char* datos) {
    t_buffer* buffer = crear_buffer();
    cargar_entero_al_buffer(buffer, pid);
    cargar_entero_al_buffer(buffer, direccion_fisica);
    cargar_string_al_buffer(buffer, datos);  // cargás los datos a escribir

    t_paquete* paquete = crear_paquete(ESCRIBIR_VALOR, buffer);
    enviar_paquete(paquete, fd_conexion_memoria);
    eliminar_paquete(paquete);

op_code cod_op = recibir_operacion(fd_conexion_memoria);

if (cod_op == ESCRIBIR_VALOR_RTA) {
    log_debug(cpu_logger, "se recibe el cod %d ", cod_op);
    t_buffer* buffer = recibir_todo_un_buffer(fd_conexion_memoria);
    int confirmacion = extraer_int_del_buffer(buffer);
    destruir_buffer(buffer);

    if (confirmacion != 1) {
        log_error(cpu_logger, "Error: Memoria no confirmó la escritura.");
    } else if (confirmacion == 1) {
        log_info(cpu_logger, "Confirmación recibida: Escritura exitosa.");
    }
} else {
    log_error(cpu_logger, "Error: Operación inesperada desde Memoria %d ", cod_op);
}


}



char* pedir_valor_a_memoria(int direccion_fisica, int tamanio, int pid, int numPag){
    // Crear buffer y cargar dirección física y tamaño
    t_buffer* buffer = crear_buffer();
    cargar_entero_al_buffer(buffer, pid);
    cargar_entero_al_buffer(buffer, numPag);
    cargar_entero_al_buffer(buffer, direccion_fisica);
    cargar_entero_al_buffer(buffer, tamanio);

    // Crear paquete con el código de operación (asumimos OP_LEER_MEMORIA, definilo si no existe)
    t_paquete* paquete = crear_paquete(SOLICITAR_VALOR, buffer);

    // Enviar el paquete a memoria
    enviar_paquete(paquete, fd_conexion_memoria);

    // Limpiar memoria del paquete enviado
    eliminar_paquete(paquete);

    // Esperar respuesta de memoria (asumimos que es un string de tamaño `tamanio`)
    char* valor;
    op_code cod_op = recibir_operacion(fd_conexion_memoria);
    
    // if (cod_op == SOLICITAR_VALOR_RTA) {
    //     if (recv(fd_conexion_memoria, valor, tamanio, MSG_WAITALL) <= 0) {
    //         log_error(cpu_logger, "Error al recibir datos desde memoria");
    //         free(valor);
    //         return NULL;
    //     }
    // }
   
    // valor[tamanio] = '\0';  // asegurarse que es string null-terminated

    if(cod_op == SOLICITAR_VALOR_RTA){
        t_buffer* bufferValor = recibir_todo_un_buffer(fd_conexion_memoria);
        valor = extraer_string_del_buffer(bufferValor);
        destruir_buffer(bufferValor);
    } else {
        log_error(cpu_logger, "Error: Operación inesperada desde Memoria %d ", cod_op);
        return NULL;
    }
    return valor;
}


int pedir_marco(int pid, int tabla, int entrada) {
    t_buffer* buffer = crear_buffer();
    cargar_entero_al_buffer(buffer, pid);
    cargar_entero_al_buffer(buffer, tabla);
    cargar_entero_al_buffer(buffer, entrada);

    t_paquete* paquete = crear_paquete(PEDIR_MARCO, buffer);
    enviar_paquete(paquete, fd_conexion_memoria);
    eliminar_paquete(paquete);

    int marco = -1; // Inicializar marco como -1 para manejar errores
    op_code cod_op = recibir_operacion(fd_conexion_memoria);
    if (cod_op == PEDIR_MARCO_RTA) {
       t_buffer* bufferMarco = recibir_todo_un_buffer(fd_conexion_memoria);
       marco = extraer_int_del_buffer(bufferMarco);
        destruir_buffer(bufferMarco);     
    }
    
    return marco;
}


int pedir_tabla_nivel(int pid,int entrada, int nivel) {
    t_buffer* buffer = crear_buffer();
    cargar_entero_al_buffer(buffer, pid);
    cargar_entero_al_buffer(buffer, entrada);
    cargar_entero_al_buffer(buffer, nivel); // 1 2 3

    t_paquete* paquete = crear_paquete(PEDIR_TABLA_NIVEL, buffer);
    enviar_paquete(paquete, fd_conexion_memoria);
    eliminar_paquete(paquete);

    // Recibir respuesta: el número de la tabla del siguiente nivel
    int  tabla_siguiente = -1;

    op_code cod_op = recibir_operacion(fd_conexion_memoria);
    if (cod_op == PEDIR_TABLA_NIVEL_RTA) {
        t_buffer* bufferTablaPag = recibir_todo_un_buffer(fd_conexion_memoria);
        int pid = extraer_int_del_buffer(bufferTablaPag);
        tabla_siguiente = extraer_int_del_buffer(bufferTablaPag);
        log_debug(cpu_logger, "Tabla del siguiente nivel: %d para el PID: %d", tabla_siguiente, pid);
        destruir_buffer(bufferTablaPag);
    }  
    return tabla_siguiente;
}



void actualizar_cache(int nro_pagina, int direccion_logica, int direccion_fisica, char* valor) {
    entrada_cache* nueva_entrada = malloc(sizeof(entrada_cache));
    nueva_entrada->pid_entrada = proceso->pid; // Asignar el PID del proceso actual para que no rompa al enviar direcciones fisicas a cache
    nueva_entrada->nro_pagina = nro_pagina;
    nueva_entrada->direccion_logica = direccion_logica;
    nueva_entrada->direccion_fisica = direccion_fisica;// la guardo por las dudas
    nueva_entrada->valor = NULL;
    nueva_entrada->bit_uso = true;
    nueva_entrada->bit_modificado = false;
    nueva_entrada->offset = 0; // Asignar el offset 
    if(nueva_entrada->valor == NULL){
        nueva_entrada->valor = malloc(TAM_PAGINA);
        memset(nueva_entrada->valor, 0, TAM_PAGINA); // Inicializar el valor a cero
        escribirPaginaCompletaEnCache(valor, nro_pagina,nueva_entrada);
       
    }
    if (list_size(cache) < ENTRADAS_CACHE) {
        list_add(cache, nueva_entrada);
        log_debug(cpu_logger,"Iniciacion de cache. Se agrego en la entrada %d la pagina %d",list_size(cache),nueva_entrada->nro_pagina);
    } else {
        if (strcmp(REEMPLAZO_CACHE, "CLOCK") == 0) {
            reemplazo_cache_clock(nueva_entrada); // ya actualiza la pagina en memoria
        } else if (strcmp(REEMPLAZO_CACHE, "CLOCK-M") == 0) {
            reemplazo_cache_clock_m(nueva_entrada); // ya actualiza en memoria
        }
    }
}


void actualizar_tlb(int nro_pagina,int nro_marco){
    entrada_tlb* nueva_entrada = malloc(sizeof(entrada_tlb));
    nueva_entrada->nro_pagina = nro_pagina;
    nueva_entrada->nro_marco = nro_marco;
    nueva_entrada->timestamp = get_timestamp();
    

    if (list_size(tlb) < ENTRADAS_TLB){
        list_add(tlb, nueva_entrada);
        log_debug(cpu_logger,"Se agrego en la entrada %d el marco %d", list_size(tlb),nueva_entrada->nro_marco);

    }else{       
        
        if(strcmp(REEMPLAZO_TLB, "FIFO") == 0){
            reemplazo_tlb_fifo(nueva_entrada);
        }else if (strcmp(REEMPLAZO_TLB, "LRU") == 0){
            reemplazo_tlb_lru(nueva_entrada);
        }

        // list_add(tlb, nueva_entrada);
        // log_debug(cpu_logger,"Se agrego en la entrada %d el marco %d", list_size(tlb),nueva_entrada->nro_marco);
    }
}



void reemplazo_tlb_fifo(entrada_tlb* nueva_entrada) {
    // list_remove_and_destroy_element(tlb, 0, free); Si se reemplaza siempre la primera entrada entonces esta bien

    // Implementacion reemplazando la pagina con más tiempo en la tlb
    
    entrada_tlb* victima = list_get(tlb, 0);
    int index = 0;

    for (int i = 1; i < list_size(tlb); i++) {
        entrada_tlb* actual = list_get(tlb, i);
        if (actual->timestamp < victima->timestamp) {
            victima = actual;
            index = i;
        }
    }
    
    log_debug(cpu_logger,"Se va a reemplazar la entrada %d con pagina %d y marco %d",index+1, victima->nro_pagina ,victima->nro_marco);
    list_replace_and_destroy_element(tlb,index,nueva_entrada,free);
}

double calcularTiempo(entrada_tlb* entrada){
    uint64_t tiempo_inicio = entrada->timestamp;
    uint64_t tiempo_fin = get_timestamp();

    double tiempo = tiempo_fin-tiempo_inicio;
    return tiempo;
}

uint64_t get_timestamp() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

void reemplazo_tlb_lru(entrada_tlb* entrada_nueva) {
    entrada_tlb* victima = list_get(tlb, 0);
    int index = 0;

    for (int i = 1; i < list_size(tlb); i++) {
        entrada_tlb* actual = list_get(tlb, i);
        if (actual->timestamp < victima->timestamp) {
            victima = actual;
            index = i;
        }
    }

    log_debug(cpu_logger,"Se va a reemplazar la entrada %d donde estaba pagina %d y marco %d",index+1, victima->nro_pagina ,victima->nro_marco);
    list_replace_and_destroy_element(tlb, index,entrada_nueva, free);
}



void reemplazo_cache_clock(entrada_cache* nueva_entrada) {
    int size = list_size(cache);
    int vueltas = 0;

    while (vueltas < 3) {
      
            for (int i = 0; i < size; i++) {
                entrada_cache* entrada = list_get(cache, puntero_clock);

                if (vueltas < 3 && !entrada->bit_uso) {
                
                    log_debug(cpu_logger, "Reemplazo de cache - Página: %d - Entrada %d", entrada->nro_pagina, (puntero_clock)+1);
                        // if(entrada->bit_modificado){
                            log_debug(cpu_logger, "Página modificada. Se va a enviar a Memoria la Página: %d valor %s :", entrada->nro_pagina, entrada->valor);
            
                            int nro_pagina_a_reemplazar = floor(entrada->direccion_logica / TAM_PAGINA);
                            int desplazamientoDePaginaAReemplazar= entrada->direccion_logica % TAM_PAGINA;
            
                            if (ENTRADAS_TLB > 0 && list_size(tlb) > 0){
                                    if (verificar_tlb(nro_pagina_a_reemplazar)) {
                                        log_info(cpu_logger, "## PID : <%d> - TLB HIT - Página %d", proceso->pid, nro_pagina_a_reemplazar);
                                        direccion_fisica = entrada_tlb_actual->nro_marco * TAM_PAGINA + desplazamientoDePaginaAReemplazar;
                                        enviar_valor_a_memoria(direccion_fisica, entrada->valor, entrada->pid_entrada); // enviar valor a memoria es ACTUALIZAR_PAGINA_COMPLETA
                                        free(entrada->valor);
                                        list_replace_and_destroy_element(cache, puntero_clock, nueva_entrada, free); // chequear
                                        log_debug(cpu_logger,"Se agrego en la entrada %d la pagina %d",puntero_clock,nueva_entrada->nro_pagina);
                                        puntero_clock = (puntero_clock + 1) % size;
                                        return;
                                    
                                    }else {
                                        log_info(cpu_logger, "## PID : <%d> - TLB MISS - Página %d", proceso->pid, nro_pagina_a_reemplazar);
                                    }
                            }
                            nro_marco = obtener_marco_mmu(proceso->pid, nro_pagina_a_reemplazar);
                            direccion_fisica = nro_marco * TAM_PAGINA + desplazamientoDePaginaAReemplazar;
                            enviar_valor_a_memoria(direccion_fisica, entrada->valor,entrada->pid_entrada);
                            free(entrada->valor);
                            list_replace_and_destroy_element(cache, puntero_clock, nueva_entrada, free); // chequear
                            log_debug(cpu_logger,"Se agrego en la entrada %d la pagina %d",(puntero_clock)+1,nueva_entrada->nro_pagina);
                            puntero_clock = (puntero_clock + 1) % size;
                            return;
                        // }
                }
        
                if (vueltas == 1 && entrada->bit_uso) {
                    entrada->bit_uso = false;
                }
                puntero_clock = (puntero_clock + 1) % size;
            }

        vueltas++;
    }
}




void reemplazo_cache_clock_m(entrada_cache* nueva_entrada) {
    int size = list_size(cache);
    int vueltas = 0;

    while (vueltas < 4) {
        for (int i = 0; i < size; i++) {
            entrada_cache* entrada = list_get(cache, puntero_clock_m);
            log_debug(cpu_logger, "Se va a chequear entrada %d PAG %d ", puntero_clock_m + 1,entrada->nro_pagina);

            if ((vueltas == 0 || vueltas == 2) && !entrada->bit_uso && !entrada->bit_modificado){
                    log_debug(cpu_logger, "Reemplazo de cache de página sin modificar - Página: %d Entrada: %d", entrada->nro_pagina, puntero_clock_m+1);
                    free(entrada->valor);
                    list_replace_and_destroy_element(cache, puntero_clock_m, nueva_entrada, free); // chequear
                    log_debug(cpu_logger,"Se agrego en la entrada %d la pagina %d",(puntero_clock_m)+1,nueva_entrada->nro_pagina);
                    puntero_clock_m = (puntero_clock_m + 1) % size;
                    for(int i = 0; i<list_size(cache);i++){
                        entrada_cache* entrada = list_get(cache,i);
                        log_debug(cpu_logger, "Cache final - Entrada: %d,  PAG: %d",i+1,entrada->nro_pagina);
                    }
                    return;         
            }
   

            if ((vueltas == 1 || vueltas == 3) && !entrada->bit_uso && entrada->bit_modificado) {
                int desplazamiento = entrada->direccion_logica % TAM_PAGINA;
                log_debug(cpu_logger, "Reemplazo de cache de página  modificada - Página: %d Entrada: %d", entrada->nro_pagina, puntero_clock_m+1);

                if (ENTRADAS_TLB > 0 && list_size(tlb) > 0){
                    if (verificar_tlb(entrada->nro_pagina)) {
                        log_info(cpu_logger, "TLB HIT - Página %d", entrada->nro_pagina);
                        direccion_fisica = entrada_tlb_actual->nro_marco * TAM_PAGINA + desplazamiento;
                        enviar_valor_a_memoria(direccion_fisica, entrada->valor, entrada->pid_entrada); // enviar valor a memoria es ACTUALIZAR_PAGINA_COMPLET
                        free(entrada->valor);
                        list_replace_and_destroy_element(cache, puntero_clock_m, nueva_entrada, free); // chequear
                        puntero_clock_m = (puntero_clock_m + 1) % size;
                        return;
                    
                    }else {
                        log_info(cpu_logger, "TLB MISS - Página %d", entrada->nro_pagina);
                    }
                }
                nro_marco = obtener_marco_mmu(proceso->pid, entrada->nro_pagina);
                direccion_fisica = nro_marco * TAM_PAGINA + desplazamiento;
                enviar_valor_a_memoria(direccion_fisica, entrada->valor, entrada->pid_entrada); // enviar valor a memoria es ACTUALIZAR_PAGINA_COMPLETA
                free(entrada->valor);
                list_replace_and_destroy_element(cache, puntero_clock_m, nueva_entrada, free); // chequear
                puntero_clock_m = (puntero_clock_m + 1) % size;
                for(int i = 0; i<list_size(cache);i++){
                    entrada_cache* entrada = list_get(cache,i);
                    log_debug(cpu_logger, "Cache final - Entrada: %d,  PAG: %d",i+1,entrada->nro_pagina);
                }
                return;
                
            }
            if(vueltas==1){
                entrada->bit_uso = false;
            }

            puntero_clock_m = (puntero_clock_m + 1) % size;
        }

        vueltas++;
    }

 
 
    // Fallback si no encuentra
    // list_remove_and_destroy_element(cache, 0, free);
}


void obtener_config_de_memoria(int socket_memoria) {

    t_buffer* bufferConfig = crear_buffer();
    cargar_uint32_al_buffer(bufferConfig, 1); // No se envía información, solo un numero random
    t_paquete* paquete = crear_paquete(SOLICITAR_CONFIG_MEMORIA, bufferConfig);     
    enviar_paquete(paquete, socket_memoria);
    eliminar_paquete(paquete);
    
    int cod_op = recibir_operacion(socket_memoria);
    t_buffer* buffer = NULL;
    if (cod_op == SOLICITAR_CONFIG_MEMORIA_RTA) {
        buffer = recibir_todo_un_buffer(socket_memoria);
        CANTIDAD_NIVELES = extraer_int_del_buffer(buffer);
        ENTRADAS_POR_TABLA = extraer_int_del_buffer(buffer);
        TAM_PAGINA = extraer_int_del_buffer(buffer);
    }else {
        log_error(cpu_logger, "Error al recibir la configuración de memoria");
        return;
    }
    destruir_buffer(buffer);

    log_debug(cpu_logger, "Configuración recibida: Niveles=%d, Entradas=%d, TamPag=%d",CANTIDAD_NIVELES, ENTRADAS_POR_TABLA, TAM_PAGINA);
}


void liberar_cache(t_list* cache) {
    void liberarEntradaCache(void* ptr){
        entrada_cache* unaEntrada = (entrada_cache*) ptr;
        free(unaEntrada->valor);
        free(unaEntrada);

    };
    
    list_clean_and_destroy_elements(cache, liberarEntradaCache);
}


void liberar_tlb(t_list* tlb) {
    void liberarEntradaTLB(void* ptr){
        entrada_tlb* unaEntrada = (entrada_tlb*) ptr;
        free(unaEntrada);

    };
    
    list_clean_and_destroy_elements(tlb, liberarEntradaTLB);
}


void escribirEnCache(char* dato_a_escribir, int numPag, int desplazamiento){

    usleep(RETARDO_CACHE*1000);
    entrada_cache* entrada = buscarEntradaCache(numPag);
    int tamanioTexto = strlen(dato_a_escribir); //no le agrego el /0
    // entrada->offset = desplazamiento; // si yo quiero escribir en determinada posicion del char* solo se lo asigo al offset que al principio es 0

   
    if (entrada != NULL && entrada->valor != NULL){
        memcpy(entrada->valor+desplazamiento, dato_a_escribir, tamanioTexto);
        // entrada->offset += tamanioTexto; // Actualizar el offset, aca no se necesita porque sino la prueba fallaria
    }

    char* textoEscrito = entrada->valor;
    textoEscrito[TAM_PAGINA-1] = '\0';

    // log_info(cpu_logger, "Se acaba de escribir el string :  %s  la cache quedo asi: %s", dato_a_escribir,textoEscrito);
    log_info(cpu_logger, "PID: %d - Acción: ESCRIBIR - Dirección Física: %d - Valor: %s", //LOG OBLIGATORIO AUQNUE ESCRIBA DESDE LA CACHE LO PONGO
        proceso->pid,
        entrada->direccion_fisica,
       dato_a_escribir);   
}

void escribirPaginaCompletaEnCache(char* dato_a_escribir, int numPag,entrada_cache* entrada_a_agregar){
    
    entrada_cache_actual = entrada_a_agregar; 
    if (entrada_a_agregar != NULL && entrada_a_agregar->valor != NULL) {
        memcpy(entrada_a_agregar->valor, dato_a_escribir, TAM_PAGINA);
        dato_a_escribir[TAM_PAGINA]='\0';
        log_debug(cpu_logger,"Contenido en la cache %s de la pagina %d que se acaba de agregar",dato_a_escribir,numPag);
        // entrada->offset += offsetDePagEnMemoria; // Actualizar el offset asi escribi a partir de lo que ya habia en Memoria
    }


}


entrada_cache* buscarEntradaCache(int numPag){
    for (int i = 0; i < list_size(cache); i++) {
        entrada_cache* entrada = list_get(cache, i);
        if (entrada->nro_pagina == numPag){
            return entrada;
        }
    }
    return NULL; // Si no se encuentra la entrada

}

char* leerEnCache(int tamanio, int numPag, int desplazamiento){
    usleep(RETARDO_CACHE*1000);
    entrada_cache* entrada = buscarEntradaCache(numPag);
    if(entrada != NULL && entrada->valor != NULL){
        char* valor = malloc(tamanio + 1); // +1 para el \0
        memcpy(valor, entrada->valor+desplazamiento, tamanio);
        valor[tamanio] = '\0'; // Asegurar que es un string con el \0 al final
        // log_info(cpu_logger,"Se leyo de la cache el contenido: %s",valor);
        log_info(cpu_logger, "PID: %d - Acción: LEER - Dirección Física: %d - Valor: %s", //LOG OBLIGATORIO AUQNUE LEA DESDE LA CACHE LO PONGO
            proceso->pid,
            entrada->direccion_fisica,
            valor);
        return valor;
    }
    return NULL; // Si no se encuentra la entrada o no hay valor
}

int procedimientoParaCacheHabilitada(int direccionLogica,int direccionFisicaParaPagEnCache, int nro_pagina, int desplazamiento){
   
        //como la tlb esta habilitada le voy a pedir la pagina completa despues de sacar la direccion fisica
        t_buffer* bufferPaginaCompleta = crear_buffer();
        cargar_entero_al_buffer(bufferPaginaCompleta, proceso->pid);
        cargar_entero_al_buffer(bufferPaginaCompleta, direccionFisicaParaPagEnCache);

        log_debug(cpu_logger,"Se carga el pid %d y la dir fisica %d",proceso->pid, direccionFisicaParaPagEnCache);

        t_paquete* paquete = crear_paquete(LEER_PAGINA_COMPLETA, bufferPaginaCompleta);
        enviar_paquete(paquete, fd_conexion_memoria);
        eliminar_paquete(paquete);

        op_code cod_op = recibir_operacion(fd_conexion_memoria);
            
            if (cod_op == LEER_PAGINA_COMPLETA_RTA) {
                t_buffer* buffer = recibir_todo_un_buffer(fd_conexion_memoria);
                char* contenido_pagina = (char*)extraer_choclo_del_buffer(buffer);
                contenido_pagina[TAM_PAGINA-1]='\0';
                log_debug(cpu_logger,"Se recibio de Memoria el contenido: %s de la pagina: %d",contenido_pagina,nro_pagina);
                destruir_buffer(buffer);
                actualizar_cache(nro_pagina, direccionLogica, direccionFisicaParaPagEnCache, contenido_pagina);
                
            }

        if(strcmp(instruccion->operacion, "READ")== 0){
            leerEnCache(instruccion->args->tamanio, nro_pagina, desplazamiento);
            entrada_cache_actual->bit_uso = true;
            return -1;
        }else if(strcmp(instruccion->operacion, "WRITE")==0){
            escribirEnCache(instruccion->args->datos, nro_pagina,desplazamiento);
            entrada_cache_actual->bit_modificado = true;
            entrada_cache_actual->bit_uso = true;
            return -1;
        }else{
            log_error(cpu_logger, "la operacion no es READ ni WRITE, no se puede operar sobre la cache");
            // exit(EXIT_FAILURE);
        }
    
}





void liberarEntrada_tlb(void* entrada){
    entrada_tlb* unaEntrada = (entrada_tlb*) entrada;
    free(unaEntrada);
}

void liberaraEntrada_cache(void* entrada){
    entrada_cache* unaEntrada = (entrada_cache*) entrada;
    free(unaEntrada->valor);
    free(unaEntrada);
}