import monitor.*;
import java.util.Random;

class Estanco extends AbstractMonitor{
  private int ingredienteActual = -1;
  private Condition estadoFumadores[] = new Condition[3];
  private Condition estadoEstanquero;

  public Estanco(){
    for (int i = 0; i < this.estadoFumadores.length; i++)
      this.estadoFumadores[i] = makeCondition();
    this.estadoEstanquero = makeCondition();
  }

  public void obtenerIngrediente(int miIngrediente){

    enter();
      // No puedo consumir el ingrediente que no haya sido
      // puesto encima de la mesa, así que le hago esperar
      if(ingredienteActual != miIngrediente)
        estadoFumadores[miIngrediente].await();

      if (miIngrediente == 0) {
        System.out.println(Thread.currentThread().getName()+"recoge [Tabaco]");
      }else if (miIngrediente == 1) {
        System.out.println(Thread.currentThread().getName()+"recoge [Papel]");
      }else
        System.out.println(Thread.currentThread().getName()+"recoge [Cerillas]");

      // Quito el ingrediente de la mesa y produzco!
      ingredienteActual = -1;
      estadoEstanquero.signal();

    leave();
  }

  public void ponerIngrediente(int ingrediente){

    enter();
      if (ingrediente == 0) {
        System.out.println(Thread.currentThread().getName()+"pone [Tabaco]");
      }else if (ingrediente == 1) {
        System.out.println(Thread.currentThread().getName()+"pone [Papel]");
      }else
        System.out.println(Thread.currentThread().getName()+"pone [Cerillas]");

      // Lo pongo encima de la mesa y llamo al fumador
      ingredienteActual = ingrediente;
      estadoFumadores[ingrediente].signal();
    leave();
  }

  public void esperarRecogidaIngrediente(){
    enter();
      if (ingredienteActual != -1) {
        estadoEstanquero.await();
      }
    leave();
  }
}

/*********************************************************************/

class Fumador implements Runnable{
  private int miIngrediente;
  private Estanco estanco;
  public Thread thr;

  public Fumador(Estanco newEstanco, int p_miIngrediente){
    miIngrediente = p_miIngrediente;
    estanco       = newEstanco;
    thr           = new Thread(this, "Fumador ["+miIngrediente+"] ");
  }

  public void run(){
    while (true){
      estanco.obtenerIngrediente( miIngrediente );
      aux.dormir_max( 2000 );
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
}

/*********************************************************************/

class Estanquero implements Runnable{
  public Thread thr;
  private Estanco estanco;

  public Estanquero(Estanco elEstanco){
    estanco       = elEstanco;
    thr           = new Thread(this, "Estanquero :D ");
  }

  public void run(){
    int ingrediente;
    while(true){
      ingrediente = (int) (Math.random () * 3.0); // 0,1 o 2
      estanco.ponerIngrediente(ingrediente);
      estanco.esperarRecogidaIngrediente();
    }
  }
}

/*********************************************************************/

class MainFumadores{
  public static void main(String [] args){
    Estanco         estanco     = new Estanco();
    Fumador     []  fumadores   = new Fumador[3];
    Estanquero      estanquero  = new Estanquero(estanco);

    for (int i = 0; i < fumadores.length; i++) {
      fumadores[i] = new Fumador(estanco, i);
    }

    estanquero.thr.start();
    for (int i = 0; i < fumadores.length; i++) {
      fumadores[i].thr.start();
    }
  }
}

/*********************************************************************/
