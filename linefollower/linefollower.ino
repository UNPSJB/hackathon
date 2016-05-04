#include <StandardCplusplus.h>
#include <utility.h>
#include <system_configuration.h>
#include <unwind-cxx.h>
#include <DCMotor.h>
#include "BehaviorTree.h"

#define izquierdaIR A0
#define derechaIR A1
#define pingPin A5
#define IR_MIN 330
#define IR_MAX 990
#define IR_CORTE ((IR_MIN + IR_MAX) / 2)
#define MOTOR_MIN 20
#define MOTOR_MAX 70

DCMotor motor1(M0_EN, M0_D0, M0_D1);
DCMotor motor2(M1_EN, M1_D0, M1_D1);

Comportamiento* comportamiento = new Comportamiento("N6 Seguidor de lineas");

unsigned long sensores[10] = {0};
unsigned long comportamientos[10] = {0};

Memoria memoria = {sensores, comportamientos};

void setup(){

  Serial.begin(9600);

  //Motores
  motor1.setClockwise(true);
  motor2.setClockwise(true);
  memoria.comportamientos[0] = MOTOR_MAX;
  memoria.comportamientos[1] = MOTOR_MAX;

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
  c_seguir->aprender(m_s0b);
  c_seguir->aprender(m_s1b);
  c_seguir->aprender(a_avanzar);
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

  //Serial.println(" === END SETUP ===");
}



void loop(){
  // El seguidor de lineas que lee distancia al pedo :P
  memoria.sensores[0] = analogRead(izquierdaIR);
  memoria.sensores[1] = analogRead(derechaIR);
  /*
  Serial.print(memoria.sensores[0]);
  Serial.print(" ");
  Serial.println(memoria.sensores[1]);
  */
  memoria.sensores[2] = 0;//leerDistancia(pingPin);

  // wait 100 milli seconds before looping again
  comportamiento->actuar(memoria);
  delay(100);
}

Estado f_blanco(int indice, Memoria memoria) {
  return memoria.sensores[indice] > IR_CORTE? BH_EXITO : BH_FALLO;
}

Estado f_negro(int indice, Memoria memoria) {
  return memoria.sensores[indice] < IR_CORTE? BH_EXITO : BH_FALLO;
}

Estado f_avanzar(Memoria memoria) {
  memoria.comportamientos[0] = MOTOR_MAX;
  memoria.comportamientos[1] = MOTOR_MAX;
  motor1.setSpeed(memoria.comportamientos[0]);
  motor2.setSpeed(memoria.comportamientos[1]);
  return BH_EXITO;
  //return BH_CORRIENDO;
}

Estado f_retroceder(Memoria memoria) {
  motor1.setSpeed(memoria.comportamientos[0]);
  motor2.setSpeed(memoria.comportamientos[1]);
  return BH_EXITO;
  //return BH_CORRIENDO;
}

Estado f_girar(Memoria memoria) {
  float m2_ = ((float)memoria.sensores[0] / 1000);
  float m1_ = ((float)memoria.sensores[1] / 1000);
  memoria.comportamientos[0] = memoria.comportamientos[0] * m1_;
  memoria.comportamientos[1] = memoria.comportamientos[1] * m2_;
  motor1.setSpeed(memoria.comportamientos[0]);
  motor2.setSpeed(memoria.comportamientos[1]);
  return BH_EXITO;
  //return BH_CORRIENDO;
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
