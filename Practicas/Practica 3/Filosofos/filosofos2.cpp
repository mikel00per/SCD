#include <iostream>
#include <time.h>      // incluye "time"
#include <unistd.h>    // incluye "usleep"
#include <stdlib.h>    // incluye "rand" y "srand"
#include <mpi.h>

using namespace std;

// Identificador del proceso CAMARERO
#define CAMARERO  10

/*
  Acciones asociadas a los TAGS de los procesos.
*/
#define SOLTAR      0
#define COGER       1
#define SENTARSE    2
#define LEVANTARSE  3


void Filosofo( int id, int nprocesos);
void Tenedor ( int id, int nprocesos);
void Camarero();

// ###########################################################################
// ###########################################################################

int main( int argc, char** argv )
{
  int rank, size;

  srand(time(0));
  MPI_Init( &argc, &argv );
  MPI_Comm_rank( MPI_COMM_WORLD, &rank );
  MPI_Comm_size( MPI_COMM_WORLD, &size );

  if( size!=11)
  {
    if( rank == 0)
      cout<<"El numero de procesos debe ser 11" << endl << flush ;
    MPI_Finalize( );
    return 0;
  }

  if (rank == CAMARERO) { // 10 es el camarero
    Camarero();
  }                        // El resto es igual, resto -1 pues size es ahora 11
  else if ((rank%2) == 0)
    Filosofo(rank,size-1); // Los pares son Filosofos
  else
    Tenedor(rank,size-1);  // Los impares son Tenedores

  MPI_Finalize();
  return 0;
}

void Camarero()
{
  int      sentados = 0,
               Filo    ,
               rama    ,
                buf    ;
  MPI_Status status    ;

  while(1)
  {
    if (sentados < 4) { // Sigo recibiendo mensajes, pueden LEVANTARSE y SENTARSE
      MPI_Recv(&buf, 0, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    }else               // Todos se han sentado, busco el mensaje de levantarse
      MPI_Recv(&buf, 0, MPI_INT, Filo, LEVANTARSE, MPI_COMM_WORLD, &status);

    // Sé lo que hace según el tag del mensaje origen a diferencia del prodCons
    rama = status.MPI_TAG;

    switch (rama) {

      case SENTARSE:
        Filo = status.MPI_SOURCE;
        sentados++;
        cout << "Camarero     sienta Filos.  [" << Filo << "]" << endl << flush;
        break;

      case LEVANTARSE:
        Filo = status.MPI_SOURCE;
        sentados--;
        cout << "Camarero     levanta Filos. [" << Filo << "]" << endl << flush;
        break;
    }
  }
}

// ---------------------------------------------------------------------

void Filosofo( int id, int nprocesos )
{
  int peticion;
  int asiento;
  int izq = (id+1) % nprocesos;
  int der = ((id+nprocesos)-1) % nprocesos;

  while(true)
  {

    // Solicitar asiento
    cout << "Filosofo [" << id << "] solicita un    [ASIENTO]" << endl << flush;
    MPI_Ssend( &peticion, 0, MPI_INT, CAMARERO, SENTARSE, MPI_COMM_WORLD );
    cout << "Filosofo [" << id << "] se sienta en   [ASIENTO]" << endl << flush;

    // Solicita tenedor izquierdo
    cout << "Filosofo [" << id << "] pide iTenedor  [" << izq << "]" << endl << flush;
    MPI_Ssend( &peticion, 0, MPI_INT, izq, COGER, MPI_COMM_WORLD );
    cout << "Filosofo [" << id << "] coge iTenedor  [" << izq << "]" << endl << flush;

    // Solicita tenedor derecho
    cout <<"Filosofo [" << id << "] pide dTenedor  [" << der  << "]" << endl << flush;
    MPI_Ssend( &peticion, 0, MPI_INT, der, COGER, MPI_COMM_WORLD );
    cout <<"Filosofo [" << id << "] coge dTenedor  [" << der  << "]" << endl << flush;


    // C O M E
    cout <<"Filosofo [" << id << "] come           [RAND]"<< endl << flush;
    sleep((rand() % 3)+1);  //comiendo

    // Suelta el tenedor izquierdo
    cout <<"Filosofo [" << id << "] deja Tenedor   [" << izq << "]" << endl << flush;
    MPI_Ssend( &peticion, 0, MPI_INT, izq, SOLTAR, MPI_COMM_WORLD );

    // Suelta el tenedor derecho
    cout <<"Filosofo [" << id << "] deja Tenedor   [" << der << "]" << endl << flush;
    MPI_Ssend( &peticion, 0, MPI_INT, der, SOLTAR, MPI_COMM_WORLD );

    // L E V A N T A R S E
    cout <<"Filosofo [" << id << "] se levanta de  [ASIENTO]" << endl << flush;
    MPI_Ssend( &peticion, 0, MPI_INT, CAMARERO, LEVANTARSE, MPI_COMM_WORLD );

    // P I E N S A
    cout <<"Filosofo [" << id << "] piensa         [RAND]" << endl << flush;
    usleep( 1000U * (100U+(rand()%900U)) );


  }
}

// ---------------------------------------------------------------------

void Tenedor(int id, int nprocesos)
{
  int Filo, buf;
  MPI_Status status;

  while( true )
  {
    // Espera un peticion desde cualquier filosofo vecino ...
    MPI_Recv(&buf, 1, MPI_INT, MPI_ANY_SOURCE, COGER, MPI_COMM_WORLD,&status);

    //MPI_Recv(&Filo, 1, MPI_INT, buf, COGER, MPI_COMM_WORLD, &status);
    // Recibe la peticion del filosofo ...
    Filo = status.MPI_SOURCE;
    cout << "Tenedor  [" << id << "] pet. Filosofo  [" << Filo << "]" << endl << flush;

    // Espera a que el filosofo suelte el tenedor...
    MPI_Recv(&Filo, 1, MPI_INT, Filo, SOLTAR, MPI_COMM_WORLD, &status);
    cout << "Tenedor  [" << id << "] lib. Filosofo  [" << Filo << "]" << endl << flush;
  }
}
// ---------------------------------------------------------------------
