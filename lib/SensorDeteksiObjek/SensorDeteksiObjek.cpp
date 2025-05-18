#include "SensorDeteksiObjek.h"

SensorDeteksiObjek::SensorDeteksiObjek(uint8_t trigPin, uint8_t echoPin) {
  this->trigPin = trigPin;
  this->echoPin = echoPin;
}

void SensorDeteksiObjek::begin() {
  pinMode(this->trigPin, OUTPUT);
  pinMode(this->echoPin, INPUT);
}

long SensorDeteksiObjek::readDistance() {
  this->currentMilis = millis();
  
  if (this->previousMilis == 0 || this->currentMilis - this->previousMilis >= this->interval) {
    // Kirim pulsa Trigger selama 10 mikrosekon
    digitalWrite(this->trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(this->trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(this->trigPin, LOW);
    
    // Mengukur waktu pulang Echo
    long duration = pulseIn(echoPin, HIGH);
    
    // Menghitung jarak dalam cm
    this->distance = duration * 0.034 / 2;
    
    this->previousMilis = this->currentMilis;
  }

  return this->distance;
}
