#pragma once
#include <Arduino.h>

class PhSensor
{
    private:
        uint8_t phPin;
        uint8_t tempPin;
        float voltage;
        float temperature;
        float phValue;

        float readVoltage();
        
    public:
        PhSensor(const uint8_t phPin, const uint8_t tempPin);
        void begin();
        float readTemperature();
        float readPhValue();
};