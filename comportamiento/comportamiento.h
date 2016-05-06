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
  BH_ABORTADO,
};

typedef struct Memoria {
  unsigned long *estimulos, *nodos;
} Memoria;

typedef Estado (*Tarea)(Memoria);
typedef Estado (*Ver)(int, Memoria);

class Nodo {
public:
  Nodo(const char* nombre):
  nombre(nombre), estado(BH_INVALIDO) {
  }

  virtual ~Nodo() {
  }

  virtual Estado actualizar(Memoria memoria) = 0;
  virtual void entrar() {
  }
  virtual void iniciar() {
  }
  Estado tick(Memoria memoria) {
    entrar();
    if (estado != BH_CORRIENDO) {
      iniciar();
    }

    estado = actualizar(memoria);

    if (estado != BH_CORRIENDO) {
      finalizar(estado);
    }
    salir(estado);
    return estado;
  }
  virtual void finalizar(Estado) {
  }
  virtual void salir(Estado) {
  }
  bool estaFinalizado() const
  {
    return estado == BH_EXITO  ||  estado == BH_FALLO;
  }
  bool estaCorriendo() const
  {
    return estado == BH_CORRIENDO;
  }
  void reset() {
    estado = BH_INVALIDO;
  }
  void abortar()
  {
    finalizar(BH_ABORTADO);
    estado = BH_ABORTADO;
  }
  Estado leerEstado() const
  {
    return estado;
  }
protected:
  Estado estado;
  const char* nombre;
};

class Accion : public Nodo {
public:
  Accion(const char* nombre, Tarea tarea):
  Nodo(nombre), tarea(tarea) {
  }

  virtual ~Accion() {
  }

  virtual Estado actualizar(Memoria memoria) {
    return tarea(memoria);
  }

protected:
  Tarea tarea;
};

class Mirar : public Nodo {
public:
  Mirar(const char* nombre, int indice, Ver ver):
  Nodo(nombre), indice(indice), ver(ver) {
  }

  virtual ~Mirar() {
  }

  virtual Estado actualizar(Memoria memoria) {
    return ver(indice, memoria);
  }

protected:
  int indice;
  Ver ver;
};

class Compuesto : public Nodo {
public:
  Compuesto(const char* nombre) :
  Nodo(nombre) {
  }
  void agregarHijo(Nodo* hijo) {
    hijos.push_back(hijo);
  }
protected:
  typedef std::vector<Nodo*> Nodos;
  Nodos hijos;
};

class Secuencia : public Compuesto {
public:
  Secuencia() :
  Compuesto("Secuencia") {
  }

  virtual void iniciar() {
    actual = hijos.begin();
  }

  virtual Estado actualizar(Memoria memoria)
  {
    // Ejecutar hasta que un comportamiento diga que esta corriendo
    for (;;) {
      Estado e = (*actual)->tick(memoria);
      // Si un comportamiento esta corriendo o fallo hace lo mismo
      if (e != BH_EXITO) {
        return e;
      }

      // Hit the end of the array, job done!
      if (++actual == hijos.end()) {
        return BH_EXITO;
      }
    }
  }
  Nodos::iterator actual;
};

class Selector : public Compuesto {
public:
  Selector() :
  Compuesto("Selector") {
  }

  virtual void iniciar() {
    actual = hijos.begin();
  }

  virtual Estado actualizar(Memoria memoria) {
    // Ejecutar hasta que un comportamiento diga que esta corriendo
    for (;;) {
      Estado e = (*actual)->tick(memoria);

      // Si un comportamiento esta corriendo o fue exitoso hace lo mismo
      if (e != BH_FALLO) {
        return e;
      }

      // Hit the end of the array, it didn't end well...
      if (++actual == hijos.end()) {
        return BH_FALLO;
      }
    }
  }
  Nodos::iterator actual;
};

class Paralelo : public Compuesto
{
public:
  enum Politica
  {
    RequiereUno,
    RequiereTodos,
  };

  Paralelo(Politica paraExito, Politica paraFallo)
  :	Compuesto("Paralelo"), politicaExito(paraExito),	politicaFallo(paraFallo)
  {
  }

  virtual ~Paralelo() {}

protected:
  Politica politicaExito;
  Politica politicaFallo;

  virtual Estado actualizar(Memoria memoria)
  {
    size_t iContadorExito = 0, iContadorFallo = 0;

    for (Nodos::iterator it = hijos.begin(); it != hijos.end(); ++it)
    {
      Nodo& b = **it;
      if (!b.estaFinalizado())
      {
        b.tick(memoria);
      }

      if (b.leerEstado() == BH_EXITO)
      {
        ++iContadorExito;
        if (politicaExito == RequiereUno)
        {
          return BH_EXITO;
        }
      }

      if (b.leerEstado() == BH_FALLO)
      {
        ++iContadorFallo;
        if (politicaFallo == RequiereUno)
        {
          return BH_FALLO;
        }
      }
    }

    if (politicaFallo == RequiereTodos  &&  iContadorFallo == hijos.size())
    {
      return BH_FALLO;
    }

    if (politicaExito == RequiereTodos  &&  iContadorExito == hijos.size())
    {
      return BH_EXITO;
    }

    return BH_FALLO;
  }

  virtual void finalizar(Estado)
  {
    for (Nodos::iterator it = hijos.begin(); it != hijos.end(); ++it)
    {
      Nodo& b = **it;
      if (b.estaCorriendo())
      {
        b.abortar();
      }
    }
  }
};

class Decorador : public Nodo {
protected:
  Nodo* hijo;

public:
  Decorador(Nodo* hijo) :
  Nodo("Decorador"), hijo(hijo) {
  }
};

class Repetir : public Decorador {
public:
  Repetir(Nodo* hijo, int cantidad):
  Decorador(hijo), limite(cantidad)
  {
  }

  void iniciar()
  {
    contador = 0;
  }

  virtual Estado actualizar(Memoria memoria)
  {
    for (;;)
    {
      hijo->tick(memoria);
      if (hijo->leerEstado() == BH_CORRIENDO) break;
      if (hijo->leerEstado() == BH_FALLO) return BH_FALLO;
      if (++contador == limite) return BH_EXITO;
      hijo->reset();
    }
    return BH_INVALIDO;
  }

protected:
  int limite;
  int contador;
};

class Comportamiento : public Selector {
public:
  Comportamiento() :
  Selector() {
  }

  Estado actuar(Memoria memoria) {
    return tick(memoria);
  }
};
#endif
