#include "PhSensor.h"

PhSensor::PhSensor(const uint8_t phPin, const uint8_t tempPin)
{
    this->phPin = phPin;
    this->tempPin = tempPin;
    this->voltage = 0.0;
    this->temperature = 0.0;
    this->phValue = 0.0;
}

void PhSensor::begin()
{
    pinMode(this->phPin, INPUT);
    pinMode(this->tempPin, INPUT);
}

float PhSensor::readVoltage()
{
    int rawValue = analogRead(this->phPin);
    this->voltage = (rawValue / 1023.0) * 5.0;
    return this->voltage;
}

float PhSensor::readPhValue()
{
    this->voltage = this->readVoltage();
    this->phValue = 7 + ((this->voltage - 2.5) / 0.18); // Example formula
    return this->phValue;
}