#ifndef PRUEBA_H_
#define PRUEBA_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>
#include <commons/config.h>
#include <pthread.h>
#include <semaphore.h>
#include <commons/collections/list.h>
#include <commons/bitarray.h>
#include <stdint.h>
#include <stdbool.h>
#include <commons/collections/queue.h>





typedef enum
{
	MENSAJE,
	SOYKERNEL,
	PAQUETE,
	HANDSHAKE_IO_KERNEL,
	HANDSHAKE_CPU_KERNEL,
	HANDSHAKE_MEM_KERNEL,
	HANDSHAKE_MEM_CPU,
	//KERNEL -> MEMORIA
	CREATE_PROCESS,
	ESPACIO_DISPONIBLE_PARA_PROCESO,
	ELIMINAR_PROCESO_DE_MEMORIA,
	DUMP_MEMORY,
	SUSPENDER_PROCESO,
	DESSUSPENDER_PROCESO,
	//MEMORIA -> KERNEL
	RTA_ESPACIO_DISPONIBLE_PARA_PROCESO,
	RTA_ELIMINAR_PROCESO_DE_MEMORIA,
	RTA_DUMP_MEMORY,
	RTA_CREATE_PROCESS,
	//KERNEL - CPU 
	EJECUCION_DE_PROCESO_KER_CPU,           
	RTA_EJECUCION_DE_PROCESO_KERNEL_CPU,   //no se usa 
	DESALOJAR_PROCESO_KER_CPU,
	RTA_DESALOJO_DE_PROCESO_KERNEL_CPU,
	//KERNEL -> IO
	SOLICITUD_IO_DE_KERNEL,
	RTA_SOLICITUD_IO_DE_KERNEL,
	//MEMORIA -> CPU
	OBTENER_CDE_RTA,
	OBTENER_INSTRUCCION_RTA,
	SOLICITAR_VALOR_RTA,
	ESCRIBIR_VALOR_RTA,
	SOLICITAR_CONFIG_MEMORIA_RTA,
	PEDIR_TABLA_NIVEL_RTA,
	PEDIR_MARCO_RTA,
	LEER_PAGINA_COMPLETA_RTA,
	OK,
	//CPU -> MEMORIA
	SOLICITAR_CONFIG_MEMORIA,
	SOLICITAR_VALOR, //leer memoria
	ESCRIBIR_VALOR,
	GET_INSTRUCCION,
	ACTUALIZAR_CONTEXTO,
	PEDIR_TABLA_NIVEL,
	PEDIR_MARCO,
	LEER_PAGINA_COMPLETA,
	ACTUALIZAR_PAGINA_COMPLETA,
	PAG_MODIFICADAS,
	//CPU - KERNEL NUEVAS 
	SOL_EXIT_CPU_KER,
	SOL_INIT_PROC_CPU_KER,
	RTA_EJECUCION_DE_PROCESO_KER_CPU,
	SOL_IO_CPU_KER,
	SOL_DUMP_MEMORY_CPU_KER

}op_code;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;


//FUNCIONES INICIALIZADORAS DE CONFIG Y LOGGERS
t_log* iniciar_logger(char* direccion,char* infConetenida);
t_config* iniciar_config();

int crear_conexion(char *ip, char* puerto);
int iniciar_servidor(char *puerto, t_log* logger, char* mensaje);
int esperar_cliente(int socket_servidor);
int recibir_operacion(int socket_cliente);

//FUNCIONES COMPARTIDAS ENTRE MODULOS
void enviar_hand_shake_cpu(op_code operacion, int fd_server, int identificador);
void enviar_hand_shake_kernel_mem(op_code operacion, int fd_server);


//funciones cargar a un buffer 
void cargar_choclo_al_buffer(t_buffer* un_buffer,void* choclo, int size_choclo);
void cargar_entero_al_buffer(t_buffer* un_buffer, int value);
void cargar_string_al_buffer(t_buffer* un_buffer, char* un_string);
void cargar_uint32_al_buffer(t_buffer* un_buffer,uint32_t value);
void cargar_double_al_buffer(t_buffer* un_buffer, double value);

//funciones para recibir de un buffer 
//ORDENAR POR FUNCIONALIDADES!!!!!!!!!!!!!!!!

t_buffer* recibir_todo_un_buffer(int fd);
t_buffer* crear_buffer();
void eliminar_buffer(t_buffer *buffer);
void destruir_buffer(t_buffer* un_buffer);
t_paquete* crear_paquete(op_code codigo, t_buffer* un_buffer);
void* serializar_paquete(t_paquete* un_paquete);
void enviar_paquete(t_paquete* un_paquete, int conexion);
void eliminar_paquete(t_paquete* paquete);
void eliminar_paquete(t_paquete *paquete);
t_paquete *iniciar_paquete(int cod_op);


void* extraer_choclo_del_buffer(t_buffer* un_buffer);
int extraer_int_del_buffer(t_buffer* buffer);
uint32_t extraer_uint32_del_buffer(t_buffer* buffer);
double extraer_double_del_buffer(t_buffer* un_buffer);
char* extraer_string_del_buffer(t_buffer* buffer);

#endif