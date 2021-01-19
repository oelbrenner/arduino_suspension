// Connections and Layout:
// Connect the common (middle) pin on each encoder to 5v.
// Connect a 10k resistor between ground and each pin on the board you are going to connect to an encoder.
// This pulls the pins low to avoid fluttering mis-reads.
// Connect the appropriate board pins the A and B pins on the encoder. Define which pins these are in the code.
// Board setup:
// analog pins: a0 - a15
// digital pins: d0-d54
// see README for pin restrictions

// LED pin to act as stand in for mosfet
const int mosfetLed_Pin = 13;


// screen setup
#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_HX8357.h"
  // above libs assume the below pin assignments:
  // CLK connects to SPI clock. On Mega's, its Digital 52
  // MISO connects to SPI MISO. On Mega's, its Digital 50
  // MOSI connects to SPI MOSI. On Mega's, its Digital 51
  //
  // These are 'flexible' lines that can be changed
  // CS connects to our SPI Chip Select pin. We'll be using Digital 10
  // D/C connects to our SPI data/command select pin. We'll be using Digital 9
#define TFT_CS 10
#define TFT_DC 9
#define TFT_RST 8 // RST can be set to -1 if you tie it to Arduino's reset
// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);
// Assign human-readable names to some common 16-bit color values:
#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define ORANGE  0xFD20

// Global vars
// number of encoders
int numEnc = 4;
// define all the A pins on each encoder (in order)
const int encPinA[4] = {30, 32, 34, 36};
// define all the B pins on each encoder (in order)
const int encPinB[4] = {31, 33, 35, 37};
// last mode of each pin (HIGH/LOW) for comparison - see if it changed
int lastModeA[4];
int lastModeB[4];
// current mode of each encoder pin (HIGH/LOW)
int curModeA[4];
int curModeB[4];
// current and last encoder positions
float encPos[4];
float encPosLast[4];
// utility variables for encoder timing
int change = 0;
int c = 0;

byte solenoidActivity_State = LOW;
// use the following delay to deal with encoder bounce
const int encoderCheckInterval = 1000;
// stores the value of millis() in each iteration of loop()
unsigned long currentMillis = 0;
// time since encoder was checked
unsigned long previousEncoderCheck = 0;

// height sensor values
int lfSensor = 0;
int rfSensor = 0;
int lrSensor = 0;
int rrSensor = 0;

// height sensor pins
const int lfSensorPin = 6;
const int rfSensorPin = 7;
const int lrSensorPin = 8;
const int rrSensorPin = 9;

// valve block controls
const int lfValvePin = 10;
const int rfValvePin = 11;
const int lrValvePin = 12;
const int rrValvePin = 13;
const int DiaphramValvePin = 14;
const int OutletValvePin   = 15;
const int InletValvePin    = 16;


// individual wheel dwell time
int singleDwell = 200;
// all wheels dwell time
int allDwell = 300;

// setup all the vars for the LCD screen
char lfPosString, rfPosString, lrPosString, rrPosString, msPosString;
char line1[21];
char line2[21];
char line3[21];
char line4[21];

void setup () {
  Serial.begin(9600);
  // refactor this to blink LED for boot sequence
  delay(20);
  // start up the TFT screen
  tft.begin();
  // rotation 1 is landscape with SPI pins on right side
  tft.setRotation(1);
  blackScreen();
  //splashScreen();
  //blackScreen();
  colorSegment();


  // pinMode(mosfetLed_Pin, OUTPUT);
  // digitalWrite(mosfetLed_Pin, solenoidActivity_State);
  // // loop to set up each encoder's initial values
  // for (c = 0; c < numEnc; c++) {
  //   // tell us what it is doing
  //   Serial.print("Initializing encoders ");
  //   Serial.println(c);
  //   // set the pins form INPUT
  //   pinMode(encPinA[c], INPUT);
  //   pinMode(encPinB[c], INPUT);
  //   // set the modes and positions - on first read, it may change position once
  //   //   depending on how the encoders are sitting (having a HIGH position that
  //   //   gets compared to the initial LOW setting here in the first iteration of
  //   //   the loop).
  //   lastModeA[c] = LOW;
  //   lastModeB[c] = LOW;
  //   curModeA[c] = LOW;
  //   curModeB[c] = LOW;
  //   encPos[c] = 0;
  //   encPosLast[c] = 0;
  // }
}

