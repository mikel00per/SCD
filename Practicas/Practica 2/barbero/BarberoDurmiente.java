import monitor.*;
import java.util.Random;

class Barberia extends AbstractMonitor{

  private Condition estadoBarbero,
                    estadoSala   ,
                    sillaOcupada ;

  /**
   * Constructor por defecto;
   * @return devulve el objeto implicito.
   */
  public Barberia(){
    estadoBarbero = makeCondition();
    estadoSala    = makeCondition();
    sillaOcupada  = makeCondition();
  }

  /**
   * Función que simula a un cliente cortandonse el pelo.
   * Si el cliente llega a la barbería y la silla está ocupado
   * lo dice y espera en la sala. Si hay un estanquero dormido,
   * lo despierta, comienza a pelarse y ocupa la silla.
   */
  public void cortarPelo(){

    enter();
      aux.mensaje("He llegado a la barbería");

      if (sillaOcupada.isEmpty() && !estadoBarbero.isEmpty()) {
        aux.mensaje("Despierto al barbero para pelarme");
        estadoBarbero.signal(); // 'Consumo al barbero dormido'
      }

      if (!sillaOcupada.isEmpty()) {
        aux.mensaje("Silla ocupada :/");
        estadoSala.await();  // Meto un cliente más a la espera
      }

      aux.mensaje("¡Comienzo a pelarme!");
      sillaOcupada.await();     // Ocupo la silla
    leave();

  }

  /**
   * Función que simula decidiendo si trabajar o irse a dormir.
   * Si la sala está vacía me voy a dormir, si no lo está significa que
   * pelaré a un nuevo cliente.
   */
  public void siguienteCliente(){

    enter();
      if (estadoSala.isEmpty()) {   // Silla vacía
        aux.mensaje("¡No hay trabajo, me voy a dormir!");
        estadoBarbero.await();  // Duermo al barbero
      }

      aux.mensaje("¡Puedo pelar!");
      estadoSala.signal();

    leave();
  }

  /**
   * Función que simula el fin de un servicio.
   * Se anuncia el fin y la silla se cambia de estado para notificar
   * que un nuevo cliente puede sentarse.
   */
  public void finCliente(){

    enter();
      aux.mensaje("He acabado con un cliente.");
      sillaOcupada.signal();
    leave();
  }
}

/*********************************************************************/

class Cliente implements Runnable{
  int idCliente;
  private Barberia barberia;
  public Thread thr;

  /**
   * Constructor por parámetros
   * @param  Barberia laBarberia    monitor para controlar a los clientes.
   * @param  int      miCliente     cliente al que se le asigna el monitor.
   * @return          devuelve el objeto implícito.
   */
  public Cliente(Barberia laBarberia, int miCliente){
    barberia        = laBarberia;
    idCliente       = miCliente;
    thr             = new Thread(this, "Cliente ["+miCliente+"]: ");
  }

  /**
   * Función que simula a un cliente.
   * El cliente está constantemente entrando y saliendo de la barbería.
   */
  public void run (){
    while (true) {
      barberia.cortarPelo (); // el cliente espera (si procede) y se corta el pelo
      aux.dormir_max( 2000 ); // el cliente está fuera de la barberia un tiempo
    }
  }
}

/*********************************************************************/

class aux{
  static Random rnd = new Random();

  static void dormir_max(int max){
    try {
      Thread.sleep(rnd.nextInt(max));
    }catch (InterruptedException e) {
        System.err.println("Sleep interrumpido\n");
    }
  }

  static public void mensaje(String str){
    System.out.println( Thread.currentThread().getName()+str);
  }
}

/*********************************************************************/

class Barbero implements Runnable{
  public Thread thr;
  private Barberia barberia;

  public Barbero(Barberia laBarberia){
    barberia       = laBarberia;
    thr            = new Thread(this, "Barbero [?]: ");
  }

  public void run(){
    while(true){
      barberia.siguienteCliente ();
      aux.dormir_max( 2500 ); // el barbero está cortando el pelo
      barberia.finCliente ();
    }
  }
}

/*********************************************************************/

class MainBarberia{
  public static void main(String [] args)throws InterruptedException {
    int           numClientes   =   5;
    Barberia      barberia      =   new Barberia();
    Cliente   []  clientes      =   new Cliente[numClientes];
    Barbero       barbero       =   new Barbero(barberia);

    System.out.println("\n/************************************\\");
    System.out.println("/************  BARBERÍA  ************\\");
    System.out.println("/************************************\\ \n");

    for (int i = 0; i< numClientes; i++)
      clientes[i] = new Cliente(barberia, i);

    barbero.thr.start();
    for (int i = 0; i<numClientes; i++)
      clientes[i].thr.start();

    barbero.thr.join();
    for (int i = 0; i < numClientes; i++) {
      clientes[i].thr.join();
    }

    System.out.println("/************************************\\");
    System.out.println("/**************  FIN  ***************\\");
    System.out.println("/************************************\\");
  }
}

/*********************************************************************/
