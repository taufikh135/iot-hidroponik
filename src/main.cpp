#include <WiFi.h>

#include "config.h"
#include "SensorTDS.h"
#include "SensorKelembapan.h"
#include "SensorSuhuAir.h"
#include "PowerControl.h"
#include "WifiControl.h"
#include "MqttClient.h"
#include "IntervalRunner.h"

#define TOPIC_POWER_COMMAND "power/command"

SensorTDS sensorTds(SensorTDSConfig::TDS_PIN);
SensorKelembapan sensorKelembapan(SensorKelembapanConfig::DHT_PIN);
SensorSuhuAir sensorSuhuAir(SensorSuhuAirConfig::DS18B20_PIN);
PowerControl powerControl(PowerConfig::POWER_PIN);
WifiControl wifiControl(WifiConfig::SSID, WifiConfig::PASSWORD);
WiFiClient espClient;
MqttClient mqttClient( 
  espClient,
  MqttConfig::SERVER, 
  MqttConfig::PORT, 
  MqttConfig::USERNAME, 
  MqttConfig::PASSWORD
);

// Data Sensor
float tds = 0;
float kelembapan = 0;
float suhu = 0;

// Callback untuk menerima pesan MQTT
void callback(String message) {
  int command = message.toInt();

  if (command == 1) {
    powerControl.on(); // Nyalakan power
  } else if (command == 0) {
    powerControl.off(); // Matikan power
  } else {
    Serial.println("Perintah tidak dikenali: " + message);
  }
}

// Send data ke MQTT
void sendData() {
  if (!powerControl.getState()) {
    // Jika power mati, set nilai sensor ke 0
    tds = 0;
    kelembapan = 0;
    suhu = 0;
  } else {
    // Membaca data sensor
    tds = sensorTds.readTDSPpm();
    kelembapan = sensorKelembapan.readHumidity();
    suhu = sensorSuhuAir.readTemperature();
  }

  // Mengirim data ke Blynk
  mqttClient.publish("sensor/tds", String(tds).c_str());
  mqttClient.publish("sensor/kelembapan", String(kelembapan).c_str());
  mqttClient.publish("sensor/suhu-air", String(suhu).c_str());
}

IntervalRunner sendDataRunner(sendData, 1000); // Set interval to 1 detik

void setup() {  
  // Serial
  Serial.begin(115200);
  // Power
  powerControl.begin();
  // Begin sensor
  sensorTds.begin();
  sensorKelembapan.begin();
  sensorSuhuAir.begin();
  // WiFi
  wifiControl.connect();
  // MQTT
  mqttClient.begin();
  mqttClient.setCallback(callback);
  mqttClient.setSubscribe(TOPIC_POWER_COMMAND);
}

void loop() {
  if (!mqttClient.isConnect()) {
    mqttClient.reconnect();
  }

  mqttClient.loop();

  sendDataRunner.update();
}