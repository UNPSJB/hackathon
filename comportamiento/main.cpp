#include <iostream>
#include "comportamiento.h"

using namespace std;

int MAX_IR = 990;
int MIN_IR = 330;
int CORTE_IR = (MAX_IR + MIN_IR) / 2;
int MAX = 90;

unsigned long estimulos[2] = {0};
unsigned long nodos[10] = {0};

Memoria memoria = {estimulos, nodos};

Estado blanco(int indice, Memoria memoria) {
  return memoria.estimulos[indice] > CORTE_IR? BH_EXITO : BH_FALLO;
}

Estado negro(int indice, Memoria memoria) {
  return memoria.estimulos[indice] < CORTE_IR? BH_EXITO : BH_FALLO;
}

Estado f_avanzar(Memoria memoria) {
  cout << "Avanzando" << endl;
  return BH_EXITO;
}

Estado f_girar(Memoria memoria) {
  cout << "Girando" << " " << memoria.estimulos[0] << " " << memoria.estimulos[1] << endl;
  return BH_EXITO;
}

int main(int argc, char const *argv[]) {
  Comportamiento* comportamiento = new Comportamiento();

  // Acciones
  Accion* a_avanzar = new Accion("Avanzar", f_avanzar);
  Accion* a_girar = new Accion("Rotar", f_girar);
  Mirar* s0b = new Mirar("s0blanco", 0, blanco);
  Mirar* s1b = new Mirar("s1blanco", 1, blanco);
  Mirar* s0n = new Mirar("s0negro", 0, negro);
  Mirar* s1n = new Mirar("s1negro", 1, negro);

  // seguir := s0blanco y s1blanco => avanzar
  Secuencia* seguir = new Secuencia();
  seguir->agregarHijo(s0b);
  seguir->agregarHijo(s1b);
  seguir->agregarHijo(a_avanzar);
  comportamiento->agregarHijo(seguir);

	// girar derecha := s0negro y s1blanco => girar
  Secuencia* girar_der = new Secuencia();
  girar_der->agregarHijo(s0n);
  girar_der->agregarHijo(s1b);
  girar_der->agregarHijo(a_girar);
  comportamiento->agregarHijo(girar_der);

	// girar izquierda := s0blanco y s1negro => girar
  Secuencia* girar_izq = new Secuencia();
  girar_izq->agregarHijo(s0b);
  girar_izq->agregarHijo(s1n);
  girar_izq->agregarHijo(a_girar);
  comportamiento->agregarHijo(girar_izq);

	memoria.estimulos[0] = 700;
	memoria.estimulos[1] = 700;
  comportamiento->actuar(memoria);

	memoria.estimulos[0] = 400;
	memoria.estimulos[1] = 700;
  comportamiento->actuar(memoria);

	memoria.estimulos[0] = 700;
	memoria.estimulos[1] = 400;
  comportamiento->actuar(memoria);

	memoria.estimulos[0] = 400;
	memoria.estimulos[1] = 400;
  comportamiento->actuar(memoria);
	return 0;
}
