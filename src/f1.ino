/*
 * F1
 *
 * Produces a full F1 startgrid sequence.
 *
 * Developed for the LSST Data Management System.
 * This product includes software developed by the LSST Project
 * (https://www.lsst.org).
 * See the COPYRIGHT file at the top-level directory of this distribution
 * for details of code ownership.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Circuit:
 * - 8 ohm speaker on digital pin 8
 *
 * The circuit:
 * - LED attached from pin 13 to ground
 * - pushbutton attached to pin 2 from +5V
 * - 10K resistor attached to pin 2 from ground
 *
 * created 1 Ap 2020
 * modified 1 Ap 2020
 * by defborn
 *
 * This code is in the public domain.
 *
 * TODO:
 * - use F() macro when printing
 */

#include "pitches.h"
#include <LiquidCrystal.h>

const int PIN_BUZZER = 12;
const int PIN_BTN = A5;
const int PIN_LCD_RS = 3;
const int PIN_LCD_E = 8;
const int PIN_LCD_D4 = 7;
const int PIN_LCD_D5 = 9;
const int PIN_LCD_D6 = 10;
const int PIN_LCD_D7 = 11;
const int NUM_LIGHTS = 5;
const int PIN_LIGHTS[NUM_LIGHTS] = {2, 13, 4, 5, 6};
const int NOTE_DURATION_SHORT = 500;
const int NOTE_DURATION_LONG = 1500;
const int NOTE_DELAY = 1000;
const int PIN_P1_SENSOR = A0;
const int PIN_P2_SENSOR = A1;

char p1Name[4] = "HAM";
unsigned long p1PrevLap = 0;
unsigned long p1CurLap = 0;
unsigned long p1BestLap = 0;
char p2Name[4] = "VET";
unsigned long p2PrevLap = 0;
unsigned long p2CurLap = 0;
unsigned long p2BestLap = 0;
const char PLAYERS_PAD[11] =      "..........";
const char PLAYER_1_FASTEST[11] = " <-       ";
const char PLAYER_2_FASTEST[11] = ".......->.";
const int DISPLAY_START = 0;
const int DISPLAY_COUNTDOWN_START = 1;
const int DISPLAY_COUNTDOWN_GO = 2;
const int DISPLAY_RACE = 3;
const int DISPLAY_FINISH = 4;
int displayState = 0;
float sensorP1Value = 0.0;
float sensorP2Value = 0.0;
char* p1Time = new char[9];
char* p2Time = new char[9];
char _m[3];
char _s[3];
char _ms[3];
char* _mls = new char[12];

const int BLOCK_POS[5] = {1, 4, 7, 10, 13};
const byte FULL_BLOCK[] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};

LiquidCrystal lcd(PIN_LCD_RS, PIN_LCD_E, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);

int lastButtonState = LOW;
// In F1, the time between all lights on
// and all lights out (= GO GO GO!) is
// a random delay between 1 and 5 seconds.
int randomStartDelay;

void setup() {
  lcd.begin(16, 2);
  lcd.createChar(0, FULL_BLOCK);
  initLights();
  Serial.begin(9600);
}

void initLights() {
  randomStartDelay = random(1000, 5000);
  for (int i = 0; i < NUM_LIGHTS; i++) {
    pinMode(PIN_LIGHTS[i], OUTPUT);
  }
  pinMode(PIN_BTN, INPUT);
}

void loop() {
  checkMainBtn();
  checkSensor();
  updateLapTimer();
  drawDisplay();
}

/*
* byte(0) is a full block
*/
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
  delay(NOTE_DELAY);

  writeFullBlock(BLOCK_POS[0]);

  tone(PIN_BUZZER, NOTE_C5, NOTE_DURATION_SHORT);
  digitalWrite(PIN_LIGHTS[0], HIGH);
  delay(NOTE_DELAY);

  writeFullBlock(BLOCK_POS[1]);

  tone(PIN_BUZZER, NOTE_C5, NOTE_DURATION_SHORT);
  digitalWrite(PIN_LIGHTS[1], HIGH);
  delay(NOTE_DELAY);

  writeFullBlock(BLOCK_POS[2]);

  tone(PIN_BUZZER, NOTE_C5, NOTE_DURATION_SHORT);
  digitalWrite(PIN_LIGHTS[2], HIGH);
  delay(NOTE_DELAY);

  writeFullBlock(BLOCK_POS[3]);

  tone(PIN_BUZZER, NOTE_C5, NOTE_DURATION_SHORT);
  digitalWrite(PIN_LIGHTS[3], HIGH);
  delay(NOTE_DELAY);

  writeFullBlock(BLOCK_POS[4]);

  tone(PIN_BUZZER, NOTE_C5, NOTE_DURATION_SHORT);
  digitalWrite(PIN_LIGHTS[4], HIGH);
  delay(randomStartDelay);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  Go! Go! Go! ");

  tone(PIN_BUZZER, NOTE_G5, NOTE_DURATION_LONG);
  lightsOut();

  delay(NOTE_DELAY);
  noTone(PIN_BUZZER);

  startRace();
}

/**
   Read the main button state

   If the button is pressed (compared to last state)
   we (re)start the sequence.
   Analog read!
*/
void checkMainBtn() {
  int buttonState = digitalRead(PIN_BTN);
  if (buttonState != lastButtonState) {
    if (buttonState == HIGH) {
      startSequence();
      // startRace();
    }
    delay(50);
  }
  lastButtonState = buttonState;
}

void startSequence() {
  resetSequence();
  playSequence();
}

void resetSequence() {
  lightsOut();
}

void lightsOut() {
  for (int i = 0; i < NUM_LIGHTS; i++) {
    digitalWrite(PIN_LIGHTS[i], LOW);
  }
}

// Controleer de waarde van de lichtsensor
void checkSensor() {
  sensorP1Value = analogRead(PIN_P1_SENSOR);
  if (sensorP1Value > 100 && sensorP1Value < 200) {
    if (p1CurLap < p1BestLap) {
      p1BestLap = p1CurLap;
    }
    // if (p2CurLap < p2BestLap || p2BestLap == 0) {
    //   p2BestLap = p2CurLap;
    // }
    p1PrevLap = millis();
  }
  sensorP2Value = analogRead(PIN_P1_SENSOR);
  if (sensorP2Value > 100 && sensorP2Value < 200) {
    if (p1CurLap < p1BestLap) {
      p1BestLap = p1CurLap;
    }
    // if (p2CurLap < p2BestLap || p2BestLap == 0) {
    //   p2BestLap = p2CurLap;
    // }
    p2PrevLap = millis();
  }
}

void updateLapTimer() {
  unsigned long timestamp = millis();
  p1CurLap = timestamp - p1PrevLap;
  // p2CurLap = timestamp - p2PrevLap;
}

void startRace() {
  displayState = DISPLAY_RACE;
  // p1PrevLap = p1CurLap = p2PrevLap = p2CurLap = millis();
}

void drawDisplay() {
  switch (displayState) {
    case DISPLAY_START:
      Serial.println("START DISPLAY");
      lcd.setCursor(0, 0);
      lcd.write("*  FORMULA ONE * ");
      lcd.setCursor(0, 1);
      lcd.write(" -> press start ");
      break;

    case DISPLAY_RACE:
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
      strcat(line1, PLAYER_1_FASTEST);
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
