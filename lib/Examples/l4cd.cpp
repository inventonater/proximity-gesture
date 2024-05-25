/* Includes ------------------------------------------------------------------*/
#include <Arduino.h>
#include <Wire.h>
#include <vl53l4cd_class.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

#define DEV_I2C Wire
VL53L4CD sensor_vl53l4cd_sat(&DEV_I2C, A1);

bool _debug = false;

void _setup_l4cd()
{
  Serial.println("Starting l4cd...");
  DEV_I2C.begin();

  sensor_vl53l4cd_sat.begin();
  sensor_vl53l4cd_sat.VL53L4CD_Off();
  sensor_vl53l4cd_sat.InitSensor();
  sensor_vl53l4cd_sat.VL53L4CD_SetRangeTiming(200, 0);
  sensor_vl53l4cd_sat.VL53L4CD_StartRanging();
}

uint16_t _l4cd_proximity = 0;
uint16_t _l4cd_get_proximity() {
  return _l4cd_proximity;
}

void _loop_l4cd()
{
  uint8_t NewDataReady = 0;
  VL53L4CD_Result_t results;
  uint8_t status;
  char report[64];

  // do {
  //   status = sensor_vl53l4cd_sat.VL53L4CD_CheckForDataReady(&NewDataReady);
  // } while (!NewDataReady);

  status = sensor_vl53l4cd_sat.VL53L4CD_CheckForDataReady(&NewDataReady);

  if ((!status) && (NewDataReady != 0)) {
    // (Mandatory) Clear HW interrupt to restart measurements
    sensor_vl53l4cd_sat.VL53L4CD_ClearInterrupt();

    // Read measured distance. RangeStatus = 0 means valid data
    sensor_vl53l4cd_sat.VL53L4CD_GetResult(&results);
    snprintf(report, sizeof(report), "Status = %3u, Distance = %5u mm, Signal = %6u kcps/spad\r\n",
             results.range_status,
             results.distance_mm,
             results.signal_per_spad_kcps);
    if(_debug) Serial.print(report);

    _l4cd_proximity = results.distance_mm;
  }
}