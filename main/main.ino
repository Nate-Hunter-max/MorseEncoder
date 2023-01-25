uint8_t BUZZER_PIN = 13;
uint16_t DotDelay = 100;
#define MORSE_SERIAL_OUTPUT false

uint8_t dataArr[]{
  0b01,
  0b1000,
  0b1010,
  0b100,
  0b0,
  0b0010,
  0b110,
  0b0000,
  0b00,
  0b0111,
  0b101,
  0b0100,
  0b11,
  0b10,
  0b111,
  0b0110,
  0b1101,
  0b010,
  0b000,
  0b1,
  0b001,
  0b0001,
  0b011,
  0b1001,
  0b1011,
  0b1100,
  0b11111,
  0b01111,
  0b00111,
  0b00011,
  0b00001,
  0b00000,
  0b10000,
  0b11000,
  0b11100,
  0b11110,
};
const uint8_t lenArr[] = { 2, 4, 4, 3, 1, 4, 3, 4, 2, 4, 3, 4, 2, 2, 3, 4, 4, 3, 3, 1, 3, 4, 3, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5 };
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

void mainState(const char* mess, uint16_t& size, uint16_t& dataIndex) {
  if (state != lastState) { lastState = state; }
  static uint16_t currentCharIndex;
  if (currentCharIndex < size) {
    if (mess[currentCharIndex] == ' ') state = WORD_DELAY;
    else {
      dataIndex = mess[currentCharIndex] - (mess[currentCharIndex] < 57 ? 22 : 97);
      state = CHR;
    }
    currentCharIndex++;
  }
}

void charState(uint16_t& dataIndex) {
  static int bitIndex;
  if (state != lastState) {
    if (lastState == MAIN) bitIndex = lenArr[dataIndex];
    lastState = state;
  }
  if (bitIndex > 0) {
    bitIndex--;
    state = bitRead(dataArr[dataIndex], bitIndex) ? DASH : DOT;
  } else state = CHAR_DELAY;
}

void dotState(uint16_t& startMillis) {
  if (state != lastState) {
    lastState = state;
    startMillis = millis();
    if (MORSE_SERIAL_OUTPUT) Serial.print("DOT");
    digitalWrite(BUZZER_PIN, 0x01);
  }
  if (millis() - startMillis >= DotDelay) {
    digitalWrite(BUZZER_PIN, 0x00);
    state = SHORT_DELAY;
  }
};

void dashState(uint16_t& startMillis) {
  if (state != lastState) {
    lastState = state;
    startMillis = millis();
    if (MORSE_SERIAL_OUTPUT) Serial.print("DASH");
    digitalWrite(BUZZER_PIN, 0x01);
  }
  if (millis() - startMillis >= DotDelay * 3) {
    digitalWrite(BUZZER_PIN, 0x00);
    state = SHORT_DELAY;
  }
}

void shortDelayState(uint16_t& startMillis) {
  if (state != lastState) {
    lastState = state;
    startMillis = millis();
    if (MORSE_SERIAL_OUTPUT) Serial.print("-");
  }
  if (millis() - startMillis >= DotDelay) state = CHR;
}

void charDelayState(uint16_t& startMillis) {
  if (state != lastState) {
    lastState = state;
    startMillis = millis();
    if (MORSE_SERIAL_OUTPUT) Serial.print(">");
  }
  if (millis() - startMillis >= DotDelay << 1) state = MAIN;
}

void wordDelayState(uint16_t& startMillis) {
  if (state != lastState) {
    lastState = state;
    startMillis = millis();
    if (MORSE_SERIAL_OUTPUT) Serial.print("==");
  }
  if (millis() - startMillis >= DotDelay << 2) state = MAIN;
}
void setup() {
  if (MORSE_SERIAL_OUTPUT) Serial.begin(9600);
  pinMode(BUZZER_PIN, OUTPUT);
  state = MAIN;
};

void loop() {
  static uint16_t stateMillis;
  static uint16_t dataIndex;
  char mess[] = "hello world";
  uint16_t size = sizeof(mess) / sizeof(char) - 1;
  switch (state) {
    case MAIN:
      mainState(mess, size, dataIndex);
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