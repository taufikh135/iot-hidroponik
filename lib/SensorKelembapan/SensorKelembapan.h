#pragma once
#include <DHT.h>

// Sensor DHT22
class SensorKelembapan
{
  private:
    DHT dht;

  public:
    SensorKelembapan(uint8_t PIN_DATA);
    void begin();
    float readHumidity();
    float readTemperature();
};