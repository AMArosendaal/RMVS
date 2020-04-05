#include <Arduino.h>
#include <RMVS.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:

  Serial.println("Hello World");
  int a = addFunction(1,2);
}