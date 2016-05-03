#include <StandardCplusplus.h>
#include <utility.h>
#include <system_configuration.h>
#include <unwind-cxx.h>
#include <DCMotor.h>
#include <FuzzyRule.h>
#include <FuzzyComposition.h>
#include <Fuzzy.h>
#include <FuzzyRuleConsequent.h>
#include <FuzzyOutput.h>
#include <FuzzyInput.h>
#include <FuzzyIO.h>
#include <FuzzySet.h>
#include <FuzzyRuleAntecedent.h>
#include "BehaviorTree.h"

#define izquierdaIR A0
#define derechaIR A1
#define pingPin A5
#define IR_MIN = 430;
#define IR_MAX = 990;
#define IR_CORTE = (IR_MIN + IR_MAX) / 2;
#define MOTOR_MIN = 10;
#define MOTOR_MAX = 100;

DCMotor motor1(M0_EN, M0_D0, M0_D1);
DCMotor motor2(M1_EN, M1_D0, M1_D1);

// Instanciando un objeto de biblioteca
Fuzzy* fuzzy = new Fuzzy();
Comportamiento* comportamiento = new Comportamiento("N6 Seguidor de lineas");

unsigned long sensores[10] = {0};
unsigned long comportamientos[10] = {0};

Memoria memoria = {sensores, comportamientos};

void setup(){

  Serial.begin(9600);

  //Motores
  motor1.setClockwise(false);
  motor2.setClockwise(false);

  FuzzyInput* intencidad = new FuzzyInput(1);
  FuzzySet* baja = new FuzzySet(0, 30, 30, 50);
  intencidad->addFuzzySet(baja);
  FuzzySet* alta = new FuzzySet(40, 60, 60, 80);
  intencidad->addFuzzySet(alta);

  fuzzy->addFuzzyInput(intencidad);

  FuzzyOutput* velocidad = new FuzzyOutput(1);
  FuzzySet* lenta = new FuzzySet(0, 10, 10, 20);
  velocidad->addFuzzySet(lenta);
  FuzzySet* rapida = new FuzzySet(10, 20, 30, 40);
  velocidad->addFuzzySet(rapida);

  fuzzy->addFuzzyOutput(velocidad);

  // Si la intensidad es baja => la velocidad es rapida
  FuzzyRuleAntecedent* ifIntensidadBaja = new FuzzyRuleAntecedent();
  ifIntensidadBaja->joinSingle(baja);
  FuzzyRuleConsequent* thenVelocidadRapida = new FuzzyRuleConsequent();
  thenVelocidadRapida->addOutput(rapida);
  // La regla
  FuzzyRule* fuzzyRegla01 = new FuzzyRule(1, ifIntensidadBaja, thenVelocidadRapida);
  fuzzy->addFuzzyRule(fuzzyRegla01);

  // Si la intensidad es alta => la velocidad es baja
  FuzzyRuleAntecedent* ifIntensidadAlta = new FuzzyRuleAntecedent();
  ifIntensidadAlta->joinSingle(alta);
  FuzzyRuleConsequent* thenVelocidadLenta = new FuzzyRuleConsequent();
  thenVelocidadLenta->addOutput(lenta);
  // La regla
  FuzzyRule* fuzzyRegla02 = new FuzzyRule(3, ifIntensidadAlta, thenVelocidadLenta);
  fuzzy->addFuzzyRule(fuzzyRegla02);

  // Acciones del comportamiento
  Accion* a_avanzar = new Accion("Avanzar", f_avanzar);
  Accion* a_girar = new Accion("Rotar", f_girar);
  Accion* a_retroceder = new Accion("Rotar", f_retroceder);
  Mirar* m_s0b = new Mirar("S0 blanco", 0, f_blanco);
  Mirar* m_s1b = new Mirar("S1 blanco", 1, f_blanco);
  Mirar* m_s0n = new Mirar("S1 negro", 0, f_negro);
  Mirar* m_s1n = new Mirar("S1 negro", 1, f_negro);

  // Comportamientos
  // Seguir la linea
  // seguir := blanco y blanco => avanzar
  Secuencia* c_seguir = new Secuencia("Seguir linea");
  seguir->aprender(m_s0b);
  seguir->aprender(m_s1b);
  seguir->aprender(a_avanzar);
  comportamiento->aprender(c_seguir);

  // Girar derecha
  // girar derecha := negro y blanco => girar
  Secuencia* c_girar_der = new Secuencia("Girar derecha");
  c_girar_der->aprender(m_s0n);
  c_girar_der->aprender(m_s1b);
  c_girar_der->aprender(a_girar);
  comportamiento->aprender(c_girar_der);

  // Girar izquierda
  // girar izquierda := blanco y negro => girar
  Secuencia* c_girar_izq = new Secuencia("Girar izquierda");
  c_girar_izq->aprender(m_s0b);
  c_girar_izq->aprender(m_s1n);
  c_girar_izq->aprender(a_girar);
  comportamiento->aprender(c_girar_izq);

  // Retroceder
  // retroceder := negro y negro => retroceder
  Secuencia* c_retroceder = new Secuencia("Retroceder");
  c_retroceder->aprender(m_s0n);
  c_retroceder->aprender(m_s1n);
  c_retroceder->aprender(a_retroceder);
  comportamiento->aprender(c_retroceder);

  Serial.println(" === END SETUP ===");
}

