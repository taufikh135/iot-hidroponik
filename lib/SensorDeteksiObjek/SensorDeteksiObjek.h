#pragma once
#include <Arduino.h>

// Sensor HC_SR04
class SensorDeteksiObjek 
{    
  public:
    // Konstruktor untuk inisialisasi pin Trigger dan Echo
    SensorDeteksiObjek(uint8_t trigPin, uint8_t echoPin);
    
    // Fungsi untuk menginisialisasi sensor
    void begin();
    
    // Fungsi untuk membaca jarak
    long readDistance();

  private:
    uint8_t trigPin;
    uint8_t echoPin;
    long distance;
    unsigned long previousMilis = 0;
    const long interval = 500;
    unsigned long currentMilis;
};