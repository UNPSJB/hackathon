#include <DCMotor.h>

// TRIGGER, valor definido segun lo que retorne el debug de los sensores, menos un valor aleatorio proximo.  En mi caso el valor de los sensores fue de 930
#define TRIGGER		850
#define MOTOR_SPEED	60.0

float sDerecha, sIzquierda;
DCMotor mDerecha(M0_EN, M0_D0, M0_D1);
DCMotor mIzquierda(M1_EN, M1_D0, M1_D1);

void setup()
{
  Serial.begin(115200);
  // Con este metodo seteamos el sentido de giro de las ruedas.
  mIzquierda.setClockwise(false);
  mDerecha.setClockwise(false);
  mDerecha.setSpeed(MOTOR_SPEED);
  mIzquierda.setSpeed(MOTOR_SPEED);
}

void loop()
{
  //Read sensors:
  sDerecha = analogRead(0);
  sIzquierda = analogRead(1);

  
  //Debug:
  /*
  Serial.print("sDerecha = ");
  Serial.print(sDerecha);
  Serial.print("/ sIzquierda = ");
  Serial.println(sIzquierda);
  */
  
  if ( ( sDerecha > TRIGGER) && ( sIzquierda > TRIGGER ) ) {
          mDerecha.setSpeed(MOTOR_SPEED);
          mIzquierda.setSpeed(MOTOR_SPEED);
  }
  if ( ( sDerecha > TRIGGER ) && ( sIzquierda < TRIGGER ) ) {
          mDerecha.setSpeed(0.1*MOTOR_SPEED);
          mIzquierda.setSpeed(MOTOR_SPEED);
  }
  if ( ( sDerecha < TRIGGER ) && ( sIzquierda > TRIGGER ) ) { 
          mDerecha.setSpeed(MOTOR_SPEED);
          mIzquierda.setSpeed(0.1*MOTOR_SPEED);
  }

}
