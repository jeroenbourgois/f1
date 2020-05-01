// Just a testfile to tryout some of the methods used
// in the Arduino sketch. It was faster doing it
// in C++ and test with the compiler/cli
#include<iostream>

unsigned long millis() {
  return 1000.0;
}

void checkSensor(int sensorPin, unsigned long &curLap, unsigned long &bestLap, unsigned long &prevLap) {
  unsigned long val = 5000; //analogRead(sensorPin);
  prevLap = 1234.0;
  curLap = 20.0;
  if (val > 100 && val < 200) {
    if (curLap < bestLap) {
      bestLap = curLap;
    }
    prevLap = millis();
  }
}

char * millisToString(unsigned long millis, char *out) {
  int s = (millis / 1000) % 60;
  int m = (millis / 1000) / 60;
  int ms = (millis % 1000) / 10;
  sprintf(out, "%0d:%02d:%02d", m, s, ms);
  return out;
}

int main () {
  int sensorPin = 1;
  unsigned long curLap = 1000;
  unsigned long bestLap = 1000;
  unsigned long prevLap = 1000;

  /* std::cout << curLap << std::endl; */
  /* std::cout << bestLap << std::endl; */
  /* std::cout << prevLap << std::endl; */
  /* std::cout << "-----" << std::endl; */
  /* checkSensor(sensorPin, curLap, bestLap, prevLap); */
  /* std::cout << curLap << std::endl; */
  /* std::cout << bestLap << std::endl; */
  /* std::cout << prevLap << std::endl; */
  /* std::cout << "-----" << std::endl; */
  /* char p1Time[9] = {0}; */
  char *p1Time[9] = {0};
  millisToString(1500, p1Time);
  std::cout << p1Time << std::endl;
  /* const char* p2Time = millisToString(2500); */
  /* std::cout << p2Time << std::endl; */
  /* p1Time = millisToString(1500); */
  /* std::cout << p1Time << std::endl; */
  /* p2Time = millisToString(0); */
  /* std::cout << p2Time << std::endl; */
}
