#include "apds.h"
#include <Adafruit_NeoPixel.h>
#include "Adafruit_APDS9960.h"

const uint16_t NEO_BRIGHTNESS = 20;
const uint16_t PROX_THRESHOLD = 10;
const uint16_t STABLE_DURATION = 100;
const uint16_t SWIPE_THRESHOLD = 60;
const uint16_t SWIPE_DURATION = 100;
const uint16_t STABLE_SWIPE_DURATION = 200;
const uint16_t SWIPE_WINDOW = 40;

enum GestureState { IDLE, HAND_DETECTED, SWIPE_TOWARDS, SWIPE_AWAY };

const uint16_t NUM_NEOPIXEL = 1;
Adafruit_NeoPixel strip(NUM_NEOPIXEL, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
Adafruit_APDS9960 apds;

GestureState currentState = IDLE;
GestureState previousState = IDLE;
uint32_t stateStartTime = 0;
uint16_t proximityHistory[SWIPE_WINDOW / 10];
uint8_t historyIndex = 0;

void setLedColor(uint32_t color, uint16_t proximity) {
  strip.fill(color);
  strip.setBrightness(proximity * NEO_BRIGHTNESS / 256);
  strip.show();
}

void blinkErrorLed() {
  Serial.println("Failed to initialize device! Please check your wiring.");
  while (1) {
    setLedColor(0xFF0000, NEO_BRIGHTNESS);
    delay(100);
    setLedColor(0x000000, NEO_BRIGHTNESS);
    delay(100);
  }
}

bool isSwipeTowards() {
  uint16_t oldestProximity = proximityHistory[historyIndex];
  uint16_t newestProximity = proximityHistory[(historyIndex - 1 + SWIPE_WINDOW / 10) % (SWIPE_WINDOW / 10)];
  return newestProximity > oldestProximity + SWIPE_THRESHOLD;
}

bool isSwipeAway() {
  uint16_t oldestProximity = proximityHistory[historyIndex];
  uint16_t newestProximity = proximityHistory[(historyIndex - 1 + SWIPE_WINDOW / 10) % (SWIPE_WINDOW / 10)];
  return newestProximity < oldestProximity - SWIPE_THRESHOLD;
}

void logStateChange(uint16_t proximity) {
  Serial.print("Proximity: ");
  Serial.println(proximity);

  switch (currentState) {
    case IDLE:
      Serial.println("State changed to IDLE");
      break;
    case HAND_DETECTED:
      Serial.println("State changed to HAND_DETECTED");
      break;
    case SWIPE_TOWARDS:
      Serial.println("State changed to SWIPE_TOWARDS");
      break;
    case SWIPE_AWAY:
      Serial.println("State changed to SWIPE_AWAY");
      break;
  }
}

void handleIdleState(uint16_t proximity) {
  setLedColor(0x000000, proximity);  // Off
  if (proximity > PROX_THRESHOLD) {
    currentState = HAND_DETECTED;
    stateStartTime = millis();
  }
}

void handleHandDetectedState(uint16_t proximity) {
  setLedColor(0x800080, proximity);  // Purple
  if (proximity <= PROX_THRESHOLD) {
    currentState = IDLE;
  } else if (millis() - stateStartTime >= STABLE_DURATION) {
    if (isSwipeTowards()) {
      currentState = SWIPE_TOWARDS;
      stateStartTime = millis();
    } else if (isSwipeAway()) {
      currentState = SWIPE_AWAY;
      stateStartTime = millis();
    }
  }
}

void handleSwipeState(uint16_t proximity) {
  setLedColor(currentState == SWIPE_TOWARDS ? 0xFF8000 : 0x00FF00, proximity);
  
  // delay(SWIPE_DURATION);
  
  if (proximity <= PROX_THRESHOLD) {
    currentState = IDLE;
  } else if (millis() - stateStartTime >= STABLE_SWIPE_DURATION) {
    currentState = HAND_DETECTED;
    stateStartTime = millis();
  }
  return;
}

void _setup_apds() {
  strip.begin();
  strip.show();
  if (!apds.begin()) blinkErrorLed();

  Serial.println("APDS initialized!");
  apds.enableProximity(true);
  apds.setProxGain(APDS9960_PGAIN_8X);
  apds.setLED(APDS9960_LEDDRIVE_100MA, APDS9960_LEDBOOST_300PCNT);
  apds.setProxPulse(APDS9960_PPULSELEN_16US, 1);
}

void handleGestureState(uint16_t proximity) {
  proximityHistory[historyIndex] = proximity;
  historyIndex = (historyIndex + 1) % (SWIPE_WINDOW / 10);

  previousState = currentState;

  switch (currentState) {
    case IDLE:
      handleIdleState(proximity);
      break;
    case HAND_DETECTED:
      handleHandDetectedState(proximity);
      break;
    case SWIPE_TOWARDS:
    case SWIPE_AWAY:
      handleSwipeState(proximity);
      break;
  }

  if (currentState != previousState) {
    logStateChange(proximity);
  }
}

uint16_t _apds_proximity = 0;
uint16_t _apds_get_proximity() {
  return 255 - _apds_proximity;
}

void _loop_apds() {
  _apds_proximity = apds.readProximity();
  handleGestureState(_apds_proximity);
}