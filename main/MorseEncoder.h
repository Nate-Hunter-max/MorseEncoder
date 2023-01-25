/*
   ######################################################################
   #    MorseEncoder.h - Library for generating Morse code using FSM    #
   #            Created by Nate E. Hunter, January 26, 2023.            #
   #                  Released into the public domain.                  #
   ###################################################################### 
*/
#pragma once
#include "morseCode.h"
class MorseEncoder {
private:
  enum : uint8_t {
    MAIN,
    CHR,
    DASH,
    DOT,
    SHORT_DELAY,
    CHAR_DELAY,
    WORD_DELAY,
  } lastState,
    state = MAIN;
  String mess;
  uint16_t currentCharIndex;
  bool isBusy;
  uint8_t OutputPin;
  uint16_t DotDelay;
  void mainState(uint16_t&);
  void charState(uint16_t&);
  void dotState(uint32_t&);
  void dashState(uint32_t&);
  void shortDelayState(uint32_t&);
  void charDelayState(uint32_t&);
  void wordDelayState(uint32_t&);

public:
  MorseEncoder(uint8_t, uint16_t);
  void begin();
  void update(bool);
  void send(String);
};

void MorseEncoder::mainState(uint16_t& dataIndex) {
  if (state != lastState) {
    lastState = state;
    if (currentCharIndex == mess.length()) {
      state = WORD_DELAY;
      currentCharIndex++;
    } else if (currentCharIndex == mess.length() + 1) {
#if (MORSE_SERIAL_OUTPUT == 1)
      Serial.println();
#endif
      isBusy = false;
    };
  }
  if (currentCharIndex < mess.length()) {
    if (mess[currentCharIndex] == ' ') state = WORD_DELAY;
    else {
      dataIndex = mess[currentCharIndex] - (mess[currentCharIndex] < 57 ? 22 : 97);
      state = CHR;
    }
    currentCharIndex++;
  }
}

void MorseEncoder::charState(uint16_t& dataIndex) {
  static int bitIndex;
  if (state != lastState) {
    if (lastState == MAIN) bitIndex = lenArr[dataIndex];
    lastState = state;
  }
  if (bitIndex > 0) {
    bitIndex--;
    state = bitRead(pgm_read_byte(&dataArr[dataIndex]), bitIndex) ? DASH : DOT;
  } else state = CHAR_DELAY;
}

void MorseEncoder::dotState(uint32_t& startMillis) {
  if (state != lastState) {
    lastState = state;
    startMillis = millis();
#if (MORSE_SERIAL_OUTPUT == 1)
    Serial.print("DOT");
#endif
    digitalWrite(OutputPin, 0x01);
  }
  if (millis() - startMillis >= DotDelay) {
    digitalWrite(OutputPin, 0x00);
    state = SHORT_DELAY;
  }
};

void MorseEncoder::dashState(uint32_t& startMillis) {
  if (state != lastState) {
    lastState = state;
    startMillis = millis();
#if (MORSE_SERIAL_OUTPUT == 1)
    Serial.print("DASH");
#endif
    digitalWrite(OutputPin, 0x01);
  }
  if (millis() - startMillis >= DotDelay * 3) {
    digitalWrite(OutputPin, 0x00);
    state = SHORT_DELAY;
  }
}

void MorseEncoder::shortDelayState(uint32_t& startMillis) {
  if (state != lastState) {
    lastState = state;
    startMillis = millis();
#if (MORSE_SERIAL_OUTPUT == 1)
    Serial.print("-");
#endif
  }
  if (millis() - startMillis >= DotDelay) state = CHR;
}

void MorseEncoder::charDelayState(uint32_t& startMillis) {
  if (state != lastState) {
    lastState = state;
    startMillis = millis();
#if (MORSE_SERIAL_OUTPUT == 1)
    Serial.print(">");
#endif
  }
  if (millis() - startMillis >= DotDelay << 1) state = MAIN;
}

void MorseEncoder::wordDelayState(uint32_t& startMillis) {
  if (state != lastState) {
    lastState = state;
    startMillis = millis();
#if (MORSE_SERIAL_OUTPUT == 1)
    Serial.print("==");
#endif
  }
  if (millis() - startMillis >= DotDelay << 2) state = MAIN;
}

MorseEncoder::MorseEncoder(uint8_t pin = 13, uint16_t speed = 100) {
  OutputPin = pin;
  DotDelay = speed;
};
void MorseEncoder::begin() {
  pinMode(OutputPin, OUTPUT);
  state = MAIN;
}

void MorseEncoder::update(bool newMess = false) {
  static uint32_t stateMillis;
  static uint16_t dataIndex;
  switch (state) {
    case MAIN:
      mainState(dataIndex);
      break;
    case DOT:
      dotState(stateMillis);
      break;
    case DASH:
      dashState(stateMillis);
      break;
    case CHR:
      charState(dataIndex);
      break;
    case SHORT_DELAY:
      shortDelayState(stateMillis);
      break;
    case CHAR_DELAY:
      charDelayState(stateMillis);
      break;
    case WORD_DELAY:
      wordDelayState(stateMillis);
      break;
  }
}
void MorseEncoder::send(String message) {
  if (!isBusy) {
    isBusy = true;
    mess = message;
    currentCharIndex = 0;
  }
}
