#include <StandardCplusplus.h>
#include <vector>
#include <iterator>

#ifndef BEHAVIORTREE_H
#define BEHAVIORTREE_H

enum Estado
{
  BH_INVALIDO,
	BH_EXITO,
	BH_FALLO,
	BH_CORRIENDO,
};

typedef struct Memoria {int *sensores, *comportamientos;} Memoria;

typedef Estado (*Tarea)(Memoria);

class Nodo
{
public:
  Nodo(const char* name)
  :	name(name), estado(BH_INVALIDO)
  {
  }

	virtual ~Nodo() {}

  virtual Estado hacer(Memoria memoria) { return tarea(memoria); }
  virtual void entrar() {}
	virtual void iniciar() {}
	Estado tick(Memoria memoria) {
    entrar();
    if (estado != BH_EJECUTANDO)
      iniciar();

    estado = hacer(memoria);

    if (estado != BH_EJECUTANDO)
      finalizar(estado);
    salir(estado);
    return estado;
  }
  virtual void finalizar(Estado) {}
  virtual void salir(Estado) {}

protected:
  Estado estado;
  const char* nombre;
};

class Accion
{
public:
  Accion(const char* nombre, Tarea tarea)
  :	Nodo(nombre), tarea(tarea)
  {
  }

	virtual ~Accion() {}

  virtual Estado hacer(Memoria memoria) { return tarea(memoria); }

protected:
	Tarea tarea;
};

class Compuesto : public Nodo
{
public:
  Composite(const char* nombre) :	Nodo(nombre) { }
  void aprender(Comportamiento* comportamiento) {
    comportamientos.push_back(comportamiento);
  }
protected:
  typedef std::vector<Nodos*> Nodos;
  Nodos nodos;
};

class Secuencia : public Compuesto {
public:
  Secuencia(const char* nombre) :	Compuesto(nombre) { }

  virtual void iniciar() {
    actual = nodos.begin();
  }

  virtual Estado hacer(Memoria memoria)
  {
    // Ejecutar hasta que un comportamiento diga que esta corriendo
    for (;;) {
      Estado e = (*actual)->tick(memoria);
      // Si un comportamiento esta corriendo o fallo hacer lo mismo
      if (e != BH_EXITO) {
        return e;
      }

      // Hit the end of the array, job done!
      if (++actual == nodos.end()) {
        return BH_EXITO;
      }
    }
  }
  Nodo::iterator actual;
};

class Selector : public Compuesto {
public:
  Selector(const char* nombre) :	Compuesto(nombre) { }

  virtual void iniciar() {
    actual = nodos.begin();
  }

  virtual Estado hacer(Memoria memoria) {
    // Ejecutar hasta que un comportamiento diga que esta corriendo
  	for (;;) {
      Estado e = (*actual)->tick(memoria);

      // Si un comportamiento esta corriendo o fue exitoso hacer lo mismo
      if (e != BH_FALLO) {
          return e;
      }

      // Hit the end of the array, it didn't end well...
      if (++actual == nodos.end()) {
          return BH_FALLO;
      }
    }
  }
  Nodo::iterator actual;
};

class Comportamiento {
public:
  Comportamiento(const char* nombre, Nodo nodo) :
    nombre(nombre), nodo(nodo) { }

  void actuar(Memoria memoria) {
    estado = nodo.tick(memoria);
  }
protected:
  Nodo nodo;
  Estado estado;
  const char* nombre;
};
#endif