// void loop () {
//
//   currentMillis = millis();
//   check_encoders();
//
// }

void loop(void) {
  cornerRects(WHITE);
}

// fill screen with black
unsigned long blackScreen() {
  unsigned long start = micros();
  tft.fillScreen(HX8357_BLACK);
  return micros() - start;
}
// color bars
unsigned long colorSegment() {
  unsigned long start = micros();
  // red segments
  tft.fillRect(0,     0, 21, 10, RED); // LF
  tft.fillRect(459,   0, 21, 10, RED); // RF
  tft.fillRect(0,   200, 21, 10, RED); // LR
  tft.fillRect(459, 200, 21, 10, RED); // RR
  // yellow segments
  tft.fillRect(0,    10, 21, 30, YELLOW); // LF
  tft.fillRect(459,  10, 21, 30, YELLOW); // RF
  tft.fillRect(0,   210, 21, 30, YELLOW); // LR
  tft.fillRect(459, 210, 21, 30, YELLOW); // RR
  // green segments
  tft.fillRect(0,    40, 21, 50, GREEN); // LF
  tft.fillRect(459,  40, 21, 50, GREEN); // RF
  tft.fillRect(0,   240, 21, 50, GREEN); // LR
  tft.fillRect(459, 240, 21, 50, GREEN); // RR
  // orange segments
  tft.fillRect(0,    90, 21, 30, ORANGE); // LF
  tft.fillRect(459,  90, 21, 30, ORANGE); // RF
  tft.fillRect(0,   290, 21, 30, ORANGE); // LR
  tft.fillRect(459, 290, 21, 30, ORANGE); // RR

  // test stuff
  tft.fillRect(200, 25, 21, 65, GREEN); // test
  tft.fillRect(200, 10, 21, 30, YELLOW); // LF
  tft.fillRect(200,  0, 21, 10, RED); // LF
  tft.fillRect(200, 84, 21, 30, ORANGE); // LF
  tft.fillRect(210,  4, 21, 5, WHITE); // LF
  tft.drawRect(209,  3, 22, 6, BLACK);
  tft.fillRect(210, 24, 21, 5, WHITE); // LF
  tft.drawRect(209, 23, 22, 6, BLACK);
  tft.fillRect(210, 44, 21, 5, WHITE); // LF
  tft.drawRect(209, 43, 22, 6, BLACK);
  tft.fillRect(210, 84, 21, 5, WHITE); // LF
  tft.drawRect(209, 83, 22, 6, BLACK);
  return micros() - start;
}

// startup message
unsigned long splashScreen() {
  unsigned long start = micros();
  tft.setCursor(0, 0);
  tft.setTextColor(ORANGE);    tft.setTextSize(3);
  tft.println("OMFG industries");
  tft.println();
  tft.setTextColor(HX8357_GREEN);
  tft.setTextSize(5);
  tft.println("Range Rover P38");
  tft.println();
  tft.setTextColor(HX8357_RED);    tft.setTextSize(3);
  tft.println("Suspension Controller");
  tft.println();
  tft.setTextSize(1);
  tft.setTextColor(HX8357_WHITE);
  tft.println(F("the software is provided as is, without warranty of any kind, express or implied, including but not limited to the warranties of merchantability, fitness for a particular purpose and noninfringement. in no event shall the authors or copyright holders be liable for any claim, damages or other liability, whether in an action of contract, tort or otherwise, arising from, out of or in connection with the software or the use or other dealings in the software."));
  delay(1500);
  return micros() - start;
}

