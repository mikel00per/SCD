import monitor.*;
// ****************************************************************************

/**
 * Clase Buffer.
 * Clase que implementa un buffer de dobles con funciones de
 * extracción e inserción de datos que funciona como monitor para
 * los procesos que extraigan e inserten en el buffer.
 *
 */
class Buffer extends AbstractMonitor {
  // De esta forma marcaremos cuando podemos depositar o cuando no en función
  // de una serie de condiciones que prefijaremos.
  private Condition puedo_poner = makeCondition();
  private Condition puedo_quitar = makeCondition();

  private int tam = 0;  // Tam de memooria que reservamos
  private int ocupados = 0; // Posicion hasta donde está ocupado
  private double[] buffer = null;  // Vector dinamico

  /**
   * Constructor por parámentros de la clase
   * @param tam tamaño a reservar que necesitamos
   */
  public Buffer(int tamBuffer){
    tam = tamBuffer;
    buffer = new double [tam];
  }

  /**
   * Función que añade un valor al buffer.
   * @param valor que se quiere introducir
   *
   * Funcionalmente lo que hacemos es esperar cuando el buffer
   * está lleno. Sino introducimos el valor, incrementamos los ocupados
   * variable que sabemos que es compartida por las hebras pero gracias
   * a enter() garantizamos que se hará en exlusión posteriormente
   * liberará una consumición.
   */
  public void depositar(double valor) throws InterruptedException{
    enter();

    if (tam == ocupados)
      puedo_poner.await();

    buffer[ocupados] = valor;
    ocupados++;
    puedo_quitar.signal();

    leave();
  }

  /**
   * Funcion que elimina un valor del buffer
   * @return valor extraidos.
   * Funcionalmente sigue la misma filosofía que poner un dato, la
   * única diferencia que no puedo quitar nada si el vector está
   * vacío por ello se realiza la comprobación de si los ocupados=0.
   */
  public double extraer() throws InterruptedException{

    enter();
    double valor;
    if (ocupados == 0)
      puedo_quitar.await();

    ocupados--;
    valor = buffer[ocupados];
    puedo_poner.signal();
    leave();

    return valor;
  }
}

// ****************************************************************************

/**
 * Clase que simula la produción de un dato.
 * @autor Antonio Miguel Morillo Chica
 */
class Productor implements Runnable{
  private Buffer buf                ;
  private int    numeroProducciones ;
  private int    idHebra            ;
  public  Thread thr                ;

  /**
   * Constructor por parámetros.
   * @param   newBuf buffer donde guardará datos.
   * @param   nProd numero de producciones que se harán
   * @param   id identificador de la hebra Runnable
   * @return  devuelve el objeto implicito
   */
  public Productor( Buffer newBuf, int nProd, int id ){
    buf                = newBuf;
    numeroProducciones = nProd;
    idHebra            = id;
    thr                = new Thread(this,"Hebra Productora  [" +idHebra+ "]");
  }

  /**
   * Ejecución de la hebra productora
   */
  public void run(){
    try{
      double item = 100*idHebra ;

      for( int i=0 ; i < numeroProducciones ; i++ ){
        System.out.println(thr.getName()+" -> produce [" + item + "]");
        buf.depositar( item++ );
      }
    }
    catch( Exception e ){
      System.err.println("Excepcion en main: " + e);
    }
  }
}

// ****************************************************************************

/**
 * Clase que simula a un consumidor.
 * No se especifica el nombre de la hebra que simulará a
 * un consumidor pues lo que se hará será preguntar a la
 * propia clase que hebra está actuando como un consumidor.
 * @autor Antonio Miguel Morillo Chica
 */
class Consumidor implements Runnable{
  private Buffer  buf            ;
  private int     nConsumiciones ;
  private int     idHebra        ;
  public  Thread  thr            ;

  /**
   * Constructor por defecto de la clase.
   * @param   newBuf buffer que se usará para realizar lecturas.
   * @param   nCons numero de lecturas a consumir
   * @param   id identificador de la hebra
   * @return  devuelve el objeto implicito.
   */
  public Consumidor( Buffer newBuf, int nCons, int id ){
    buf            = newBuf;
    nConsumiciones = nCons;
    idHebra        = id ;
    thr            = new Thread(this,"Hebra Consumidora ["+idHebra+"]");
  }

  /**
   * Ejecución de la hebra productora.
   */
  public void run(){
    try{
      for( int i=0 ; i<nConsumiciones ; i++ ){
        double item = buf.extraer ();
        System.out.println(thr.getName()+" -> consume ["+item+"]");
      }
    }
    catch( Exception e ){
      System.err.println("Excepcion en main: " + e);
    }
  }
}

// ****************************************************************************

/**
 * Clase que simula la comunicación entre productores y consumidores.
 * @autor Antonio Miguel Morillo Chica
 */
class MainProductorConsumidor {
  public static void main(String[] args) {
    if (args.length != 5) {
      System.err.println("USO: <nProductores> <nConsumidores> <tamBuffer> <numProducciones> <numConsumiciones>");
      return;
    }

    // Parseo de argumentos
    Productor[]  productoras   = new Productor[Integer.parseInt(args[0])];
    Consumidor[] consumidoras  = new Consumidor[Integer.parseInt(args[1])];
    Buffer     buffer          = new Buffer(Integer.parseInt(args[2]));
    int        iProducciones   = Integer.parseInt(args[3]);
    int        iConsumiciones  = Integer.parseInt(args[4]);

    if (productoras.length*iProducciones != consumidoras.length*iConsumiciones ){
      System.err.println("No coinciden número de items a producir con a cosumir\n");
      return ;
    }

    // Inicializo las hebras consumidoras a la vez que las creo.
    for (int i = 0 ; i<consumidoras.length; i++) {
      consumidoras[i] = new Consumidor(buffer, iConsumiciones, i);
    }
    // Inicializo las hebras productoras a la vez que las creo
    for (int i = 0 ; i<productoras.length; i++) {
      productoras[i] = new Productor(buffer, iProducciones, i);
    }
    // Inicializo las hebras productoras a la vez que las creo
    for (int i = 0 ; i<productoras.length; i++) {
      productoras[i].thr.start();
    }
    // Inicializo las hebras consumidoras a la vez que las creo.
    for (int i = 0 ; i<consumidoras.length; i++) {
      consumidoras[i].thr.start();
    }
  }
}
