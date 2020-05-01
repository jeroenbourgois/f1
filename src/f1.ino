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

const int noteDurationShort = 500;
const int noteDurationLong = 1500;
const int noteDelay = 1000;
char p1Name[4] = "HAM";
unsigned long p1PrevLap = 0;
unsigned long p1CurLap = 0;
unsigned long p1BestLap = 9999999;
char p2Name[4] = "VET";
unsigned long p2PrevLap = 0;
unsigned long p2CurLap = 0;
unsigned long p2BestLap = 9999999;
const char playersPad[11] =     "          ";
const char player1Fastest[11] = " <-       ";
const char player2Fastest[11] = "       -> ";

// screens
const int startScreen = 0;
const int countdownScreen = 1;
const int countdownGoScreen = 2;
const int raceScreen = 3;
const int finishScreen = 4;
int activeScreen = -1;
int nextScreen = 0;

// reed switch

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

LiquidCrystal lcd(pinLCD_RS, pinLCD_E, pinLCD_D4, pinLCD_D5, pinLCD_D6, pinLCD_D7);

int lastConfirmBtnState = LOW;
int lastCancelBtnState = LOW;
// In F1, the time between all lights on and all lights out
// is a random delay. We take something between 1 and 5 seconds.
int randomStartDelay;

void setup() {
  setupLCD();
  setupLights();
  setupButtons();
  Serial.begin(9600);
}

void loop() {
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
  delay(10000);
  delay(noteDelay);

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
  lastConfirmBtnState = checkButton(pinBtnConfirm, lastConfirmBtnState, 1);
  lastCancelBtnState = checkButton(pinBtnCancel, lastCancelBtnState, -1);
}

int checkButton(int pin, int lastState, int screenBump) {
  int state = digitalRead(pin);
  if (state != lastState) {
    if (state == HIGH) {
      setScreen(activeScreen + screenBump);
    }
    delay(50);
    return state;
  }
  return lastState;
}

// This drives what happens in the
// setup. Based on the next screen, it will
// perform the needed actions to put the game
// in the next state
void updateGameState() {
  if (nextScreen == activeScreen) {
    return;
  }
  Serial.print(" A: ");
  Serial.print(activeScreen);
  Serial.print(" | N: ");
  Serial.print(nextScreen);
  Serial.println();
  activeScreen = nextScreen;
  switch (nextScreen) {
    case countdownScreen:
      drawDisplay();
      startSequence();
      break;
    case raceScreen:
      startRace();
    case countdownGoScreen:
      drawDisplay();
      break;
  }

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
  unsigned long ms = millis();
  p1PrevLap = ms;
  p1CurLap = ms;
  p2PrevLap = ms;
  p2CurLap = ms;
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
      lcd.write("   10 SEC UNTIL ");
      lcd.setCursor(0, 1);
      lcd.write("    LIGHTS OUT  ");
      break;
    case countdownGoScreen:
      lcd.setCursor(0, 0);
      lcd.print(" It's lights out");
      lcd.setCursor(0, 1);
      lcd.print(" and away we go!");
      break;
    case raceScreen:
      char p1Time[9] = {0};
      char p2Time[9] = {0};
      millisToString(p1CurLap, p1Time);
      millisToString(p2CurLap, p2Time);
      char line1[17];
      strcpy(line1, p1Name);
      if (p1BestLap == p2BestLap) {
        strcat(line1, playersPad);
      } else if (p1BestLap < p2BestLap) {
        strcat(line1, player1Fastest);
      } else {
        strcat(line1, player2Fastest);
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
  }
}

void setScreen(int screen) {
  nextScreen = screen;
  if(nextScreen < 0) {
    nextScreen = 0;
  }
  Serial.print("setScreen(");
  Serial.print(screen);
  Serial.print(")");
  Serial.println();
}

// Check the reed sensor
// https://en.wikipedia.org/wiki/Reed_switch
// For now (no reed sensors yet) we stubbed with a push button
void checkSensors() {
  checkSensor(pinP1Sensor, p1CurLap, p1PrevLap, p1BestLap);
  checkSensor(pinP2Sensor, p2CurLap, p2PrevLap, p2BestLap);
}

void checkSensor(int pin, unsigned long &curLap, unsigned long &prevLap, unsigned long &bestLap) {
  // unsigned long val = analogRead(pin);
  int val = digitalRead(pin);
  if (val == HIGH) {
    if (curLap < bestLap && curLap > 1000) {
      bestLap = curLap;
    }
    prevLap = millis();
  }
}

void updateLapTimer() {
  unsigned long timestamp = millis();
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

