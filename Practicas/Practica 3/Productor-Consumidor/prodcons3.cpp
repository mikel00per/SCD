#include <mpi.h>
#include <iostream>
#include <math.h>
#include <time.h>      // incluye "time"
#include <unistd.h>    // incluye "usleep"
#include <stdlib.h>    // incluye "rand" y "srand"

#define Productor    4
#define Buffer       5
#define Consumidor   6

/*
  Definimos la cantidas de procesos que habrá de cada tipo para
  saber hasta donde iterar.
*/
#define NUMPRODUCTORES    5
#define NUMCONSUMIDORES   4

/*
  Defino la cantidad de iteraciones de cada proceso:
    - ITERS:      Num de producciones
    - ITERS_CONS: 25 consumiciones. Productor < Consumidor
    - ITERS_BUFF: Iteraciones totales de productoresy consumidores
*/
#define ITERS         20
#define ITERS_CONS    (ITERS * 5)/4
#define ITERS_BUFF    ITERS*NUMPRODUCTORES+ITERS_CONS*NUMCONSUMIDORES
#define TAM           20

using namespace std;

/**
 * Proceso productor.
 *
 * El único cambio ha sido la modificación de la salida a la vez que he
 * añadido idProceso para poder identificar por el rank que productor
 * produce en cada caso.
 *
 * NOTA: La salida ":::" significa profucción.
 *
 */
  void productor(int idProceso) {
    for ( unsigned int i=0; i < ITERS ; i++ ) {
      cout << "Productor  ["<< idProceso << "]   :::   {" << i << "}" << endl << flush ;

      // espera bloqueado durante un intervalo de tiempo aleatorio
      // (entre una décima de segundo y un segundo)
      usleep( 1000U * (100U+(rand()%900U)) );

      // enviar 'value'
      MPI_Ssend( &i, 1, MPI_INT, Buffer, 0, MPI_COMM_WORLD );
      cout << "Productor  ["<< idProceso << "]   -->   [" << i << "] " << endl << flush ;
    }
  }

/**
 * Proceso buffer.
 *
 * El cambio más significativo para el proceso buffer es el cambio en el
 * num de iteraciones. Por otro lado la recepción del mensaje se hace con
 * MPI_ANY_TAG ya que no sabemos que proceso consumirá o producirá. También
 * he cambiado el calculo de la rama, si el MPI.SOURCE del proceso es menor
 * que el identificador Productor implica rama 0, en caso contrario la rama es
 * uno. Por último he cambiado la salida del programa,  <-- y --->, indican
 * recepción y envio del los mensajes.
 */
void buffer()
{
  int   value[TAM]   ,
  peticion           ,
  pos  =  0          ,
  rama               ;
  MPI_Status status  ;

  for( unsigned int i=0 ; i < ITERS_BUFF ; i++ ) {
    if ( pos==0 )      // el consumidor no puede consumir
      rama = 0 ;
    else if (pos==TAM) // el productor no puede producir
      rama = 1 ;
    else {             // ambas guardas son ciertas
      // leer 'status' del siguiente mensaje (esperando si no hay)
      MPI_Probe( MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status );

      // calcular la rama en función del origen del mensaje
      if ( status.MPI_SOURCE <= Productor )
        rama = 0 ;
      else
        rama = 1 ;

    }

    switch(rama) {
      case 0:
        MPI_Recv( &value[pos], 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status );
        cout << "Buffer           <--   [" << value[pos] << "] " << " del Productor  [" << status.MPI_SOURCE <<  "]" << endl << flush;
        pos++;
        break;
      case 1:
        MPI_Recv( &peticion, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status );
        MPI_Ssend( &value[pos-1], 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
        cout << "Buffer           -->   [" << value[pos-1] << "] " << " del Consumidor [" << status.MPI_SOURCE <<  "]" << endl << flush;
        pos--;
        break;
    }
  }
}

/**
 * Proceso consumidor.
 *
 * El cambio principal es el número de iteracciónes. Cada consumidor iterará
 * 25 ya que ha de consumir 100 datos entre 4 procesos distintos. Por último
 * he cambiado la salida.
 */
void consumidor(int idProceso) {
  int         value         ,
              peticion = 1  ;
  float       raiz          ;
  MPI_Status  status        ;

  for (unsigned int i=0; i < ITERS_CONS; i++) {
    MPI_Ssend( &peticion, 1, MPI_INT, Buffer, 1, MPI_COMM_WORLD );
    MPI_Recv ( &value,    1, MPI_INT, Buffer, 0, MPI_COMM_WORLD, &status );
    cout << "Consumidor [" << idProceso << "]   <--   [" << value << "] " << endl << flush ;

    // espera bloqueado durante un intervalo de tiempo aleatorio
    // (entre una décima de segundo y un segundo)
    usleep( 1000U * (100U+(rand()%900U)) );

    raiz = sqrt(value) ;
  }
}

// ---------------------------------------------------------------------

int main(int argc, char *argv[])
{
   int rank,size;

   // inicializar MPI, leer identif. de proceso y número de procesos
   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &rank );
   MPI_Comm_size( MPI_COMM_WORLD, &size );

   // inicializa la semilla aleatoria:
   srand ( time(NULL) );

   // comprobar el número de procesos con el que el programa
   // ha sido puesto en marcha (debe ser 3)
   if ( size != 10 )
   {
      cout<< "El numero de procesos debe ser 10 "<<endl;
      return 0;
   }

   // verificar el identificador de proceso (rank), y ejecutar la
   // operación apropiada a dicho identificador
   if ( rank <= Productor )
      productor(rank);
   else if ( rank == Buffer )
      buffer();
   else
      consumidor(rank-Consumidor);  // La resta solo afecta a la salida
                                    // así, se verá consumior 0,1,2,3.

   // al terminar el proceso, finalizar MPI
   MPI_Finalize( );
   return 0;
}
