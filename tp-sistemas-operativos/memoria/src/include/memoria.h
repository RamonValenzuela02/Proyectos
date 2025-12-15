#ifndef MEMORIA_H_
#define MEMORIA_H_

#include "gestor_memoria.h"
#include <stdbool.h>
#include <commons/temporal.h>
#include "tabla.h"


typedef struct{
    int pid_proceso;
    int tamanio_proceso;
    t_queue* colaDeCaminoAPag; 
    t_queue* colaDECaminoNumTP;
    t_list* lista_instrucciones_proceso;
    t_list* lista_pag_del_proceso;
    int cantAccesosTP;
    int cantInstruccionesSolicitadas;
    int cantBajadasSwap;
    int cantSubidasMP;
    int cantLecturasMem;
    int cantEscriturasMem;
    int pc;
}t_proceso;

typedef struct{
    int* primerParamNum;
    char* primerParamString;
    int* segundoParamNum;
    char* segundoParamString;
}t_parametros;


typedef struct
{
    int id;
    char* operacion;
    t_parametros* parametros;   
}t_instruccion;

typedef struct{
    int base;
    int limite;
}t_frame;

typedef struct
{
    t_frame frame;
    int bit_ocupado;
    int num_frame;
    int num_pagina;
    int bit_modificado; //la pag fue escrita
    int num_pag_local_pid; //el numero de pág con el que lo va ver su pid
   
} t_pagina;

typedef enum{
    CREATE,
    INGRESO,
    SE_SUSPENDE,
    SE_DESUSPENDE,
    SE_MUERE
}tipo_consulta;


t_log* mem_logger;   
// t_log* logger_debug;
t_config* mem_config;

//sockets
int fd_server;      

//config
char* PUERTO_MEMORIA;
int TAM_MEMORIA;
int TAM_PAGINA; 
int ENTRADAS_POR_TABLA;  
int RETARDO_MEMORIA; 
char* PATH_SWAP; 
int RETARDO_SWAP; 
char* LOG_LEVEL; 
char* DUMP_PATH;
int CANT_NIVELES;
char* PATH_INSTRUCCIONES;

//variables globales memoria
void* memoria_usuario;
int cant_procesosMem;
int TAM_OCUPADO;
int CANT_FRAMES;
//swap
int archivoSwapFile;
void* swapFileMapeado;
typedef struct {
    int base;
    int limite;
    int ocupado;
    int numPag;
    int pid;
}t_entradaSwap;

//para cortar el hilo principal
volatile sig_atomic_t apagar_memoria = 0;  


//bitarray
t_bitarray* bitarray_frames;
//listaSwap
t_list* entradasSwap;
//listas prccesos
t_list* lista_procesos;
//tabla de paginas nivel 1
t_tabla* tabla_pag_nivel1;

t_list* procesos_suspendidos;

t_list* contenido_de_pidsNoSuspendidos;

//sockets de cpu
t_list* sockets_de_cpus;

int numTablaCreada;
int numero_de_pagina;
bool hay_proceso_suspendido;
int tamanio_necesario;

//semaforos
pthread_mutex_t mutex_pc;
pthread_mutex_t mutex_pid;
pthread_mutex_t mutex_contador;
pthread_mutex_t mutex_add_lista;
pthread_mutex_t mutex_calculoTamMem;
pthread_mutex_t mutex_remove_lista;
pthread_mutex_t mutex_get_listaProcesos;
pthread_mutex_t mutex_get_listaInstrucciones;
pthread_mutex_t dump_Memory;
pthread_mutex_t mutex_inicializador_metricas;

pthread_mutex_t incrementarCantLecturas;
pthread_mutex_t incrementarCantSolicitudesInstr;
pthread_mutex_t incrementarCantEscriturasMem;
pthread_mutex_t cant_AccesosTP;
pthread_mutex_t cant_bajadas_swap;
pthread_mutex_t cant_bajadas_MP;

pthread_mutex_t numeroDeTablaCreada;
pthread_mutex_t mutexParaNumTP;
pthread_mutex_t mutex_sacar_liberar_espacio_suspencion;
pthread_mutex_t mutex_lista_paginas_proceso;
pthread_mutex_t mutex_entradas_swap;
pthread_mutex_t mutex_eliminar_estructuras;
pthread_mutex_t mutex_para_saber_si_hay_procesos_suspendidos;
pthread_mutex_t mutex_lista_procesos_suspendidos;

pthread_mutex_t mutex_send_ok_kernel;
pthread_mutex_t send_ok_cpu;
pthread_mutex_t mutex_leer_pagina;
pthread_mutex_t mutex_actualizar_pagina;
pthread_mutex_t escribir_valor;
pthread_mutex_t solicitar_valor;
pthread_mutex_t pedir_marco;
pthread_mutex_t pedir_tabla;
pthread_mutex_t mutex_buscando_num_pag_global;
sem_t proceso_escribiendo;
sem_t modificar_tamanio_ocupado_en_memoria;