unsigned long cornerRects(uint16_t color) {
  unsigned long start = micros();
  // left front
  tft.drawRect(20, 0, 100, 120, color);
  // right front
  tft.drawRect(360, 0, 100, 120, color);
  // left rear
  tft.drawRect(20, 200, 100, 120, color);
  // right rear
  tft.drawRect(360, 200, 100, 120, color);

  // text rectangle
  // fill the background
  tft.fillRect(24, 74, 92, 42, BLACK); // LF background
  tft.fillRect(364, 74, 92, 42, BLACK);// RF background
  tft.fillRect(24, 274, 92, 42, BLACK);// LR background
  tft.fillRect(364, 274, 92, 42, BLACK);// RR background
  tft.setTextColor(WHITE, BLACK);

  for (int i = 0; i <= 42; i++) {
    tft.setTextSize(5);
    tft.setCursor(36, 74);
    tft.println(i);
    tft.setCursor(376, 74);
    tft.println(i);
    tft.setCursor(36, 274);
    tft.println(i);
    tft.setCursor(376, 274);
    tft.println(i);
    delay(60);
  }
  delay(200);
  return micros() - start;
}


void check_encoders() {
  //Serial.print("entering check_encoders");
  //Serial.println();

  if (millis() - previousEncoderCheck >= encoderCheckInterval) {
    //Serial.print("times up! checking encoders");
    //Serial.println();
    change = 0;
    for (c = 0; c < numEnc; c++) {
      // read the current state of the current encoder's pins
      curModeA[c] = digitalRead(encPinA[c]);
      curModeB[c] = digitalRead(encPinB[c]);
      // compare the four possible states to figure out what has happened
      //   then increment/decrement the current encoder's position
      if (curModeA[c] != lastModeA[c]) {
        if (curModeA[c] == LOW) {
          if (curModeB[c] == LOW) {
            decrement_encoder(c, encPos[c], encPosLast[c]);
          } else {
            increment_encoder(c, encPos[c], encPosLast[c]);
          }
        } else {
          if (curModeB[c] == LOW) {
            increment_encoder(c, encPos[c], encPosLast[c]);
          } else {
            decrement_encoder(c, encPos[c], encPosLast[c]);
          }
        }
      }
      if (curModeB[c] != lastModeB[c]) {
        if (curModeB[c] == LOW) {
          if (curModeA[c] == LOW) {
            increment_encoder(c, encPos[c], encPosLast[c]);
          } else {
            decrement_encoder(c, encPos[c], encPosLast[c]);
          }
        } else {
          if (curModeA[c] == LOW) {
            decrement_encoder(c, encPos[c], encPosLast[c]);
          } else {
            increment_encoder(c, encPos[c], encPosLast[c]);
          }
        }
      }
      // set the current pin modes (HIGH/LOW) to be the last know pin modes
      //   for the next loop to compare to
      lastModeA[c] = curModeA[c];
      lastModeB[c] = curModeB[c];
      // if this encoder's position changed, flag the change variable
      if (encPos[c] != encPosLast[c]) {
        change = 1;
        update_screen;
      }
    }
  }
  else {
    Serial.print("NOT checking encoders");
    Serial.println();
  }
}

void increment_encoder (int encoderNum, int encoderValue, int encoderValueLast) {
  Serial.print("entering increment_encoder");
  Serial.println();
  Serial.print("Encoder: ");
  Serial.print(encoderNum);
  encPos[c] = encPos[c] + .5;
  Serial.print(" Value: ");
  Serial.print(encoderValue);
  Serial.print(" Last Value: ");
  Serial.print(encoderValueLast);
  Serial.println();

}
void decrement_encoder (int encoderNum, int encoderValue, int encoderValueLast) {
  Serial.print("entering decrement_encoder");
  Serial.println();
  Serial.print("Encoder: ");
  Serial.print(encoderNum);
  encPos[c] = encPos[c] - .5;
  Serial.print(" Value: ");
  Serial.print(encoderValue);
  Serial.print(" Last Value: ");
  Serial.print(encoderValueLast);
  Serial.println();

}

