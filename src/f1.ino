// F1
//
// Produces a full F1 startgrid sequence.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// All code is available on [Github](https://github.com/jeroenbourgois/f1)
//
// Circuit:
// - 8 ohm speaker on digital pin 2
// - 5 LEDs connected to digital pins 9 -> 13,
//   each LED grounded with a 220Ω resistor
// - 'OK' pushbutton on analog pin A3 (used as digital pin) and +5V
// - 'CANCEL' pushbutton on analog pin A4 (used as digital pin) and +5V
// - two reed switches to measure passing cars, analog pins A1 and A2 and +5v
// - LCD:
//    - data pins D4 -> D7 wired to digital pins 7 -> 4
//    - E pin wired to digital pin 8
//    - RS pin wired to digital pin 3
//    - VCC+ and
//    - a potentionmeter with it's middle leg connected to the LCD 4
//
// A detailed Fritzing sketch is available in the repository.
//
// On the LCD we have the following screens:
//
// START -> [Y] -> COUNTDOWN -> RACE --> [Y] -> QUIT? -> [Y] -> START
//                                   |                -> [N] -> RACE
//                                   --> [N] -> n/a
//
// Created:       Apr 1, 2020
// Last Modified: May 11, 2020
// By Jeroen Bourgois.
//
// This code is in the public domain.

#include "pitches.h"
#include <LiquidCrystal.h>

// Board pins
const int pinBuzzer = 2;
const int pinBtnConfirm = A3;
const int pinBtnCancel = A4;
const int pinLCD_RS = 3;
const int pinLCD_E = 8;
const int pinLCD_D4 = 7;
const int pinLCD_D5 = 6;
const int pinLCD_D6 = 5;
const int pinLCD_D7 = 4;
const int numLights = 5;
const int pinLights[numLights] = {13, 9, 10, 11, 12};
const int pinP1Sensor = A1;
const int pinP2Sensor = A2;

// constants
const unsigned long sensorInterval = 500; // time between sensor reads
const unsigned long buttonInterval = 500; // time between button presses
const int noteDurationShort = 500;
const int noteDurationLong = 1500;
const int noteDelay = 1000;
const int blockPos[5] = {1, 4, 7, 10, 13};
const byte fullBlock[] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};

// screens
const int startScreen = 0;
const int countdownScreen = 1;
const int countdownGoScreen = 2;
const int raceScreen = 3;
const int finishScreen = 4;
const int quitRaceScreen = 5;

// global variables
char p1Name[4] = "HAM";
unsigned long p1PrevLap = 0;
unsigned long p1CurLap = 0;
unsigned long p1BestLap = 9999999;
unsigned long p1PreviousSensorMillis = 0;
int p1Laps = 0;
char p2Name[4] = "VET";
unsigned long p2PrevLap = 0;
unsigned long p2CurLap = 0;
unsigned long p2BestLap = 9999999;
unsigned long p2PreviousSensorMillis = 0;
int p2Laps = 0;
const char playersPad[11] =     "          ";
const char player1Fastest[11] = " <-       ";
const char player2Fastest[11] = "       -> ";
bool raceActive = false;
int activeScreen = -1;
int nextScreen = 0;
unsigned long currentMillis = 0;
unsigned long previousButtonMillis = 0;

LiquidCrystal lcd(pinLCD_RS, pinLCD_E, pinLCD_D4, pinLCD_D5, pinLCD_D6, pinLCD_D7);

int lastConfirmBtnState = LOW;
int lastCancelBtnState = LOW;
// In F1, the time between all lights on and all lights out
// is a random delay. We take something between 1 and 5 seconds.
int randomStartDelay;

void setup() {
  setupDefaults();
  setupLCD();
  setupLights();
  setupButtons();
  setupReeds();
  Serial.begin(9600);
}

void loop() {
  currentMillis = millis();
  checkButtons();
  checkReeds();
  updateLapTimer();
  updateGameState();
  drawDisplay();
}

void setupLights() {
  randomStartDelay = random(1000, 5000);
  for (int i = 0; i < numLights; i++) {
    pinMode(pinLights[i], OUTPUT);
  }
}

