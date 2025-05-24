#include <ArduinoJson.h>

#include "WifiControl.h"
#include "SensorDeteksiObjek.h"
#include "MqttClient.h"
#include "SensorTDS.h"
#include "SensorSuhuAir.h"
#include "SensorKelembapan.h"
#include "PhSensor.h"

#include "config.h"

WifiControl wifiControl(WifiConfig::SSID, WifiConfig::PASSWORD);
SensorDeteksiObjek sensorDeteksiObjek(SensorDeteksiObjekConfig::TRIGER_PIN, SensorDeteksiObjekConfig::ECHO_PIN);
MqttClient mqttClient(wifiControl.getClient(), MqttConfig::SERVER, MqttConfig::PORT);
SensorTDS sensorTDS(SensorTDSConfig::TDS_PIN);
SensorSuhuAir sensorSuhuAir(SensorSuhuAirConfig::DS18B20_PIN);
SensorKelembapan sensorKelembapan(SensorKelembapanConfig::DHT_PIN);
PhSensor phSensor(PhSensorConfig::PH_PIN, PhSensorConfig::TEMP_PIN);

unsigned long lastSend = 0;
const unsigned long interval = 5000; // Interval pengiriman data (5 detik)

void callback(String message) {

}

void readAndSendData() {
    long jarakObjek = sensorDeteksiObjek.readDistance();
    float suhuAir = sensorSuhuAir.readTemperature();
    float tds = sensorTDS.readTDSPpm();
    float kelembapan = sensorKelembapan.readHumidity();
    float phValue = phSensor.readPhValue();

    StaticJsonDocument<200> doc;
    doc["jarak"] = jarakObjek;
    doc["suhuAir"] = suhuAir;
    doc["tds"] = tds;
    doc["kelembapan"] = kelembapan;
    doc["phValue"] = phValue;
    
    String jsonString;
    serializeJson(doc, jsonString);

    mqttClient.publish(MqttConfig::TOPIC, jsonString.c_str());
}

void setup() {
    // Serial Monitor
    Serial.begin(115200);
    // WiFi
    wifiControl.connect();
    // MQTT
    mqttClient.setCallback(callback);
    mqttClient.setSubscribe(MqttConfig::TOPIC);
    mqttClient.begin();
    // Sensor Deteksi Objek
    sensorDeteksiObjek.begin();
    // Sensor TDS
    sensorTDS.begin();
    // Sensor Suhu Air
    sensorSuhuAir.begin();
    // Sensor Kelembapan
    sensorKelembapan.begin();
    // Sensor pH
    phSensor.begin();
}

void loop() {
    if (!mqttClient.isConnect()) {
        mqttClient.reconnect();
    }

    mqttClient.loop();

    if (millis() - lastSend >= interval) {
        readAndSendData();
        lastSend = millis();
    }
}