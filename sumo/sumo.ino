#include <StandardCplusplus.h>
#include <utility.h>
#include <system_configuration.h>
#include <unwind-cxx.h>
#include <DCMotor.h>
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
Comportamiento* comportamiento = new Comportamiento("N6 Sumo");

unsigned long sensores[3] = {0};
unsigned long comportamientos[100] = {0};

Memoria memoria = {sensores, comportamientos};

void setup(){

  Serial.begin(9600);

  //Motores
  motor1.setClockwise(false);
  motor2.setClockwise(false);

  // Acciones del comportamiento
  Accion* a_avanzar = new Accion("Avanzar", f_avanzar);
  Accion* a_evitar = new Accion("Evitar", f_evitar);
  Accion* a_girar = new Accion("Rotar", f_girar);
  Accion* a_retroceder = new Accion("Rotar", f_retroceder);
  Mirar* m_s0b = new Mirar("S0 blanco", 0, f_blanco);
  Mirar* m_s1b = new Mirar("S1 blanco", 1, f_blanco);
  Mirar* m_s0n = new Mirar("S1 negro", 0, f_negro);
  Mirar* m_s1n = new Mirar("S1 negro", 1, f_negro);

  // Comportamientos
  // Evitar la linea
  // evitar linea := blanco o blanco => evitar
  Secuencia* c_evitar = new Secuencia("Evitar linea");
  Selector* m_blanco = new Selector("Mirar blanco");
  c_blanco->aprender(m_s0b);
  c_blanco->aprender(m_s1b);
  c_evitar->aprender(m_blanco);
  c_evitar->aprender(a_evitar);
  comportamiento->aprender(c_evitar);

  // Buscar oponente
  // buscar oponente := !oponente => buscar
  Secuencia* c_buscar = new Secuencia("Buscar oponente");
  c_girar_der->aprender(m_oponente);  //Negar esto o hacer algo
  c_girar_der->aprender(a_buscar);
  comportamiento->aprender(c_buscar);

  // Atacar
  // atacar := oponente => avanzar
  Secuencia* c_atacar = new Secuencia("Atacar");
  c_girar_izq->aprender(m_oponente);
  c_girar_izq->aprender(a_avanzar);
  comportamiento->aprender(c_atacar);

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

Estado f_evitar(Memoria memoria) {
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
