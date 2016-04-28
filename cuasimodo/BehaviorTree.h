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

class Comportamiento
{
public:
  Comportamiento(const char* name)
  :	name(name), estado(BH_INVALIDO)
  {
  }

  Comportamiento(const char* nombre, Tarea tarea)
  :	tarea(tarea), nombre(nombre), estado(BH_INVALIDO)
  {
  }

	virtual ~Comportamiento() {}

  virtual Estado hacer(Memoria memoria) { return tarea(memoria); }
  virtual void entrar() {}
	virtual void iniciar() {}
	Estado tick(Memoria memoria) {
    entrar();
    if (estado != BH_EJECUTANDO)
      iniciar();

    estado = hacerTarea(memoria);

    if (estado != BH_EJECUTANDO)
      finalizar(estado);
    salir(estado);
    return estado;
  }
  virtual void finalizar(Estado) {}
  virtual void salir(Estado) {}

protected:
	Tarea tarea;
  Estado estado;
  const char* nombre;
};

class Compuesto : public Comportamiento
{
public:
  Composite(const char* nombre) :	Comportamiento(nombre) { }
  void aprender(Comportamiento* comportamiento) {
    comportamientos.push_back(comportamiento);
  }
protected:
  typedef std::vector<Comportamiento*> Comportamientos;
  Comportamientos comportamientos;
};

class Secuencia : public Compuesto {
public:
  Secuencia(const char* nombre) :	Compuesto(nombre) { }

  virtual void iniciar() {
    actual = comportamientos.begin();
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
      if (++actual == comportamientos.end()) {
        return BH_EXITO;
      }
    }
  }
  Comportamiento::iterator actual;
};

class Selector : public Compuesto {
public:
  Selector(const char* nombre) :	Compuesto(nombre) { }

  virtual void iniciar() {
    actual = comportamientos.begin();
  }

  virtual Status hacer(Memoria memoria) {
    // Ejecutar hasta que un comportamiento diga que esta corriendo
  	for (;;) {
      Estado e = (*actual)->tick(memoria);

      // Si un comportamiento esta corriendo o fue exitoso hacer lo mismo
      if (e != BH_FALLO) {
          return e;
      }

      // Hit the end of the array, it didn't end well...
      if (++actual == comportamientos.end()) {
          return BH_FALLO;
      }
    }
  }
  Comportamientos::iterator actual;
};

class Cerebro {
public:
  Cerebro(const char* nombre, Comportamiento comportamiento) :
    nombre(nombre), comportamiento(comportamiento) { }

  void actuar(Memoria memoria) {
    estado = comportamiento.tick(memoria);
  }
protected:
  Comportamiento comportamiento;
  Estado estado;
  const char* nombre;
};
#endif
