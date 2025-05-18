#include "SensorSuhuAir.h"

SensorSuhuAir::SensorSuhuAir(uint8_t dataPin):
  oneWire(dataPin), sensor(&oneWire) 
{
  
}

void SensorSuhuAir::begin() {
  sensor.begin();
}

float SensorSuhuAir::readTemperature() {
  sensor.requestTemperatures();
  float temp = sensor.getTempCByIndex(0);
  return (temp == DEVICE_DISCONNECTED_C) ? NAN : temp;
}