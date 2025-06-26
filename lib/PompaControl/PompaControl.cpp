#include "PompaControl.h"

PompaControl::PompaControl(int pin) {
    this->pin = pin;
}

void PompaControl::begin() {
    pinMode(this->pin, OUTPUT);
    this->turnOff(); // Pastikan pompa dimatikan saat inisialisasi
}

void PompaControl::turnOn() {
    digitalWrite(this->pin, HIGH);
    this->isOnVar = true; // Set status pompa menyala
}


void PompaControl::turnOff() {
    digitalWrite(this->pin, LOW);
    this->isOnVar = false; // Set status pompa mati
}

bool PompaControl::isOn() {
    return this->isOnVar; // Kembalikan status pompa
}