#include "MqttClient.h"

MqttClient::MqttClient(WiFiClient& wifiClient, const char* server, int port, const char* username, const char* password): client(wifiClient) {
    this->server = server;
    this->port = port;
    this->username = username;
    this->password = password;
}

void MqttClient::begin() {
    this->client.setServer(this->server, this->port);
}

PubSubClient& MqttClient::getClient() {
    return this->client;
}

void MqttClient::setCallback(MessageCallback callback) {
    this->messageCallback = callback;
    this->client.setCallback([this](char* topic, byte* payload, unsigned int length) {
        this->onMessageReceived(topic, payload, length);
    });
}

// Method untuk memproses pesan MQTT
void MqttClient::onMessageReceived(char* topic, byte* payload, unsigned int length) {
    // Mengonversi payload char menjadi String
    String message = "";
    for (unsigned int i = 0; i < length; i++) {
      message += (char)payload[i];
    }

    message.trim();

    // Panggil callback dengan pesan yang diterima
    if (this->messageCallback != nullptr) {
      this->messageCallback(message);
    }
}

void MqttClient::loop() {
    this->client.loop();
}

void MqttClient::reconnect() {
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);

    while (!this->isConnect()) {
        Serial.print("Menghubungkan ke MQTT...");
        if (this->client.connect(clientId.c_str(), this->username, this->password)) {
            Serial.println("terhubung");

            if (this->messageCallback != nullptr) {
                this->client.subscribe(this->topic);
            }
        } else {
            Serial.print("gagal, rc=");
            Serial.print(this->client.state());
            delay(2000);
        }
    }
}

void MqttClient::setSubscribe(const char* topic) {
    this->topic = topic;
}

bool MqttClient::isConnect() {
    return this->client.connected();
}

void MqttClient::publish(const char* topic, const char* message) {
    this->client.publish(topic, message);
}