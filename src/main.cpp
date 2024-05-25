#include <Arduino.h>
#include "l4cd.h"
#include "apds.h"

void setup() {
  Serial.begin(9600);

  _setup_l4cd();
  _setup_apds();
}

void loop() {
  // Serial.print("Time: ");
  // Serial.println(millis());

  _loop_l4cd();
  _loop_apds();

  Serial.print("Proximity:\t");
  Serial.print(_l4cd_get_proximity());
  Serial.print(", \t");
  Serial.println(_apds_get_proximity());
}