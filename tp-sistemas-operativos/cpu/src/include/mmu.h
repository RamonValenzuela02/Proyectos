#ifndef MMU_H
#define MMU_H



#include "gestor_cpu.h"
#include "cpu.h"
#include "bibliotecas.h"
#include <math.h>
#include "../../utils/src/utils/include/conexiones.h"

typedef struct {
    int nro_pagina;
    int  nro_marco;
    uint64_t timestamp;  // Para LRU
} entrada_tlb;


// ----- CACHE -----
typedef struct {
    int pid_entrada;
    int nro_pagina;
    int direccion_fisica;
    char* valor;
    bool bit_uso;
    bool bit_modificado;
    int offset; 
    int direccion_logica;  
} entrada_cache;

extern int CANTIDAD_NIVELES;
extern int ENTRADAS_POR_TABLA;
extern int TAM_PAGINA;

void obtener_config_de_memoria(int socket_memoria);
int traducir_direccion_logica(int pid, t_instruccion* instrucciones);
void inicializar_estructuras();
bool verificar_cache(int nro_pagina);
bool verificar_tlb(int nro_pagina);
char* pedir_valor_a_memoria (int direccion_fisica, int tamanio, int pid, int numPag);
void escribir_memoria(int pid, int direccion_fisica, char* datos);
int obtener_marco_mmu(int pid,int nro_pagina);
void actualizar_tlb(int nro_pagina,int nro_marco);
// void reemplazo_tlb_fifo();
uint64_t get_timestamp();
// void reemplazo_tlb_lru();
void reemplazo_cache_clock(entrada_cache* nueva_entrada);
void reemplazo_cache_clock_m(entrada_cache* nueva_entrada);
// void actualizar_cache(int nro_pagina, int direccion_fisica, char* valor);
void actualizar_cache(int nro_pagina, int direccion_logica, int direccion_fisica, char* valor);
void liberar_tlb(t_list* tlb);
void liberar_cache(t_list* cache);

// Reemplazo de TLB por FIFO

void reemplazo_tlb_fifo(entrada_tlb* nueva_entrada);
void reemplazo_tlb_lru(entrada_tlb* nueva_entrada);
double calcularTiempo(entrada_tlb* entrada);

int pedir_tabla_nivel(int pid, int entrada, int nivel);
int pedir_marco(int pid, int tabla, int entrada);
void escribirPaginaCompletaEnCache(char* dato_a_escribir, int numPag,entrada_cache* entrada_a_agregar);
entrada_cache* buscarEntradaCache(int numPag);
char* leerEnCache(int tamanio, int numPag, int desplazamiento);

void escribirEnCache(char* dato_a_escribir, int numPag, int desplazamiento);
  
int procedimientoParaObtenerMarcoDeMemoria(int nro_pagina, int desplazamiento, int pid);

// int procedimientoParaCacheHabilitada(int direccionFisicaParaPagEnCache, int nro_pagina, int desplazamiento);
int procedimientoParaCacheHabilitada(int direccionLogica,int direccionFisicaParaPagEnCache, int nro_pagina, int desplazamiento);

// extern t_proceso* proceso_anterior;
// extern t_proceso* proceso;
extern char* valor_leido_memoria;
extern  int direccion_fisica;


extern t_list* cache;
extern t_list* tlb;

#endif

