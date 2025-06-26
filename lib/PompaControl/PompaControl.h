#pragma once
#include <Arduino.h>

class PompaControl {
    private:
        int pin; // Pin untuk mengontrol pompa
        bool isOnVar = false; // Status pompa, default mati

    public:
        /**
         * Konstruktor
         * @param pin Pin yang digunakan untuk mengontrol pompa
         */
        PompaControl(int pin);

        /**
         * Inisialisasi pompa
         */
        void begin();

        /**
         * Menghidupkan pompa
         */
        void turnOn();

        /**
         * Mematikan pompa
         */
        void turnOff();

        /**
         * Mengembalikan status pompa
         * @return true jika pompa menyala, false jika tidak
         */
        bool isOn();
};