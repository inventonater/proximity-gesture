#include <Arduino.h>

#define PROXIMITY
#ifdef PROXIMITY
#include "l4cd.h"
#include "apds.h"
#else
#include "gesture.h"
#endif

void setup() {
  Serial.begin(9600);
#ifdef PROXIMITY
  _setup_l4cd();
  _setup_apds();
#else
  _gesture_setup();
#endif
}

void loop() {
  // Serial.print("Time: ");
  // Serial.println(millis());
#ifdef PROXIMITY
  _loop_l4cd();
  _loop_apds();
  Serial.print("Proximity:\t");
  Serial.print(_l4cd_get_proximity());
  Serial.print(", \t");
  Serial.println(_apds_get_proximity());
#else
  _gesture_loop();
#endif
}