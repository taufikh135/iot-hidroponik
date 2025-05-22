#include <ArduinoJson.h>

#include "WifiControl.h"
#include "SensorDeteksiObjek.h"
#include "MqttClient.h"
#include "SensorTDS.h"
#include "SensorSuhuAir.h"
#include "SensorKelembapan.h"
#include "PhSensor.h"

#include "config.h"

WifiControl wifiControl(WIFI_SSID, WIFI_PASSWORD);
SensorDeteksiObjek sensorDeteksiObjek(TRIGGER_PIN, ECHO_PIN);
MqttClient mqttClient(wifiControl.getClient(), MQTT_SERVER, MQTT_PORT);
SensorTDS sensorTDS(TDS_PIN);
SensorSuhuAir sensorSuhuAir(DS18B20_PIN);
SensorKelembapan sensorKelembapan(DHT_PIN);
PhSensor phSensor(PH_PIN, TEMP_PIN);

void callback(String message) {

}

void setup() {
    // Serial Monitor
    Serial.begin(115200);
    // WiFi
    wifiControl.connect();
    // MQTT
    mqttClient.setCallback(callback);
    mqttClient.setSubscribe(MQTT_TOPIC);
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

    mqttClient.publish(MQTT_TOPIC, jsonString.c_str());
}