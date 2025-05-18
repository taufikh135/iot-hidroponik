#pragma once 
#include <Arduino.h>

class SensorTDS
{
  private:
    int tdsPin;

  public:
    SensorTDS(const int tdsPin);
    float readTDSPpm();
    void begin();
};