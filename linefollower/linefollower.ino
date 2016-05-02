#include <StandardCplusplus.h>
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

int pingPin = 15;
unsigned long pulseTime = 0;

DCMotor motor1(M0_EN, M0_D0, M0_D1);
DCMotor motor2(M1_EN, M1_D0, M1_D1);

// Instanciando um objeto da biblioteca
Fuzzy* fuzzy = new Fuzzy();
Comportamiento comportamiento("N6 Seguidor de lineas");

int BLANCO = 80;
int MAX = 90;

int sensores[2] = {100, 50};
int comportamientos[1] = {0};

Memoria memoria = {sensores, comportamientos};

Estado blanco(int indice, Memoria memoria) {
  cout << "Es blanco" << endl;
  return memoria.sensores[indice] < BLANCO? BH_EXITO : BH_FALLO;
}

Estado negro(int indice, Memoria memoria) {
  cout << "Es negro" << endl;
  return memoria.sensores[indice] > BLANCO? BH_EXITO : BH_FALLO;
}

Estado traccionar(Memoria memoria) {
  Serial.println("Avanzando");
  motor1.setSpeed(MAX);
  motor2.setSpeed(MAX);
  return BH_CORRIENDO;
}

Estado rotar(Memoria memoria) {
  Serial.println("Rotar");
  fuzzy->setInput(0, memoria.sensores[0]);
  fuzzy->setInput(1, memoria.sensores[1]);
  fuzzy->fuzzify();
  motor1.setSpeed(fuzzy->defuzzify(1));
  motor2.setSpeed(fuzzy->defuzzify(0));
  return BH_CORRIENDO;
}

void setup(){

  Serial.begin(9600);

  //Motores
  motor1.setClockwise(false);
  motor2.setClockwise(false);

  FuzzyInput intencidad(1);
  FuzzySet small(0, 30, 30, 50);
  distance.addFuzzySet(&small);
  FuzzySet safe(40, 60, 60, 80);
  distance.addFuzzySet(&safe);
  FuzzySet big(60, 80, 80, 100);
  distance.addFuzzySet(&big);

  fuzzy.addFuzzyInput(intencidad);

  FuzzyOutput velocidad(1);
  FuzzySet slow(0, 10, 10, 20);
  velocity.addFuzzySet(&slow);
  FuzzySet average(10, 20, 30, 40);
  velocity.addFuzzySet(&average);
  FuzzySet fast(30, 40, 40, 50);
  velocity.addFuzzySet(fast);

  fuzzy.addFuzzyOutput(velocidad);

  // FuzzyRule "SE distancia = pequena ENTAO velocidade = lenta"
  FuzzyRuleAntecedent ifDistanceSmall(); // Instanciando um Antecedente para a expresso
  ifDistanceSmall.joinSingle(&small); // Adicionando o FuzzySet correspondente ao objeto Antecedente
  FuzzyRuleConsequent thenVelocitySlow(); // Instancinado um Consequente para a expressao
  thenVelocitySlow.addOutput(&slow);// Adicionando o FuzzySet correspondente ao objeto Consequente
  // Instanciando um objeto FuzzyRule
  FuzzyRule fuzzyRule01(1, &ifDistanceSmall, &thenVelocitySlow); // Passando o Antecedente e o Consequente da expressao
  fuzzy.addFuzzyRule(&fuzzyRule01); // Adicionando o FuzzyRule ao objeto Fuzzy

  // FuzzyRule "SE distancia = segura ENTAO velocidade = normal"
  FuzzyRuleAntecedent ifDistanceSafe(); // Instanciando um Antecedente para a expresso
  ifDistanceSafe.joinSingle(&safe); // Adicionando o FuzzySet correspondente ao objeto Antecedente
  FuzzyRuleConsequent thenVelocityAverage(); // Instancinado um Consequente para a expressao
  thenVelocityAverage.addOutput(&average);// Adicionando o FuzzySet correspondente ao objeto Consequente
  // Instanciando um objeto FuzzyRule
  FuzzyRule fuzzyRule02(2, &ifDistanceSafe, &thenVelocityAverage); // Passando o Antecedente e o Consequente da expressao
  fuzzy.addFuzzyRule(&fuzzyRule02); // Adicionando o FuzzyRule ao objeto Fuzzy

  // FuzzyRule "SE distancia = grande ENTAO velocidade = alta"
  FuzzyRuleAntecedent ifDistanceBig(); // Instanciando um Antecedente para a expresso
  ifDistanceBig.joinSingle(&big); // Adicionando o FuzzySet correspondente ao objeto Antecedente
  FuzzyRuleConsequent thenVelocityFast(); // Instancinado um Consequente para a expressao
  thenVelocityFast.addOutput(&fast);// Adicionando o FuzzySet correspondente ao objeto Consequente
  // Instanciando um objeto FuzzyRule
  FuzzyRule fuzzyRule03(3, &ifDistanceBig, &thenVelocityFast); // Passando o Antecedente e o Consequente da expressao
  fuzzy.addFuzzyRule(&fuzzyRule03); // Adicionando o FuzzyRule ao objeto Fuzzy

  // Acciones
  Accion avanzar("Avanzar", traccionar);
  Accion girar("Rotar", rotar);
  Mirar s0b("S0 blanco", 0, blanco);
  Mirar s1b("S1 blanco", 1, blanco);
  Mirar s0n("S1 negro", 0, negro);
  Mirar s1n("S1 negro", 1, negro);

  // Seguir la linea
  Secuencia seguir("Seguir linea");
  seguir.aprender(&s0b);
  seguir.aprender(&s1b);
  seguir.aprender(&avanzar);
  comportamiento.aprender(&seguir);

  Secuencia girar_der("Girar derecha");
  girar_der.aprender(&s0n);
  girar_der.aprender(&s1b);
  girar_der.aprender(&girar);
  comportamiento.aprender(&girar_der);

  Secuencia girar_izq("Girar izquierda");
  girar_izq.aprender(&s1n);
  girar_izq.aprender(&s0b);
  girar_izq.aprender(&girar);
  comportamiento.aprender(&girar_izq);
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
  memoria.sensores[2] = microsecondsToCentimeters(pulseTime);

  fuzzy.setInput(1, memoria.sensores[2]);

  fuzzy.fuzzify();

  float output = fuzzy.defuzzify(1);
  Serial.print(memoria.sensores[2]);
  Serial.print(" ");
  Serial.println(output);
  // wait 100 milli seconds before looping again
  comportamiento.actuar(memoria);
  delay(100);
}

long microsecondsToCentimeters(long microseconds)
{
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}