void read_height_sensors() {
  Serial.print("entering read_height_sensors");
  Serial.println();
  // analog reads from the height sensors

  lfSensor = analogRead(lfSensorPin);
  Serial.print("Left Front Height: ");
  Serial.print(lfSensor);

  rfSensor = analogRead(rfSensorPin);
  Serial.print("Right Front Height: ");
  Serial.print(rfSensor);

  lrSensor = analogRead(lrSensorPin);
  Serial.print("Left Rear Height: ");
  Serial.print(lrSensor);

  rrSensor = analogRead(rrSensorPin);
  Serial.print("Right Rear Height: ");
  Serial.print(rrSensor);
}

void raise_wheel(int wheelID) {
  Serial.print("Raising wheel: ");
  Serial.print(wheelID);
  Serial.println();
  Serial.print("opening inlet valve");
  digitalWrite(InletValvePin, HIGH);
  Serial.print("opening wheel valve");
  digitalWrite(wheelID, HIGH);
  delay(singleDwell);
  digitalWrite(InletValvePin, LOW);
  digitalWrite(wheelID, LOW);
}

void raise_all() {
  Serial.print("Raise all wheels");
  Serial.println();
  Serial.print("opening inlet valve");
  digitalWrite(InletValvePin, HIGH);
  Serial.print("opening wheel valves");
  digitalWrite(lfValvePin, HIGH);
  digitalWrite(rfValvePin, HIGH);
  digitalWrite(lrValvePin, HIGH);
  digitalWrite(rrValvePin, HIGH);
  delay(allDwell);
  digitalWrite(lfValvePin, LOW);
  digitalWrite(rfValvePin, LOW);
  digitalWrite(lrValvePin, LOW);
  digitalWrite(rrValvePin, LOW);
  digitalWrite(InletValvePin, LOW);
}

void lower_wheel(int wheelID) {
  Serial.print("Lowering wheel: ");
  Serial.print(wheelID);
  Serial.println();
  Serial.print("opening outlet valve");
  digitalWrite(OutletValvePin, HIGH);
  Serial.print("opening wheel valve");
  digitalWrite(wheelID, HIGH);
  delay(singleDwell);
  digitalWrite(OutletValvePin, LOW);
  digitalWrite(wheelID, LOW);
}

void lower_all() {
  Serial.print("Lower all wheels");
  Serial.println();
  Serial.print("opening outlet valve");
  digitalWrite(OutletValvePin, HIGH);
  Serial.print("opening wheel valves");
  digitalWrite(lfValvePin, HIGH);
  digitalWrite(rfValvePin, HIGH);
  digitalWrite(lrValvePin, HIGH);
  digitalWrite(rrValvePin, HIGH);
  delay(allDwell);
  digitalWrite(rrValvePin, LOW);
  digitalWrite(lrValvePin, LOW);
  digitalWrite(rfValvePin, LOW);
  digitalWrite(lfValvePin, LOW);
  digitalWrite(OutletValvePin, LOW);
}


void update_screen() {

    Serial.print("entering update_screen");
    for (c = 0; c < numEnc; c++) {
          encPosLast[c] = encPos[c];
    }

    lfPosString = (encPos[0]);
    rfPosString = (encPos[1]);
    sprintf(line1, "%d %03d FRNT %d %03d", lfSensor, lfPosString, rfSensor, rfPosString);
    //mySerial.write(254);
    //mySerial.write(128);                     // move cursor to 1st position first line
    //mySerial.write(line1);

    lrPosString = (encPos[2]);
    rrPosString = (encPos[3]);
    sprintf(line2, "%d %03d REAR %d %03d", lrSensor, lrPosString, rrSensor, rrPosString);
    //mySerial.write(254);
    //mySerial.write(192);                     // move cursor to beginning of second line
    //mySerial.write(line2);

    msPosString = (encPos[4]);
    sprintf(line3, "MASTER: %03d", msPosString);
    //mySerial.write(254);
    //mySerial.write(148);                     // move cursor to beginning of third line
    //mySerial.write(line3);
    //
    //mySerial.write(254);
    //mySerial.write(212);                     // move cursor to beginning of fourth line
    //mySerial.write("____________________");

    Serial.print(line1);
    Serial.println();
    Serial.print(line2);
    Serial.println();
    Serial.print(line3);
    Serial.println();
}

