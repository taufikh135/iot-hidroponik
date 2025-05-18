#pragma once
#include <OneWire.h>
#include <DallasTemperature.h>

// Sensor DS18B20
class SensorSuhuAir
{
  private:
    DallasTemperature sensor;
    OneWire oneWire;
    
  public:
    SensorSuhuAir(uint8_t dataPin);
    void begin();
    float readTemperature();
};