#ifndef TABLA_H
#define TABLA_H

#include <commons/collections/list.h>
#include <stdbool.h>

typedef struct t_tabla
{   bool ocupadaPorCompleto;
    int numTablaCreada;//empeiza en 1
    // int numTablaUltimoNivel; //numero de tabla del ultimo nivel, o sea el numero de tabla de paginas que contiene las paginas del proceso
    int nivel;
    struct t_tabla* tabla_paginasPadre; 
    t_list* lista_paginas;//en esta lista se guardan las tablas de paginas hijas, o sea, del siguiente nivel
} t_tabla;

#endif