pthread_mutex_t mutex_conexion_con_kernel;
int TAM_SWAP;
//firmas de funciones
void iniciarMemoria();
void cargar_config();
void* manejar_cliente(void* arg);
void* manejar_cliente_cpu(void* arg);
void* manejar_cliente_kernel(void* arg);

int chequear_y_actualizar_suspensiones(int tam_proceso_que_quiere_ingresar, tipo_consulta tipo_modificacion);
t_list* crear_lista_de_instrucciones(char* path_proceso);
void verificarOperacion(char* token, t_instruccion* instruccion);
void cargar_instruccion_al_buffer(t_buffer* buffer, t_instruccion* instruccion);
// void loguear_instruccion(t_instruccion* instruccion);
void loguear_instruccion(int pid_proceso, t_instruccion* instruccion);
void liberarPaginasAsignadasAlProceso(t_proceso* un_proceso);
void destruirPagina(void* ptr);
void eliminarColaProceso(t_proceso* un_proceso);
void eliminarInstrucciones(t_proceso* un_proceso);
void imprimirMetricas(t_proceso* un_proceso);
void iniciar_semaforos();
void crear_paginacion_jerarquica_nivel_n();
void rellenarTablaDeUltimoNivel(t_list* tablaPagQueContieneAlAnteUltimoNivel, int i);
int agregarUnNivelMas(int nivelActual, int nivelALlegar,t_tabla* tablaDePagDelUltimoNivelAlcanzado);
bool verificarSiTodasLasPagEstanOcupadas(t_list* lista);
bool paginasOcupadas(void* ptr);
int retornarNumDePrimerFrameLIbre();
int asignarNumDePrimerFrameLIbre();
void asignarTablasDePagAlProceso(int tamanioProceso, t_list* listaDePaginasAsociadasAlPid, int pid);
void asignar_paginas_recursivo(t_tabla* tabla_actual, int nivel_actual, int* paginas_asignadas, int total_paginas, t_list* paginas_asociadas, int pid);
char* pedir_valor_a_memoria(int direccionFisica, int tamanio, t_proceso* proceso);
void escribirPaginaCompleta(char* datoAEScribir, int direccionFisica, int numeroPagina, t_proceso* proceso);
int buscarTablaDelSigNivel(int nivelActualTabla,int entrada, int pid_proceso);
int obtenerMarco(int ultimoNivel, int entradaPagina, int pid_proceso);
void escribir_valor_en_memoria(char* datoAEScribir, int direccionFisica);
void marcarPaginasEscritas(t_proceso* proceso,char* datoAEScribir, int dirFisica);
int buscarNroPagina(int direccionFisicaPag, int pid);
void escribirPaginasEnSwap(int pidDeSuspendido);
bool seteoAlFrameComoLibre(t_pagina* unaPagina, t_tabla* tablaDeUltimoNivel);
bool marcarPaginaComoLibreEnTablaPag(t_pagina* unaPagina, t_tabla* tablaActual);
void escribirPaginasDeSwapAMP(int pidDelDes_Suspendido);
void liberarPaginasEnSwap(int pidDelDes_Suspendido);
void asignarFrameALaPagina(t_entradaSwap* unaPaginaSwap);
void buscarFrameLibreParaProcesoDessuspendido(char* contenidoADevolverAMP, int pid);
bool buscarPrimeraPagLibre(t_tabla* tablaActual, int numFrameLibre, t_proceso* proceso);
char* buscarContenidoDePagina(t_pagina* unaPagina);
void rellenarCamposDeEntradasASwap();
int  seteoPaginaOcupadaEnSwap(int num_pagina, int pid);
void generarDumpMemory(t_proceso* procesoDump);
void* mapearArchivoDump(int fdArchivoDump, int tamanioProceso);
int truncarArchivo(int tamanioProceso, char* path);
void crearArchivoSwap();
void eliminarEstructurasDelProcesoEnMemoria(t_proceso* un_proceso);
char* pedirPaginaCompetaAMemoria(int direccionFisica, int numPag, int tamanio, t_proceso* proceso);
int buscarPaginaSegunPagLocal(int numPagLocal, t_proceso*  proceso);
// t_proceso* buscar_proceso(t_list* lista, int pid);
t_proceso* buscar_proceso(int pid);
t_proceso* buscar_proceso_y_eliminar(int pid);

void chequearQueNoSeaNUll(void* unPuntero);
int buscarTablaSigNivelparaEStabilidad(int pid_procesoTB,int entrada, int nivel);
int buscarMarcoParaEstabilidad(int pid, int ultimaEntrada, t_proceso* procesoQueBuscaMarco);
int buscarNroPaginaAtravesDeDirFisicaGlobal(int direccionFisicaPag, int pid);
int buscarPaginaSegunPag(int numPag, t_proceso*  proceso);

//señales
void manejar_sigint(int senial);
void liberar_recursos();
void destruirTablasPaginas(void* tabla);
void destruirTpaginas(void* pagina) ;
void destruirProcesos(void* unProceso);
void eliminarElemColas(void* entrada);
void destruir_instruccion(void* unaInstruccion);

#endif
