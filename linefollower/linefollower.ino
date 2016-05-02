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


int pulseTime = 0;


DCMotor motor1(M0_EN, M0_D0, M0_D1);  
DCMotor motor2(M1_EN, M1_D0, M1_D1);

// Instanciando un objeto de biblioteca
Fuzzy* fuzzy = new Fuzzy();
Comportamiento* comportamiento = new Comportamiento("N6 Seguidor de lineas");

int NEGRO = 430;
int BLANCO = 990;
int CORTE = (BLANCO + NEGRO) / 2;
int MAX = 90;

unsigned long sensores[10] = {0};
unsigned long comportamientos[10] = {0};

Memoria memoria = {sensores, comportamientos};

void setup(){

  Serial.begin(9600);

  //Motores
  motor1.setClockwise(false);
  motor2.setClockwise(false);

  FuzzyInput* intencidad = new FuzzyInput(1);
  FuzzySet* small = new FuzzySet(0, 30, 30, 50);
  intencidad->addFuzzySet(small);
  FuzzySet* safe = new FuzzySet(40, 60, 60, 80);
  intencidad->addFuzzySet(safe);
  FuzzySet* big = new FuzzySet(60, 80, 80, 100);
  intencidad->addFuzzySet(big);

  fuzzy->addFuzzyInput(intencidad);

  FuzzyOutput* velocidad = new FuzzyOutput(1);
  FuzzySet* slow = new FuzzySet(0, 10, 10, 20);
  velocidad->addFuzzySet(slow);
  FuzzySet* average = new FuzzySet(10, 20, 30, 40);
  velocidad->addFuzzySet(average);
  FuzzySet* fast = new FuzzySet(30, 40, 40, 50);
  velocidad->addFuzzySet(fast);

  fuzzy->addFuzzyOutput(velocidad);

  // FuzzyRule "SE distancia = pequena ENTAO velocidade = lenta"
  FuzzyRuleAntecedent* ifDistanceSmall = new FuzzyRuleAntecedent(); // Instanciando um Antecedente para a expresso
  ifDistanceSmall->joinSingle(small); // Adicionando o FuzzySet correspondente ao objeto Antecedente
  FuzzyRuleConsequent* thenVelocitySlow = new FuzzyRuleConsequent(); // Instancinado um Consequente para a expressao
  thenVelocitySlow->addOutput(slow);// Adicionando o FuzzySet correspondente ao objeto Consequente
  // Instanciando um objeto FuzzyRule
  FuzzyRule* fuzzyRule01 = new FuzzyRule(1, ifDistanceSmall, thenVelocitySlow); // Passando o Antecedente e o Consequente da expressao
  fuzzy->addFuzzyRule(fuzzyRule01); // Adicionando o FuzzyRule ao objeto Fuzzy

  // FuzzyRule "SE distancia = segura ENTAO velocidade = normal"
  FuzzyRuleAntecedent* ifDistanceSafe = new FuzzyRuleAntecedent(); // Instanciando um Antecedente para a expresso
  ifDistanceSafe->joinSingle(safe); // Adicionando o FuzzySet correspondente ao objeto Antecedente
  FuzzyRuleConsequent* thenVelocityAverage = new FuzzyRuleConsequent(); // Instancinado um Consequente para a expressao
  thenVelocityAverage->addOutput(average);// Adicionando o FuzzySet correspondente ao objeto Consequente
  // Instanciando um objeto FuzzyRule
  FuzzyRule* fuzzyRule02 = new FuzzyRule(2, ifDistanceSafe, thenVelocityAverage); // Passando o Antecedente e o Consequente da expressao
  fuzzy->addFuzzyRule(fuzzyRule02); // Adicionando o FuzzyRule ao objeto Fuzzy

  // FuzzyRule "SE distancia = grande ENTAO velocidade = alta"
  FuzzyRuleAntecedent* ifDistanceBig = new FuzzyRuleAntecedent(); // Instanciando um Antecedente para a expresso
  ifDistanceBig->joinSingle(big); // Adicionando o FuzzySet correspondente ao objeto Antecedente
  FuzzyRuleConsequent* thenVelocityFast = new FuzzyRuleConsequent(); // Instancinado um Consequente para a expressao
  thenVelocityFast->addOutput(fast);// Adicionando o FuzzySet correspondente ao objeto Consequente
  // Instanciando um objeto FuzzyRule
  FuzzyRule* fuzzyRule03 = new FuzzyRule(3, ifDistanceBig, thenVelocityFast); // Passando o Antecedente e o Consequente da expressao
  fuzzy->addFuzzyRule(fuzzyRule03); // Adicionando o FuzzyRule ao objeto Fuzzy

  // Acciones
  Accion* avanzar = new Accion("Avanzar", traccionar);
  Accion* girar = new Accion("Rotar", rotar);
  Mirar* s0b = new Mirar("S0 blanco", 0, blanco);
  Mirar* s1b = new Mirar("S1 blanco", 1, blanco);
  Mirar* s0n = new Mirar("S1 negro", 0, negro);
  Mirar* s1n = new Mirar("S1 negro", 1, negro);

  // Seguir la linea
  Secuencia* seguir = new Secuencia("Seguir linea");
  seguir->aprender(s0b);
  seguir->aprender(s1b);
  seguir->aprender(avanzar);
  comportamiento->aprender(seguir);

  Secuencia* girar_der = new Secuencia("Girar derecha");
  girar_der->aprender(s0n);
  girar_der->aprender(s1b);
  girar_der->aprender(girar);
  comportamiento->aprender(girar_der);

  Secuencia* girar_izq = new Secuencia("Girar izquierda");
  girar_izq->aprender(s1n);
  girar_izq->aprender(s0b);
  girar_izq->aprender(girar);
  comportamiento->aprender(girar_izq);
  
  Serial.println(" === END SETUP ===");
  Serial.println(CORTE);
  
}

void loop(){

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
  memoria.sensores[0] = analogRead(izquierdaIR);
  memoria.sensores[1] = analogRead(derechaIR);
  memoria.sensores[2] = microsecondsToCentimeters(pulseTime);
  
  
  fuzzy->setInput(1, memoria.sensores[2]);

  fuzzy->fuzzify();

  float output = fuzzy->defuzzify(1);

  //Serial.print(memoria.sensores[0]);
  //  Serial.print(" ");
  //Serial.print(memoria.sensores[1]);
  //Serial.print(" ");
  //Serial.print(memoria.sensores[2]);
  //Serial.print(" ");
  //Serial.println(output);
  
  // wait 100 milli seconds before looping again
  comportamiento->actuar(memoria);
  delay(100);
}



Estado blanco(int indice, Memoria memoria) {
  return memoria.sensores[indice] > CORTE? BH_EXITO : BH_FALLO;
}

Estado negro(int indice, Memoria memoria) {
  return memoria.sensores[indice] < CORTE? BH_EXITO : BH_FALLO;
}

Estado traccionar(Memoria memoria) {
  Serial.println("Avanzando");
  //motor1.setSpeed(MAX);
  //motor2.setSpeed(MAX);
  return BH_CORRIENDO;
}

Estado rotar(Memoria memoria) {
  Serial.println("Rotar");
  //fuzzy->setInput(0, memoria.sensores[0]);
  //fuzzy->setInput(1, memoria.sensores[1]);
  //fuzzy->fuzzify();
  //motor1.setSpeed(fuzzy->defuzzify(1));
  //motor2.setSpeed(fuzzy->defuzzify(0));
  return BH_CORRIENDO;
}

long microsecondsToCentimeters(unsigned long microseconds)
{
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}

