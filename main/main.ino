#define MORSE_SERIAL_OUTPUT 0
#include "MorseEncoder.h"
MorseEncoder m(LED_BUILTIN, 100);

void setup() {
  if (MORSE_SERIAL_OUTPUT) Serial.begin(9600);
  m.begin();
}

void loop() {
  m.send("hello world");
  m.update();
}
