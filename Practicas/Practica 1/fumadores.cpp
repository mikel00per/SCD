// *****************************************************************************
//
// Prácticas de SCD. Práctica 1.
// Plantilla de código para el ejercicio de los fumadores
//
// *****************************************************************************

#include <iostream>
#include <cassert>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>      // incluye "time(....)"
#include <unistd.h>    // incluye "usleep(...)"
#include <stdlib.h>    // incluye "rand(...)" y "srand"

using namespace std ;

// ---------------------------------------------------------------------
// como son tres hebras uso un vector de semaforos para controlarlas.
const int num_hebras = 3;

// ---------------------------------------------------------------------
// Semaforos necesarios

sem_t fumadores[num_hebras],
      estanquero,
      mutex;

// ---------------------------------------------------------------------
// introduce un retraso aleatorio de duración comprendida entre
// 'smin' y 'smax' (dados en segundos)

void retraso_aleatorio( const float smin, const float smax )
{
  static bool primera = true ;
  if ( primera )        // si es la primera vez:
  {  srand(time(NULL)); //   inicializar la semilla del generador
     primera = false ;  //   no repetir la inicialización
  }
  // calcular un número de segundos aleatorio, entre {\ttbf smin} y {\ttbf smax}
  const float tsec = smin+(smax-smin)*((float)random()/(float)RAND_MAX);
  // dormir la hebra (los segundos se pasan a microsegundos, multiplicándos por 1 millón)
  usleep( (useconds_t) (tsec*1000000.0)  );
}

// ----------------------------------------------------------------------------
// función que simula la acción de fumar, como un retardo aleatorio de la hebra.
// recibe como parámetro el numero de fumador
// el tiempo que tarda en fumar está entre dos y ocho décimas de segundo.

void fumar( int num_fumador )
{
   sem_wait(&mutex);
   cout << "Fumador número " << num_fumador << ": comienza a fumar." << endl << flush ;
   sem_post(&mutex);

   retraso_aleatorio( 0.2, 0.8 );

   sem_wait(&mutex);
   cout << "Fumador número " << num_fumador << ": termina de fumar." << endl << flush ;
   sem_post(&mutex);
}

// ----------------------------------------------------------------------------
// función que simula la acción de fumar, como un retardo aleatorio de la hebra.
// recibe como parámetro el numero de fumador
// el tiempo que tarda en fumar está entre dos y ocho décimas de segundo.

void producir( int elemento )
{
  string element = "";

  if (elemento == 0) {
    element = "cerillas";
  } else if (elemento == 1) {
    element = "tabaco";
  } else {
    element = "papel";
  }

  cout << "Soy el estanquero aqui tiene: " << element << endl << flush ;
  retraso_aleatorio (0.2, 0.5);
  cout << "Gracias por recogerlo señor.  " << endl << flush ;
}

// ----------------------------------------------------------------------------
// función que simula a un estanquero que reparte los ingredientes.

void * funcion_productor(void *)
{
  int dato;
  while (true) {

    sem_wait(&estanquero);
    dato = random()%num_hebras;

    sem_wait(&mutex);
    producir(dato);
    sem_post(&mutex);

    sem_post(&(fumadores[dato]));
  }
}

// ----------------------------------------------------------------------------
// función que simula a un fumador

void * funcion_fumador(void * id_void)
{
  unsigned long pos_fum = (unsigned long) id_void ;

  while (true) {
    sem_wait(&(fumadores[pos_fum])); // Se desbloquea (y lo coge de la mesa)
    sem_post(&estanquero);           // Le mando producir otro

    fumar(pos_fum);                  // mismamente o se está fumando o produciendo
  }
}

// ----------------------------------------------------------------------------

int main()
{
  srand( time(NULL) );

  pthread_t hebra_productora, hebras_fumadoras[num_hebras];

  // Creación de semaforos
  for (int i = 0; i < num_hebras; i++)
    sem_init(&(fumadores[i]),  0,  0);
  sem_init(&estanquero,        0,  1);
  sem_init(&mutex,             0,  1);

  // Creación hebras
  pthread_create( &hebra_productora, NULL, funcion_productor, NULL);
  for (unsigned long i  = 0; i < num_hebras; i++){
    void * arg_ptr = (void *) i;
    pthread_create( &(hebras_fumadoras[i]), NULL, funcion_fumador, arg_ptr);
  }
  // Espero a que acaben todas
  for (int i = 0; i < num_hebras; i++)
    pthread_join( hebras_fumadoras[i], NULL );
  pthread_join( hebra_productora, NULL);

  // Cierro semaforos
  for (int i = 0; i < num_hebras; i++)
    sem_destroy(&(fumadores[i]));
  sem_destroy(&estanquero);
  sem_destroy(&mutex);

  return 0 ;
}
