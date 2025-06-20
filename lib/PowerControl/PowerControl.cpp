#include "PowerControl.h"

PowerControl::PowerControl(int powerPin) : prefs() {
    this->powerPin = powerPin;
    this->isOn = 0;
}

void PowerControl::load() {
    this->prefs.begin(this->ns.c_str(), false);
    this->isOn = this->prefs.getInt(this->key.c_str(), 0);
}

void PowerControl::save() {
    this->prefs.begin(this->ns.c_str(), false);
    this->prefs.putInt(this->key.c_str(), this->isOn);
}

void PowerControl::begin() {
    // Load state
    this->load();
    // Set pin
    pinMode(this->powerPin, OUTPUT);
    // Set initial state
    digitalWrite(this->powerPin, this->isOn);
}

void PowerControl::on() {
    if (!this->isOn) {
        this->isOn = 1;
        digitalWrite(this->powerPin, HIGH);
        this->save();
    }
}

void PowerControl::off() {
    if (this->isOn) {
        this->isOn = 0;
        digitalWrite(this->powerPin, LOW);
        this->save();
    }
}

int PowerControl::getState() {
    return this->isOn;
}