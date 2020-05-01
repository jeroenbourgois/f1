// F1
//
// Produces a full F1 startgrid sequence.
//
// Developed for the LSST Data Management System.
// This product includes software developed by the LSST Project
// (https://www.lsst.org).
// See the COPYRIGHT file at the top-level directory of this distribution
// for details of code ownership.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// Circuit:
// - 8 ohm speaker on digital pin 8
// - two photo meters
// - 5 LEDs
// - 10k resistor for phototransistor
// The circuit:
// - LED attached from pin 13 to ground
// - pushbutton attached to pin 2 from +5V
// - 10K resistor attached to pin 2 from ground
//
// We have the following screens:
//
// START -> [Y] -> COUNTDOWN -> RACE --> [Y] -> QUIT? -> [Y] -> START
//                                   |                -> [N] -> RACE
//                                   --> [N] -> n/a
//
// Created:  April 1st, 2020
// Modified: April 25, 2020
// By Jeroen Bourgois.
//
// This code is in the public domain.
//
// TODO:
// - use F() macro when printing

#include "pitches.h"
#include <LiquidCrystal.h>

// Board pins
const int pinBuzzer = 12;
const int pinBtnConfirm = A3;
const int pinBtnCancel = A4;
const int pinLCD_RS = 3;
const int pinLCD_E = 8;
const int pinLCD_D4 = 7;
const int pinLCD_D5 = 9;
const int pinLCD_D6 = 10;
const int pinLCD_D7 = 11;
const int numLights = 5;
const int pinLights[numLights] = {2, 13, 4, 5, 6};
const int pinP1Sensor = A1;
const int pinP2Sensor = A2;

// constants
const unsigned long sensorInterval = 5000; // time between sensor reads
const unsigned long buttonInterval = 5000; // time between button presses
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
  Serial.begin(9600);
}

void loop() {
  currentMillis = millis();
  checkButtons();
  checkSensors();
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
  lastConfirmBtnState = checkButton(pinBtnConfirm, lastConfirmBtnState);
  lastCancelBtnState = checkButton(pinBtnCancel, lastCancelBtnState);
}

int checkButton(int pin, int lastState) {
  if (millis() - previousButtonMillis >= buttonInterval) {
    int state = digitalRead(pin);
    if (state != lastState) {
      Serial.println("BUTTON PRESS");
      if (state == HIGH) {
        handleButtonPress(pin);
      }
      return state;
    }
  }
  return lastState;
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
      millisToString(p1CurLap, p1Time);
      millisToString(p2CurLap, p2Time);
      char line1[17];
      strcpy(line1, p1Name);
      if (p1BestLap == p2BestLap) {
        strcat(line1, "*");
      } else {
        strcat(line1, " ");
      }
      strcat(line1, " ");
      if (p1Laps < 10) {
        strcat(line1, "0");
        strcat(line1, p1Laps);
      } else {
        strcat(line1, p1Laps);
      }
      strcat(line1, " ");
      // Serial.print(p1Laps);
      // Serial.print(" - ");
      // Serial.print(p2Laps);
      // Serial.println();
      if (p2Laps < 10) {
        strcat(line1, "0");
        strcat(line1, p2Laps);
      } else {
        strcat(line1, p2Laps);
      }
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
void checkSensors() {
  checkSensor(pinP1Sensor, p1PreviousSensorMillis, p1CurLap, p1PrevLap, p1BestLap, p1Laps);
  checkSensor(pinP2Sensor, p2PreviousSensorMillis, p2CurLap, p2PrevLap, p2BestLap, p2Laps);
}

void checkSensor(int pin, unsigned long &previousSensorMillis, unsigned long &curLap, unsigned long &prevLap, unsigned long &bestLap, int &laps) {
  // unsigned long val = analogRead(pin);
  if (millis() - previousSensorMillis >= sensorInterval) {

    if (digitalRead(pin) == HIGH) {
      Serial.print("SENSOR");
      Serial.print(" - ");
      Serial.print(previousSensorMillis);
      Serial.println();
      previousSensorMillis = previousSensorMillis + sensorInterval;
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
