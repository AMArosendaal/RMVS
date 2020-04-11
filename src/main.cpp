#include <Arduino.h>
#include <RMVS.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <toneAC.h>

#define OLED_ADDR   0x3C  // OLED display TWI address
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

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

float pressure;               // Measured pressure from spx? sensor. (0-100 mbar)
float  pressureLV = 0;      // Pressure lower value
float  pressureUV = 100;      // Pressure upper value
int pressurePeriod;           // Period between pressure spikes. (2 - 6 s)
int  pressurePeriodLV = 2;    // Period lower value
int  pressurePeriodUV = 6;   // Perid upper value 
float batteryVoltage;         // Battery voltage. (0-30 V)
float  batteryVoltageLV = 20; // Battery voltage lower value  
bool xPower;                  // Power available. 
bool xPressureOK;             // If true, pressure is OK
bool xPowerOK;              // If true, power supply & battery is OK
String errorMessage;          // errorMessage containing feedback 
bool xBuzzerState = false; 

unsigned long lastUpdate = 0;          // Required for buzzer
unsigned long buzzerFreq = 500;            // Buzzer frequency

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void printOLED(int X,int Y, String Text){
  display.setCursor(X,Y);
  display.print(Text);
  display.display();
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

  // Read potMeters
  pressure = map(analogRead(pressurePin), 1023, 0, -100, 200);  // Read pressure
  pressurePeriod = map(analogRead(pressurePeriodPin), 1023, 0, 0, 10);  // Read frequency
  batteryVoltage = map(analogRead(batteryVoltagePin), 1023, 0, 0, 30);  // Read frequency
  xPower = map(analogRead(xPowerPin), 1023, 0,0,10); // Read xPower

  
  // Print measured values to OLED screen. Update every 1/2 second.
  if (lastUpdate + 500 < millis()){
    display.clearDisplay();
    printOLED(0,0,String("P (mbar): " + String(pressure)));
    printOLED(0,9,String("T (s): " + String(pressurePeriod)));
    printOLED(0,18,String("Batt(V): " + String(batteryVoltage)));
    printOLED(0,27,String("Power: " + String(xPower)));
    lastUpdate = millis();
  }
 
  
  // Check threshold values for pressure
  if ((pressure > pressureUV) || (pressure < pressureLV) || (pressurePeriod > pressurePeriodUV) || (pressurePeriod < pressurePeriodLV)) {
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
  toneAC(300, 10, 100, false); // Play thisNote at full volume for noteDuration in the background.
}

}