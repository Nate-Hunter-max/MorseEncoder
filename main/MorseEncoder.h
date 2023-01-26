/*
   ######################################################################
   #    MorseEncoder.h - Library for generating Morse code using FSM    #
   #            Created by Nate E. Hunter, January 26, 2023.            #
   #                  Released into the public domain.                  #
   ###################################################################### 
*/
#pragma once
#include "morseCode.h"
class MorseEncoderFSM {
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
  char* arrPtr;
  uint16_t arrLen;
  uint16_t currentCharIndex;
  bool isBusy, signal;
  uint8_t OutputPin;
  uint16_t DotDelay;
  void updateSignal(bool);

  void mainState(uint16_t&);
  void charState(uint16_t&);
  void dotState(uint32_t&);
  void dashState(uint32_t&);
  void shortDelayState(uint32_t&);
  void charDelayState(uint32_t&);
  void wordDelayState(uint32_t&);

public:
  void begin(uint16_t, uint8_t);
  void update(bool);
  void send(char*, uint16_t);
  bool getSignal();
};

void MorseEncoderFSM::mainState(uint16_t& dataIndex) {
  if (state != lastState) {
    lastState = state;
    if (currentCharIndex == arrLen) {
      state = WORD_DELAY;
      currentCharIndex++;
    } else if (currentCharIndex == arrLen + 1) {
#ifdef MORSE_SERIAL_OUTPUT
      Serial.println();
#endif
      isBusy = false;
    };
  }
  if (currentCharIndex < arrLen) {
    if (arrPtr[currentCharIndex] == ' ') state = WORD_DELAY;
    else {
      dataIndex = arrPtr[currentCharIndex] - (arrPtr[currentCharIndex] < 57 ? 22 : 97);
      state = CHR;
    }
    currentCharIndex++;
  }
}

void MorseEncoderFSM::charState(uint16_t& dataIndex) {
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
void MorseEncoderFSM::updateSignal(bool newState) {
  signal = newState;
#ifdef MORSE_PIN_OUTPUT
  digitalWrite(OutputPin, signal);
#endif
}
void MorseEncoderFSM::dotState(uint32_t& startMillis) {
  if (state != lastState) {
    lastState = state;
    startMillis = millis();
#ifdef MORSE_SERIAL_OUTPUT
    Serial.print("DOT");
#endif
    updateSignal(0x1);
  }
  if (millis() - startMillis >= DotDelay) {
    updateSignal(0x0);
    state = SHORT_DELAY;
  }
};

void MorseEncoderFSM::dashState(uint32_t& startMillis) {
  if (state != lastState) {
    lastState = state;
    startMillis = millis();
#ifdef MORSE_SERIAL_OUTPUT
    Serial.print("DASH");
#endif
    updateSignal(0x1);
  }
  if (millis() - startMillis >= DotDelay * 3) {
    updateSignal(0x0);
    state = SHORT_DELAY;
  }
}

void MorseEncoderFSM::shortDelayState(uint32_t& startMillis) {
  if (state != lastState) {
    lastState = state;
    startMillis = millis();
#ifdef MORSE_SERIAL_OUTPUT
    Serial.print("-");
#endif
  }
  if (millis() - startMillis >= DotDelay) state = CHR;
}

void MorseEncoderFSM::charDelayState(uint32_t& startMillis) {
  if (state != lastState) {
    lastState = state;
    startMillis = millis();
#ifdef MORSE_SERIAL_OUTPUT
    Serial.print(">");
#endif
  }
  if (millis() - startMillis >= DotDelay << 1) state = MAIN;
}

void MorseEncoderFSM::wordDelayState(uint32_t& startMillis) {
  if (state != lastState) {
    lastState = state;
    startMillis = millis();
#ifdef MORSE_SERIAL_OUTPUT
    Serial.print("==");
#endif
  }
  if (millis() - startMillis >= DotDelay << 2) state = MAIN;
}
void MorseEncoderFSM::begin(uint16_t speed = 100, uint8_t pin = 13) {
  state = MAIN;
  OutputPin = pin;
  DotDelay = speed;
#ifdef MORSE_PIN_OUTPUT
  pinMode(OutputPin, OUTPUT);
#endif
}

void MorseEncoderFSM::update(bool newMess = false) {
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
void MorseEncoderFSM::send(char* charArr, uint16_t size) {
  if (!isBusy) {
    isBusy = true;
    arrPtr = charArr;
    arrLen = size - 1;
    currentCharIndex = 0;
  }
}
bool MorseEncoderFSM::getSignal() {
  return signal;
};
extern MorseEncoderFSM MorseEncoder = MorseEncoderFSM();
