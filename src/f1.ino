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
//
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

const int pinBuzzer = 12;
const int pinBtnConfirm = A5;
const int pinBtnCancel = A4;
const int pinLCD_RS = 3;
const int pinLCD_E = 8;
const int pinLCD_D4 = 7;
const int pinLCD_D5 = 9;
const int pinLCD_D6 = 10;
const int pinLCD_D7 = 11;
const int numLights = 5;
const int pinLights[numLights] = {2, 13, 4, 5, 6};
const int noteDurationShort = 500;
const int noteDurationLong = 1500;
const int noteDelay = 1000;
const int pinP1Sensor = A0;
const int pinP2Sensor = A1;

char p1Name[4] = "HAM";
unsigned long p1PrevLap = 0;
unsigned long p1CurLap = 0;
unsigned long p1BestLap = 0;
char p2Name[4] = "VET";
unsigned long p2PrevLap = 0;
unsigned long p2CurLap = 0;
unsigned long p2BestLap = 0;
const char playersPad[11] =      "..........";
const char player1Fastest[11] = " <-       ";
const char player2Fastest[11] = ".......->.";
const int displayStart = 0;
const int displayCountdownStart = 1;
const int displayCountdownGo = 2;
const int displayRace = 3;
const int displayFinish = 4;
int displayState = 0;
float sensorP1Value = 0.0;
float sensorP2Value = 0.0;
char* p1Time = new char[9];
char* p2Time = new char[9];
char _m[3];
char _s[3];
char _ms[3];
char* _mls = new char[12];

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
// In F1, the time between all lights on and all lights out is
// a random delay between 1 and 5 seconds.
int randomStartDelay;

void setup() {
  lcd.begin(16, 2);
  lcd.createChar(0, fullBlock);
  initLights();
  Serial.begin(9600);
}

void initLights() {
  randomStartDelay = random(1000, 5000);
  for (int i = 0; i < numLights; i++) {
    pinMode(pinLights[i], OUTPUT);
  }
  pinMode(pinBtnConfirm, INPUT);
}

void loop() {
  checkButtons();
  checkSensors();
  updateLapTimer();
  drawDisplay();
}

// Used to print a solid block on the LCD
// byte(0) is a full block
void writeFullBlock(int n) {
  lcd.setCursor(n, 0);
  lcd.write(byte(0));
  lcd.setCursor(n+1, 0);
  lcd.write(byte(0));
  lcd.setCursor(n, 1);
  lcd.write(byte(0));
  lcd.setCursor(n+1, 1);
  lcd.write(byte(0));
}

void playSequence() {
  lcd.clear();
  delay(noteDelay);

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

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  Go! Go! Go! ");

  tone(pinBuzzer, NOTE_G5, noteDurationLong);
  lightsOut();

  delay(noteDelay);
  noTone(pinBuzzer);

  startRace();
}

// Depending on the screen, handle the buttons
// Confirm button to start / ok / continue
// Cancel button to cancel / back / stop
void checkButtons() {
  checkConfirmButton();
  checkCancelButton();
}

void checkConfirmButton() {
  int state = digitalRead(pinBtnConfirm);
  if (state != lastConfirmBtnState) {
    if (state == HIGH) {
      startSequence();
    }
    delay(50);
    lastConfirmBtnState = state;
  }
}

void checkCancelButton() {
  int state = digitalRead(pinBtnCancel);
  if (state != lastCancelBtnState) {
    if (state == HIGH) {
      startSequence();
    }
    delay(50);
    lastCancelBtnState = state;
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

// Controleer de waarde van de lichtsensor
void checkSensors() {
  checkSensor(pinP1Sensor, curLap)
  sensorP1Value = analogRead(pinP1Sensor);
  if (sensorP1Value > 100 && sensorP1Value < 200) {
    if (p1CurLap < p1BestLap) {
      p1BestLap = p1CurLap;
    }
    // if (p2CurLap < p2BestLap || p2BestLap == 0) {
    //   p2BestLap = p2CurLap;
    // }
    p1PrevLap = millis();
  }

}

void checkSensor(int sensorPin, unsigned long *curLap, unsigned long *bestLap, unsigned long *prevLap) {
  unsigned long val = analogRead(sensorPin);
  if (val > 100 && val < 200) {
    if (curLap < bestLap) {
      *bestLap = &curLap;
    }
    *prevLap = millis();
  }
}

void updateLapTimer() {
  unsigned long timestamp = millis();
  p1CurLap = timestamp - p1PrevLap;
  // p2CurLap = timestamp - p2PrevLap;
}

void startRace() {
  displayState = displayRace;
  // p1PrevLap = p1CurLap = p2PrevLap = p2CurLap = millis();
}

void drawDisplay() {
  switch (displayState) {
    case displayStart:
      Serial.println("START DISPLAY");
      lcd.setCursor(0, 0);
      lcd.write("*  FORMULA ONE * ");
      lcd.setCursor(0, 1);
      lcd.write(" -> press start ");
      break;

    case displayRace:
      // Serial.println("RACE DISPLAY");
      p1Time = millisToString(p1CurLap);
      p2Time = millisToString(p2CurLap);
      // char p1Time[8] = "0:00:00";
      // char p2Time[8] = "0:00:00";
      //       Serial.println(p1CurLap);
      // Serial.println(p1Time);
      char line1[17];
      strcpy(line1, p1Name);
      // if (p1BestLap == p2BestLap) {
      //   strcat(line1, PLAYERS_PAD);
      // } else if (p1BestLap < p2BestLap) {
      //   strcat(line1, PLAYER_1_FASTEST);
      // } else {
      //   strcat(line1, PLAYER_2_FASTEST);
      // }
      strcat(line1, player1Fastest);
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

char * millisToString(unsigned long millis) {
  // bit dirty?
  if (millis == 0) {
    return "0:00:00";
  }
  // char* str = new char[12];
  int s = (millis / 1000) % 60;
  int m = (millis / 1000) / 60;
  int ms = (millis % 1000) / 10;
  sprintf(_mls, "%0d:%02d:%02d", m, s, ms);
  return _mls;
}

