#include <StandardCplusplus.h>
#include <utility.h>
#include <system_configuration.h>
#include <unwind-cxx.h>
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

typedef struct Memoria {
  unsigned long *sensores, *comportamientos;
} 
Memoria;

typedef Estado (*Tarea)(Memoria);
typedef Estado (*Ver)(int, Memoria);

class Nodo
{
public:
  Nodo(const char* nombre)
:	
    nombre(nombre), estado(BH_INVALIDO)
    {
    }

  virtual ~Nodo() {
  }

  virtual Estado hacer(Memoria memoria) = 0;
  virtual void entrar() {
  }
  virtual void iniciar() {
  }
  Estado tick(Memoria memoria) {
    entrar();
    if (estado != BH_CORRIENDO)
      iniciar();

    estado = hacer(memoria);

    if (estado != BH_CORRIENDO)
      finalizar(estado);
    salir(estado);
    return estado;
  }
  virtual void finalizar(Estado) {
  }
  virtual void salir(Estado) {
  }
  void reset() { 
    estado = BH_INVALIDO; 
  }
  Estado estado;
protected:
  const char* nombre;
};

class Accion : 
public Nodo
{
public:
  Accion(const char* nombre, Tarea tarea)
:	
    Nodo(nombre), tarea(tarea)
    {
    }

  virtual ~Accion() {
  }

  virtual Estado hacer(Memoria memoria) { 
    return tarea(memoria); 
  }

protected:
  Tarea tarea;
};

class Mirar : 
public Nodo
{
public:
  Mirar(const char* nombre, int indice, Ver ver)
:	
    Nodo(nombre), indice(indice), ver(ver)
    {
    }

  virtual ~Mirar() {
  }

  virtual Estado hacer(Memoria memoria) { 
    return ver(indice, memoria); 
  }

protected:
  int indice;
  Ver ver;
};

class Compuesto : 
public Nodo
{
public:
  Compuesto(const char* nombre) :	
  Nodo(nombre) { 
  }
  void aprender(Nodo* nodo) {
    nodos.push_back(nodo);
  }
protected:
  typedef std::vector<Nodo*> Nodos;
  Nodos nodos;
};

class Secuencia : 
public Compuesto {
public:
  Secuencia(const char* nombre) :	
  Compuesto(nombre) { 
  }

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
  Nodos::iterator actual;
};

class Selector : 
public Compuesto {
public:
  Selector(const char* nombre) :	
  Compuesto(nombre) { 
  }

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
  Nodos::iterator actual;
};

class Decorador : 
public Nodo
{
protected:
  Nodo* m_pChild;

public:
  Decorador(Nodo* child) : 
  Nodo("Decorador"), m_pChild(child) {
  }
};

class Repetir : 
public Decorador
{
public:
  Repetir(Nodo* child, int count)
:	
    Decorador(child), m_iLimit(count)
    {
    }


  void iniciar() 
  {
    m_iCounter = 0;
  }

  virtual Estado hacer(Memoria memoria) 
  {
    for (;;)
    {
      m_pChild->tick(memoria);
      if (m_pChild->estado == BH_CORRIENDO) break;
      if (m_pChild->estado == BH_FALLO) return BH_FALLO;
      if (++m_iCounter == m_iLimit) return BH_EXITO;
      m_pChild->reset();
    }
    return BH_INVALIDO;
  }

protected:
  int m_iLimit;
  int m_iCounter;
};

class Comportamiento : 
public Selector {
public:
  Comportamiento(const char* nombre) :
  Selector(nombre) { 
  }

  Estado actuar(Memoria memoria) {
    return tick(memoria);
  }
};
#endif

