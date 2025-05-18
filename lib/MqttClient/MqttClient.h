#pragma once
#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>

// Tipe data callback untuk menerima pesan MQTT
typedef void (*MessageCallback)(String message);

class MqttClient
{
    private:
        const char* server;
        int port;
        const char* username;
        const char* password;
        PubSubClient client;
        MessageCallback messageCallback;
        const char* topic;


    public:
        void onMessageReceived(char* topic, byte* payload, unsigned int length);
        /**
         * Konstruktor
         * @param wifiClient WiFiClient
         * @param server MQTT server
         * @param port MQTT port
         * @param username MQTT username
         * @param password MQTT password
         */
        MqttClient(WiFiClient& wifiClient, const char* server, int port, const char* username = "", const char* password = "");

        /**
         * Mengembalikan PubSubClient
         * @return PubSubClient
         */
        PubSubClient& getClient();
        
        /**
         * Mengatur callback untuk menerima pesan MQTT
         * @param callback Fungsi callback
         * @return void
         */
        void setCallback(MessageCallback callback);

        /**
         * Memproses pesan MQTT
         * @return void
         */
        void loop();

        /**
         * Menghubungkan kembali ke MQTT
         * @return void
         */
        void reconnect();

        /**
         * Mengatur topik MQTT
         * @param topic Topik MQTT
         */
        void setSubscribe(const char* topic);

        /**
         * Memeriksa apakah terhubung ke MQTT
         * @return bool
         */
        bool isConnect();

        void begin();

        void publish(const char* topic, const char* message);
};