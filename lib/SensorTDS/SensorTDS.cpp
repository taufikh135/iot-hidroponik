#include "SensorTDS.h"

SensorTDS::SensorTDS(int tdsPin) {
  this->tdsPin = tdsPin;
}

float SensorTDS::readTDSPpm() {
  int rawValue = analogRead(this->tdsPin);
  float voltage = rawValue * (3.3 / 4095.0);  // Untuk ESP32 (ADC 12-bit, 3.3V)
  
  // Rumus dari datasheet Gravity TDS Sensor V1.0
  float tdsValue = (133.42 * voltage * voltage * voltage 
                    - 255.86 * voltage * voltage 
                    + 857.39 * voltage) * 0.5;
  return tdsValue;
}

void SensorTDS::begin() {
  analogReadResolution(12);  // Baca nilai 0–4095
}