void setupLCD() {
  lcd.begin(16, 2);
  lcd.createChar(0, fullBlock);
}

void setupButtons() {
  pinMode(pinBtnConfirm, INPUT);
  pinMode(pinBtnCancel, INPUT);
}

void setupReeds() {
  pinMode(pinP1Sensor, INPUT);
  pinMode(pinP2Sensor, INPUT);
}

// Used to print a solid block on the LCD
// byte(0) is a full block
void writeFullBlock(int n) {
  lcd.setCursor(n, 0);
  lcd.write(byte(0));
  lcd.setCursor(n + 1, 0);
  lcd.write(byte(0));
  lcd.setCursor(n, 1);
  lcd.write(byte(0));
  lcd.setCursor(n + 1, 1);
  lcd.write(byte(0));
}

void playSequence() {
  delay(5000);

  lcd.clear();

  writeFullBlock(blockPos[0]);

  tone(pinBuzzer, NOTE_C5, noteDurationShort);
  digitalWrite(pinLights[0], HIGH);
  delay(noteDelay);

  writeFullBlock(blockPos[1]);

  tone(pinBuzzer, NOTE_C5, noteDurationShort);
  digitalWrite(pinLights[1], HIGH);
  delay(noteDelay);

  writeFullBlock(blockPos[2]);

  tone(pinBuzzer, NOTE_C5, noteDurationShort);
  digitalWrite(pinLights[2], HIGH);
  delay(noteDelay);

  writeFullBlock(blockPos[3]);

  tone(pinBuzzer, NOTE_C5, noteDurationShort);
  digitalWrite(pinLights[3], HIGH);
  delay(noteDelay);

  writeFullBlock(blockPos[4]);

  tone(pinBuzzer, NOTE_C5, noteDurationShort);
  digitalWrite(pinLights[4], HIGH);
  delay(randomStartDelay);

  setScreen(countdownGoScreen);
  updateGameState();

  tone(pinBuzzer, NOTE_G5, noteDurationLong);

  lightsOut();
  setScreen(raceScreen);

  delay(noteDelay);
  noTone(pinBuzzer);
}

// Depending on the screen, handle the buttons
// Confirm button to start / ok / continue
// Cancel button to cancel / back / stop
void checkButtons() {
  checkButton(pinBtnConfirm);
  checkButton(pinBtnCancel);
}

void checkButton(int pin) {
  if (currentMillis - previousButtonMillis >= buttonInterval) {
    if (digitalRead(pin) == HIGH) {
      previousButtonMillis = currentMillis;
      handleButtonPress(pin);
    }
  }
}

void handleButtonPress(int pin) {
  if (pin == pinBtnConfirm) {
    switch (activeScreen) {
      case startScreen:
        setScreen(countdownScreen);
        break;
      case raceScreen:
        setScreen(quitRaceScreen);
        break;
      case quitRaceScreen:
        resetGame();
        setScreen(startScreen);
        break;
    }
  }
  if (pin == pinBtnCancel) {
    switch (activeScreen) {
      case quitRaceScreen:
        setScreen(raceScreen);
        break;
    }
  }

}

// This drives what happens in the
// setup. Based on the next screen, it will
// perform the needed actions to put the game
// in the next state
void updateGameState() {
  if (nextScreen == activeScreen) {
    return;
  }
  activeScreen = nextScreen;
  switch (nextScreen) {
    case startScreen:
      break;
    case countdownScreen:
      drawDisplay();
      startSequence();
      break;
    case raceScreen:
      if (raceActive == false) {
        startRace();
      }
    case countdownGoScreen:
      drawDisplay();
      break;
  }
}

void resetGame() {
  setupDefaults();
}

void setupDefaults() {
  raceActive = false;
  p2PrevLap = 0;
  p2CurLap = 0;
  p2BestLap = 9999999;
  p1PrevLap = 0;
  p1CurLap = 0;
  p1Laps = 0;
  p2Laps = 0;
  p1BestLap = 9999999;
  activeScreen = -1;
  nextScreen = 0;
}

