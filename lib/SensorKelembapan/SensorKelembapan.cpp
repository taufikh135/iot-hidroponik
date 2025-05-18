#include "SensorKelembapan.h"

SensorKelembapan::SensorKelembapan(uint8_t PIN_DATA): dht(PIN_DATA, DHT22) {
  
}

void SensorKelembapan::begin() {
  dht.begin();
}

float SensorKelembapan::readTemperature() {
  float temperature = dht.readTemperature();
  return isnan(temperature) ? NAN : temperature;
}

float SensorKelembapan::readHumidity() {
  float humidity = dht.readHumidity();
  return isnan(humidity) ? NAN : humidity;
}