void loop(){
  // El seguidor de lineas que lee distancia al pedo :P
  memoria.sensores[0] = analogRead(izquierdaIR);
  memoria.sensores[1] = analogRead(derechaIR);
  memoria.sensores[2] = leerDistancia(pingPin);

  // wait 100 milli seconds before looping again
  comportamiento->actuar(memoria);
  delay(100);
}

Estado f_blanco(int indice, Memoria memoria) {
  return memoria.sensores[indice] > CORTE? BH_EXITO : BH_FALLO;
}

Estado f_negro(int indice, Memoria memoria) {
  return memoria.sensores[indice] < CORTE? BH_EXITO : BH_FALLO;
}

Estado f_avanzar(Memoria memoria) {
  Serial.println("Avanzando");
  Serial.print(MOTOR_MAX);
  Serial.print(" ");
  Serial.println(MOTOR_MAX);
  //motor1.setSpeed(MOTOR_MAX);
  //motor2.setSpeed(MOTOR_MAX);
  return BH_CORRIENDO;
}

Estado f_retroceder(Memoria memoria) {
  Serial.print("Re reversa papi ");
  Serial.print(MOTOR_MAX);
  Serial.print(" ");
  Serial.println(MOTOR_MAX);
  //motor1.setSpeed(-MOTOR_MAX);
  //motor2.setSpeed(-MOTOR_MAX);
  return BH_CORRIENDO;
}

Estado f_girar(Memoria memoria) {
  fuzzy->setInput(0, memoria.sensores[0]);
  fuzzy->setInput(1, memoria.sensores[1]);
  fuzzy->fuzzify();
  int m1_speed = fuzzy->defuzzify(1);
  int m2_speed = fuzzy->defuzzify(0);
  Serial.print("Girando ");
  Serial.print(m1_speed);
  Serial.print(" ");
  Serial.println(m2_speed);
  //motor1.setSpeed(m1_speed);
  //motor2.setSpeed(m2_speed);
  return BH_CORRIENDO;
}

long leerDistancia(int pingPin)
{
  int pulseTime = 0;
  // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  pinMode(pingPin, OUTPUT);
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pingPin, LOW);

  // The same pin is used to read the signal from the PING))): a HIGH
  // pulse whose duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(pingPin, INPUT);
  pulseTime = pulseIn(pingPin, HIGH);

  // convert the time into a distance
  return microsecondsToCentimeters(pulseTime);
}

long microsecondsToCentimeters(unsigned long microseconds)
{
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}
