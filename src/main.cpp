#include <Arduino.h>
#include <RMVS.h>

void setup() {
  // put your setup code here, to run once:
  
  Serial.begin(9600);
  int const pressurePin = A0;    // select the input pin for the potentiometer
  float pressureVal = 0;

}

void loop() {
  // put your main code here, to run repeatedly:
  // int a = addFunction(1,2);

  pressureVal = map(analogRead(pressurePin), 0, 668, 0, 100);
  Serial.print(pressureVal);
  delay(800);

}