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
Selector cerebro("Cerebro");

int BLANCO = 80;
int MAX = 90;

int sensores[3] = {0, 0, 0};
int nodos[1] = {0};

Memoria memoria = {sensores, nodos};

Status s0_blanco(Memoria memoria) {
  return memoria.sensores[0] > BLANCO? BH_SUCCESS : BH_FAILURE;
}

Status s1_blanco(Memoria memoria) {
  return memoria.sensores[1] > BLANCO? BH_SUCCESS : BH_FAILURE;
}

Status s0_negro(Memoria memoria) {
  return memoria.sensores[0] < BLANCO? BH_SUCCESS : BH_FAILURE;
}

Status s1_negro(Memoria memoria) {
  return memoria.sensores[1] < BLANCO? BH_SUCCESS : BH_FAILURE;
}

Status traccionar(Memoria memoria) {
  Serial.println("Avanzando");
  motor1.setSpeed(MAX);
  motor2.setSpeed(MAX);
  return BH_SUCCESS;
}

Status rotar(Memoria memoria) {
  Serial.println("Rotar");
  fuzzy->setInput(0, memoria.sensores[0]);
  fuzzy->setInput(1, memoria.sensores[1]);
  fuzzy->fuzzify();
  motor1.setSpeed(fuzzy->defuzzify(1));
  motor2.setSpeed(fuzzy->defuzzify(0));
  return BH_SUCCESS;
}

void setup(){

  Serial.begin(9600);
  
  //Motores
  motor1.setClockwise(false);
  motor2.setClockwise(false);

  // Criando o FuzzyInput distancia
  FuzzyInput* distance = new FuzzyInput(1);
  // Criando os FuzzySet que compoem o FuzzyInput distancia
  FuzzySet* small = new FuzzySet(0, 30, 30, 50); // Distancia pequena
  distance->addFuzzySet(small); // Adicionando o FuzzySet small em distance
  FuzzySet* safe = new FuzzySet(40, 60, 60, 80); // Distancia segura
  distance->addFuzzySet(safe); // Adicionando o FuzzySet safe em distance
  FuzzySet* big = new FuzzySet(60, 80, 80, 100); // Distancia grande
  distance->addFuzzySet(big); // Adicionando o FuzzySet big em distance

  fuzzy->addFuzzyInput(distance); // Adicionando o FuzzyInput no objeto Fuzzy

  // Criando o FuzzyOutput velocidade
  FuzzyOutput* velocity = new FuzzyOutput(1);
  // Criando os FuzzySet que compoem o FuzzyOutput velocidade
  FuzzySet* slow = new FuzzySet(0, 10, 10, 20); // Velocidade lenta
  velocity->addFuzzySet(slow); // Adicionando o FuzzySet slow em velocity
  FuzzySet* average = new FuzzySet(10, 20, 30, 40); // Velocidade normal
  velocity->addFuzzySet(average); // Adicionando o FuzzySet average em velocity
  FuzzySet* fast = new FuzzySet(30, 40, 40, 50); // Velocidade alta
  velocity->addFuzzySet(fast); // Adicionando o FuzzySet fast em velocity

  fuzzy->addFuzzyOutput(velocity); // Adicionando o FuzzyOutput no objeto Fuzzy

  //-------------------- Montando as regras Fuzzy
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

  // El cerebro
  Sequence seguir("Seguir linea");
  Behavior s0b("S0 blanco", s0_blanco);
  Behavior s1b("S1 blanco", s1_blanco);
  Behavior avanzar("Avanzar", traccionar);
  seguir.aprender(&s0b);
  seguir.aprender(&s1b);
  seguir.aprender(&avanzar);
  cerebro.aprender(&seguir);

  Sequence girar_der("Girar derecha");
  Behavior s0n("S0 negro", s0_negro);
  Behavior doblar("Rotar", rotar);
  girar_der.aprender(&s0n);
  girar_der.aprender(&s1b);
  girar_der.aprender(&doblar);
  cerebro.aprender(&girar_der);

  Sequence girar_izq("Girar izquierda");
  Behavior s1n("S1 negro", s1_negro);
  girar_izq.aprender(&s1n);
  girar_izq.aprender(&s0b);
  girar_izq.aprender(&doblar);
  cerebro.aprender(&girar_izq);
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

  fuzzy->setInput(1, memoria.sensores[2]);

  fuzzy->fuzzify();

  float output = fuzzy->defuzzify(1);
  Serial.print(memoria.sensores[2]);
  Serial.print(" ");
  Serial.println(output);
  // wait 100 milli seconds before looping again
  cerebro.run(memoria);
  delay(100);
}

long microsecondsToCentimeters(long microseconds)
{
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}