void startSequence() {
  resetSequence();
  playSequence();
}

void resetSequence() {
  lightsOut();
}

void lightsOut() {
  for (int i = 0; i < numLights; i++) {
    digitalWrite(pinLights[i], LOW);
  }
}

void startRace() {
  p1PrevLap = p1CurLap = p2PrevLap = p2CurLap = currentMillis;
  raceActive = true;
}

void drawDisplay() {
  switch (activeScreen) {
    case startScreen:
      lcd.setCursor(0, 0);
      lcd.write("*  FORMULA ONE *");
      lcd.setCursor(0, 1);
      lcd.write("*  PRESS START *");
      break;
    case countdownScreen:
      lcd.setCursor(0, 0);
      lcd.write("  15 SECS UNTIL ");
      lcd.setCursor(0, 1);
      lcd.write("   LIGHTS OUT   ");
      break;
    case countdownGoScreen:
      lcd.setCursor(0, 0);
      lcd.print(" It's lights out");
      lcd.setCursor(0, 1);
      lcd.print(" and away we go!");
      break;
    case quitRaceScreen:
      lcd.setCursor(0, 0);
      lcd.print("     QUIT?      ");
      lcd.setCursor(0, 1);
      lcd.print("  YES       NO  ");
      break;
    case raceScreen:
      char p1Time[9] = {0};
      char p2Time[9] = {0};
      char laps[3] = {0};
      millisToString(p1CurLap, p1Time);
      millisToString(p2CurLap, p2Time);
      char line1[17];
      strcpy(line1, p1Name);
      if (p1BestLap == p2BestLap) {
        strcat(line1, "*");
      } else {
        strcat(line1, " ");
      }
      sprintf(laps, " %02d ", p1Laps);
      strcat(line1, laps);
      sprintf(laps, " %02d ", p2Laps);
      strcat(line1, laps);
      if (p1BestLap > p2BestLap) {
        strcat(line1, "*");
      } else {
        strcat(line1, " ");
      }
      strcat(line1, p2Name);
      char line2[17];
      strcpy(line2, p1Time);
      strcat(line2, "  ");
      strcat(line2, p2Time);
      lcd.setCursor(0, 0);
      lcd.write(line1);
      lcd.setCursor(0, 1);
      lcd.write(line2);
      break;
    default:
      ;
  }
}

void setScreen(int screen) {
  nextScreen = screen;
  if (nextScreen < 0) {
    nextScreen = 0;
  }
}

// Check the reed sensor
// https://en.wikipedia.org/wiki/Reed_switch
// For now (no reed sensors yet) we stubbed with a push button
void checkReeds() {
  //checkSensor(pinP1Sensor, p1PreviousSensorMillis, p1CurLap, p1PrevLap, p1BestLap, p1Laps);
  checkSensor(pinP2Sensor, p2PreviousSensorMillis, p2CurLap, p2PrevLap, p2BestLap, p2Laps);
}

void checkSensor(int pin, unsigned long &previousSensorMillis, unsigned long &curLap, unsigned long &prevLap, unsigned long &bestLap, int &laps) {
  // unsigned long val = analogRead(pin);
  if (currentMillis - previousSensorMillis >= sensorInterval) {
    int s = digitalRead(pin);
    Serial.print("Reed: ");
    Serial.print(s);
    Serial.println("");
    if (s == LOW) {
      previousSensorMillis = currentMillis;
      if (curLap < bestLap && curLap > 1000) {
        bestLap = curLap;
      }
      prevLap = currentMillis;
      laps = laps + 1;
    }
  }
}

void updateLapTimer() {
  unsigned long timestamp = currentMillis;
  p1CurLap = timestamp - p1PrevLap;
  p2CurLap = timestamp - p2PrevLap;
}

char * millisToString(unsigned long millis, char *out) {
  int s = (millis / 1000) % 60;
  int m = (millis / 1000) / 60;
  int ms = (millis % 1000) / 10;
  sprintf(out, "%0d:%02d:%02d", m, s, ms);
  return out;
}
