#include <Arduino.h>
#include <RMVS.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <toneAC.h>

#define OLED_ADDR   0x3C  // OLED display TWI address
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

// Schmitt algorithm
long lastInsp = 0;
int signal = 0;
int risingTrigger = 40;
int risingMeanTrigger = 10;
int fallingTrigger = 3;
bool detectionState = false;
const int lag = 30; // lag-1 (5) for the smoothing functions
const float threshold = 7; //3.5 standard deviations for signal
float floatArray[50];
float mean; 

// *** VIRTUAL INPUTS
const int pressurePin = A0;       // (A0) select the input pin for potMeter A  
const int pressurePeriodPin = A1; // (A1) select the input pin for potMeter B
const int  batteryVoltagePin = A2; // (A2) select the input pin for potMeter C
const int  xPowerPin = A3;         // (A3) select the input for Potmeter D 

// *** OUTPUTS 
const int rLedPin = 2;
const int yLedPin = 3;
const int gLedPin = 4;
const int buzzerPin = 7;

// *** OTHER VARIABLES
const float PressureSensorOffset = 88;
float pressure;               // Measured pressure from spx? sensor. (0-100 mbar)
float  pressureLV = -5;      // Pressure lower value
float  pressureUV = 50;      // Pressure upper value (100). 50 mbar to be able to use breath to set alarm
float pressurePeriod;           // Period between pressure spikes. (2 - 6 s)
int  pressurePeriodLV = 2;    // Period lower value
int  pressurePeriodUV = 6;   // Perid upper value 
float batteryVoltage;         // Battery voltage. (0-30 V)
float  batteryVoltageLV = 20; // Battery voltage lower value  
bool xPower;                  // Power available. 
bool xPressureOK;             // If true, pressure is OK
bool xPowerOK;              // If true, power supply & battery is OK

unsigned long lastUpdate = 0;          // Required for buzzer

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void printOLED(int X,int Y, String Text){
  display.setCursor(X,Y);
  display.print(Text);
  display.display();
}

float getMean (float inptArray[]){
    float s;
    s = 0;

    for ( int i = 0; i <lag; i++)
    {
        s = s+ inptArray[i];
    }

    return s/lag;
}

void setup() {

  Serial.begin(9600); // initialize serial
  pinMode (rLedPin, OUTPUT);
  pinMode (yLedPin, OUTPUT);
  pinMode (gLedPin, OUTPUT);

  digitalWrite (rLedPin, LOW);
  digitalWrite (yLedPin, LOW);
  digitalWrite (gLedPin, LOW);

    // initialize and clear display
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.display();

  // display a pixel in each corner of the screen
  display.drawPixel(0, 0, WHITE);
  display.drawPixel(127, 0, WHITE);
  display.drawPixel(0, 63, WHITE);
  display.drawPixel(127, 63, WHITE);

  // display a line of text
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(27, 30);
  String var  = "Hello, world1";
  display.print(var);

  // update display with all of the above graphics
  display.display();
}

void loop() {
  // Read pressure sensor MPX5010DP
    pressure = analogRead(pressurePin); // Raw analog value
    pressure = pressure * 5/1023; // Analog value to Voltage
    pressure = ((pressure/5)-0.04)/0.09; // Transfer function from datasheet 
    pressure = 10 * pressure; // kPa to mbar
    pressure = pressure + 0; // Offset, (what is our offset ? )

  // Read virtual parameters from potMeters
  // pressurePeriod = map(analogRead(pressurePeriodPin), 1023, 0, 0, 10);  // Read virtual frequency
  batteryVoltage = map(analogRead(batteryVoltagePin), 1023, 0, 0, 30);  // Read virtual battery Voltge
  xPower = map(analogRead(xPowerPin), 1023, 0,0,10); // Read main power input

  // Print measured values to OLED screen. Update every 1/2 second.
  if (lastUpdate + 1000 < millis()){
    display.clearDisplay();
    printOLED(0,0,String("P (mbar): " + String(pressure)));
    printOLED(0,9,String("T (s): " + String(pressurePeriod)));
    printOLED(0,18,String("Batt(V): " + String(batteryVoltage)));
    printOLED(0,27,String("Power: " + String(xPower)));
    lastUpdate = millis();
  }

  // Check threshold values for pressure
  if ((pressure > pressureUV) || (pressure < pressureLV) || (((millis()-lastInsp)/1000) > pressurePeriodUV) || (((millis()-lastInsp)/1000) < pressurePeriodLV)) {
    digitalWrite (rLedPin, HIGH);
    xPressureOK = false;
  }
  else {
    digitalWrite (rLedPin, LOW);
    xPressureOK = true;
  }

  if ( (batteryVoltage < batteryVoltageLV) || !xPower) {
    digitalWrite (yLedPin, HIGH);
    xPowerOK = false;
  }
  else {
    digitalWrite(yLedPin, LOW);
    xPowerOK = true;
  }

  if (xPowerOK && xPressureOK){
    digitalWrite(gLedPin, HIGH);
  }

  else {
    digitalWrite(gLedPin, LOW);
  }

if (!xPowerOK || !xPressureOK) {
  //  toneAC(10, 10, 100, false); // Play thisNote at full volume for noteDuration in the background.
}

if ((((millis()-lastInsp)/1000) > pressurePeriodUV)){
  pressurePeriod = 999;
}


mean = getMean(floatArray); // get mean

// Create signal based on schmitt trigger
  if ((pressure > risingTrigger) && !detectionState) {
    detectionState = !detectionState;
  }

  if ((pressure < fallingTrigger) && detectionState && mean > risingMeanTrigger){
    signal = 1;
    pressurePeriod = (millis()-lastInsp)/1000;
    lastInsp = millis();
    detectionState = !detectionState;
  }
  else {
    signal = 0;
  }

// Fill vector with latest pressure value 
for (int i = 0 ; i <lag ; i++){
  floatArray[i] = floatArray[i+1];
}
floatArray[lag]  = pressure;

Serial.print(signal);
Serial.print("\t");
Serial.print(pressure);
Serial.print("\t");
Serial.println(mean);
}