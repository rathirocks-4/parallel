#ifndef __MULT_PRODUCTO_H
#define __MULT_PRODUCTO_H
#include "defines.h"

typedef struct { 
    long int *Ai;
    long int *Bj;
    long int *Cij;
} PuntoArg;

static DWORD WINAPI multiplica_punto_vec(LPVOID arg);

PuntoArg *th_arg;
HANDLE *th_handles;

// Multiplica 2 matrices particionando el trabajo por punto en la matriz resultante
// Espera que la matriz b est� transpuesta
void multiplica_producto_vec(long int A[HEIGHTA][WIDTHA], long int B[WIDTHB][HEIGHTB], long int C[HEIGHTA][WIDTHB]) {	
    int i, j, th_i = 0, th_counter = 0;	

    th_arg = (PuntoArg*)malloc(sizeof(PuntoArg)*HEIGHTA*WIDTHB);
    th_handles = (HANDLE*)malloc(sizeof(HANDLE)*HEIGHTA*WIDTHB);

    for(i=0; i < HEIGHTA; i++) {				
        for(j=0; j < WIDTHB; j++) {
            // Delegar cada punto a un procesador utilizando WorkPool pattern            
            th_arg[th_i].Ai = A[i];			
            th_arg[th_i].Bj = B[j];			
            th_arg[th_i].Cij = &C[i][j];	

            // invoca hilo
            th_handles[th_i] = CreateThread(
                NULL, 0, multiplica_punto_vec,
                &th_arg[th_i], 0, NULL);

            // Para ejecutar VTune Amplifier hubo que limitar los threads a 7 maximo,
            // pero en ejecucion sin analisis, se pudo crear 128x128 threads.
            // if(th_counter < 6) { th_i = ++th_counter; }
            // else { th_i = WaitForMultipleObjects(th_counter, th_handles, FALSE, INFINITE); }
        }
    }
    
    // Asegurarse de esperar a todos los threads.
    WaitForMultipleObjects(th_counter, th_handles, TRUE, INFINITE);
    
    free(th_arg);
    free(th_handles);
}


/// Utiliza vectorizacion para:
/// Multiplicar el punto PuntoArg.Cij de la matriz resultante
/// de la multiplicacion del renglon PuntoArg.Ai con el
/// renglon PuntoArg.Bj, para una matriz de tamano PutoArg.tamano.
/// <param name="p_arg">The p_arg.</param>
static DWORD WINAPI multiplica_punto_vec(LPVOID p_arg) {
    PuntoArg arg = *(PuntoArg*)p_arg;
    int k = 0;
    long int *Ai = arg.Ai, *Bj = arg.Bj, *Cij = arg.Cij;
    long int temp[WIDTHA + 4];
    temp[0] = temp[1] = temp[2] = temp[3];
#pragma ivdep
    for(k = 0; k < WIDTHA; k++) {
        temp[k+4] = temp[k] + Ai[k] * Bj[k];
    }

    *Cij = temp[WIDTHA] + temp[WIDTHA+1] + temp[WIDTHA+2] + temp[WIDTHA+3];

    return 0;
}
#endif