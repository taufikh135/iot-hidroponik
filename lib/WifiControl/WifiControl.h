#pragma once
#include <Arduino.h>
#include <WiFi.h>

class WifiControl 
{
    private:
        const char* ssid;
        const char* password;
        WiFiClient client;

    public:
        /**
         * Konstruktor
         * @param ssid SSID WiFi
         * @param password Password WiFi
         */
        WifiControl(const char* ssid, const char* password);

        /**
         * Menghubungkan ke WiFi
         */
        void connect();

        /**
         * Mengembalikan WiFiClient
         * @return WiFiClient
         */
        WiFiClient& getClient();

        /**
         * Mengembalikan status WiFi
         * @return true jika WiFi terhubung, false jika tidak
         */
        bool isConnect();
};