//#define MORSE_SERIAL_OUTPUT //define this to send morse code by Serial ('-' is for 1 dot period delay, '>' - 2 dot delay, '==' - 4 dot delay)
//#define MORSE_PIN_OUTPUT  //define this to send morse code on digital pin (LED_BUILTIN is default)
//#define MORSE_TONE_OUTPUT 10
#include "MorseEncoder.h"

char mess[] = "hello world";

void setup() {
#ifdef MORSE_SERIAL_OUTPUT
  Serial.begin(9600);
#endif
  MorseEncoder.begin(100);
}

void loop() {
  MorseEncoder.send((char*)mess, sizeof(mess));
  MorseEncoder.update();
#ifdef MORSE_TONE_OUTPUT
  if (MorseEncoder.getSignal()) tone(MORSE_TONE_OUTPUT, 15);
  else noTone(MORSE_TONE_OUTPUT);
#endif
}
