#include <iostream>
#include <time.h>      // incluye "time"
#include <unistd.h>    // incluye "usleep"
#include <stdlib.h>    // incluye "rand" y "srand"
#include <mpi.h>

using namespace std;

#define SOLTAR  0
#define COGER   1

void Filosofo( int id, int nprocesos);
void Tenedor ( int id, int nprocesos);

// ##########################################################################

int main( int argc, char** argv )
{
   int rank, size;

   srand(time(0));
   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &rank );
   MPI_Comm_size( MPI_COMM_WORLD, &size );

   if( size!=10)
   {
      if( rank == 0)
         cout<<"El numero de procesos debe ser 10" << endl << flush ;
      MPI_Finalize( );
      return 0;
   }

   if ((rank%2) == 0)
      Filosofo(rank,size); // Los pares son Filosofos
   else
      Tenedor(rank,size);  // Los impares son Tenedores

   MPI_Finalize( );
   return 0;
}


void Filosofo( int id, int nprocesos )
{
  int peticion;
  int izq = (id+1) % nprocesos;
  int der = ((id+nprocesos)-1) % nprocesos;

  while(true)
  {
    if (id == 0)  // Solución, hacer que el primero coga el de la derecha y
    {             // Posteriormente el de la izquierda.
      // Solicita tenedor derecho
      cout <<"Filosofo [" << id << "] pide dTenedor  [" << der  << "]" << endl << flush;
      MPI_Ssend( &peticion, 0, MPI_INT, der, COGER, MPI_COMM_WORLD );
      cout <<"Filosofo [" << id << "] coge dTenedor  [" << der  << "]" << endl << flush;

      // Solicita tenedor izquierdo
      cout << "Filosofo [" << id << "] pide iTenedor  [" << izq << "]" << endl << flush;
      MPI_Ssend( &peticion, 0, MPI_INT, izq, COGER, MPI_COMM_WORLD );
      cout << "Filosofo [" << id << "] coge iTenedor  [" << izq << "]" << endl << flush;
    }
    else
    {
      // Solicita tenedor izquierdo
      cout << "Filosofo [" << id << "] pide iTenedor  [" << izq << "]" << endl << flush;
      MPI_Ssend( &peticion, 0, MPI_INT, izq, COGER, MPI_COMM_WORLD );
      cout << "Filosofo [" << id << "] coge iTenedor  [" << izq << "]" << endl << flush;

      // Solicita tenedor derecho
      cout <<"Filosofo [" << id << "] pide dTenedor  [" << der  << "]" << endl << flush;
      MPI_Ssend( &peticion, 0, MPI_INT, der, COGER, MPI_COMM_WORLD );
      cout <<"Filosofo [" << id << "] coge dTenedor  [" << der  << "]" << endl << flush;
    }

    // C O M E
    cout <<"Filosofo [" << id << "] come           [RAND]"<< endl << flush;
    sleep((rand() % 3)+1);  //comiendo

    // Suelta el tenedor izquierdo
    cout <<"Filosofo [" << id << "] deja Tenedor   [" << izq << "]" << endl << flush;
    MPI_Ssend( &peticion, 0, MPI_INT, izq, SOLTAR, MPI_COMM_WORLD );

    // Suelta el tenedor derecho
    cout <<"Filosofo [" << id << "] deja Tenedor   [" << der << "]" << endl << flush;
    MPI_Ssend( &peticion, 0, MPI_INT, der, SOLTAR, MPI_COMM_WORLD );

    // Piensa (espera bloqueada aleatorio del proceso)
    cout <<"Filosofo [" << id << "] piensa         [RAND]" << endl << flush;

    // espera bloqueado durante un intervalo de tiempo aleatorio
    // (entre una décima de segundo y un segundo)
    usleep( 1000U * (100U+(rand()%900U)) );
  }
}

/**
 * [Tenedor description]
 * @param id        [description]
 * @param nprocesos [description]
 */
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
