#include <WiFiClientSecure.h>
#include <Fuzzy.h>
#include <ArduinoJson.h>

#include "config.h"
#include "SensorTDS.h"
#include "SensorKelembapan.h"
#include "SensorSuhuAir.h"
#include "PowerControl.h"
#include "WifiControl.h"
#include "MqttClient.h"
#include "IntervalRunner.h"
#include "PompaControl.h"

#define TOPIC_POWER_COMMAND "power/command"

SensorTDS sensorTds(SensorTDSConfig::TDS_PIN);
SensorKelembapan sensorKelembapan(SensorKelembapanConfig::DHT_PIN);
SensorSuhuAir sensorSuhuAir(SensorSuhuAirConfig::DS18B20_PIN);
PowerControl powerControl(PowerConfig::POWER_PIN);
WifiControl wifiControl(WifiConfig::SSID, WifiConfig::PASSWORD);
WiFiClientSecure espClient;
MqttClient mqttClient( 
  espClient,
  MqttConfig::SERVER, 
  MqttConfig::PORT, 
  MqttConfig::USERNAME, 
  MqttConfig::PASSWORD
);
PompaControl pompaNutrisiControl(PompaConfig::NUTRISI_PIN);
PompaControl pompaPendinginControl(PompaConfig::PENDINGIN_PIN);

// 🔁 Fuzzy engine
Fuzzy* fuzzy = new Fuzzy();

// Data Sensor
float tdsValue = 0;
float kelembapanValue = 0;
float suhuAirValue = 0;

String pompaNutrisi = "OFF";
String pompaPendingin = "OFF";

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

void publishData() {
  // Membuat objek JSON Sensor
  JsonDocument docSensor;
  
  // Menambahkan data ke JSON
  docSensor["tds"] = tdsValue;
  docSensor["suhu_air"] = suhuAirValue;
  docSensor["kelembapan"] = kelembapanValue;
  
  // Serialisasi JSON
  char sensorBuffer[200];
  serializeJson(docSensor, sensorBuffer);

  // Membuat objek JSON Pompa
  JsonDocument docPompa;
  
  // Menambahkan data ke JSON
  docPompa["pompa_nutrisi"] = pompaNutrisi;
  docPompa["pompa_air_dingin"] = pompaPendingin;

  // Serialisasi JSON
  char pompaBuffer[200];
  serializeJson(docPompa, pompaBuffer);

  // Mengirim data ke MQTT
  mqttClient.publish("sensor", sensorBuffer);
  mqttClient.publish("pompa", pompaBuffer);
}

// Send data ke MQTT
void sendData() {
  if (!powerControl.getState()) {
    tdsValue = 0;
    kelembapanValue = 0;
    suhuAirValue = 0;

    // Matikan pompa jika power mati
    pompaNutrisiControl.turnOff();
    pompaPendinginControl.turnOff();
    pompaNutrisi = "OFF";
  } else {
    tdsValue = sensorTds.readTDSPpm();
    kelembapanValue = sensorKelembapan.readHumidity();
    suhuAirValue = sensorSuhuAir.readTemperature();

    // Fuzzy
    fuzzy->setInput(1, tdsValue);
    fuzzy->setInput(2, suhuAirValue);
    fuzzy->setInput(3, kelembapanValue);
    fuzzy->fuzzify();

    float outputNutrisi = fuzzy->defuzzify(1);
    float outputPendingin = fuzzy->defuzzify(2);

    // Implementasi output (bisa disesuaikan dengan PWM)
    // pompaControl.setSpeed(outputNutrisi); // Jika mendukung PWM
    // Untuk pendingin bisa diimplementasikan sesuai kebutuhan

    if (outputNutrisi > 50) {
      pompaNutrisi = "ON";
      pompaNutrisiControl.turnOn();
    } else {
      pompaNutrisi = "OFF";
      pompaNutrisiControl.turnOff();
    }

    if (outputPendingin > 50) {
      pompaPendingin = "ON";
      // Implementasi pendingin, misalnya menghidupkan kipas
    } else {
      pompaPendingin = "OFF";
      // Implementasi pendingin, misalnya mematikan kipas
    }

    mqttClient.publish("test", String(outputPendingin).c_str());
  }

  publishData();
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

  // Pompa
  pompaNutrisiControl.begin();
  pompaPendinginControl.begin();

  // WiFi
  wifiControl.connect();

  // MQTT
  espClient.setCACert(MqttConfig::CA_CERT);
  mqttClient.begin();
  mqttClient.setCallback(callback);
  mqttClient.setSubscribe(TOPIC_POWER_COMMAND);

  // =======================
  // 📥 INPUT TDS (ppm) - Khusus Sawi
  // =======================
  FuzzyInput* tds = new FuzzyInput(1);

  // Range TDS optimal untuk sawi:
  // - Fase awal (1-2 minggu): 400-600 ppm
  // - Fase pertumbuhan: 800-1000 ppm
  // - Fase dewasa: 1000-1400 ppm
  FuzzySet* tdsRendah = new FuzzySet(0, 0, 400, 600);      // Untuk tanaman muda
  FuzzySet* tdsSedang = new FuzzySet(500, 700, 800, 1000); // Pertumbuhan vegetatif
  FuzzySet* tdsTinggi = new FuzzySet(900, 1100, 1300, 1500); // Tanaman dewasa
  FuzzySet* tdsSangatTinggi = new FuzzySet(1400, 1600, 2000, 2000); // Batas atas

  tds->addFuzzySet(tdsRendah);
  tds->addFuzzySet(tdsSedang);
  tds->addFuzzySet(tdsTinggi);
  tds->addFuzzySet(tdsSangatTinggi);
  fuzzy->addFuzzyInput(tds);

  // =======================
  // 📥 INPUT SUHU AIR (°C) - Khusus Sawi
  // =======================
  FuzzyInput* suhu = new FuzzyInput(2);

  // Suhu optimal untuk sawi hidroponik:
  FuzzySet* suhuDingin = new FuzzySet(0, 10, 15, 18);    // Terlalu dingin (<18°C)
  FuzzySet* suhuOptimal = new FuzzySet(17, 20, 22, 25);  // Optimal 20-25°C
  FuzzySet* suhuPanas = new FuzzySet(24, 26, 30, 50);    // Terlalu panas (>26°C)

  suhu->addFuzzySet(suhuDingin);
  suhu->addFuzzySet(suhuOptimal);
  suhu->addFuzzySet(suhuPanas);
  fuzzy->addFuzzyInput(suhu);

  // =======================
  // 📥 INPUT KELEMBAPAN (%) - Khusus Sawi
  // =======================
  FuzzyInput* kelembapan = new FuzzyInput(3);

  // Kelembapan optimal untuk sawi:
  FuzzySet* humKering = new FuzzySet(0, 40, 50, 60);     // Terlalu kering (<60%)
  FuzzySet* humOptimal = new FuzzySet(55, 65, 75, 85);    // Optimal 60-80%
  FuzzySet* humLembap = new FuzzySet(80, 90, 95, 100);    // Terlalu lembap (>85%)

  kelembapan->addFuzzySet(humKering);
  kelembapan->addFuzzySet(humOptimal);
  kelembapan->addFuzzySet(humLembap);
  fuzzy->addFuzzyInput(kelembapan);

  // =======================
  // 📤 OUTPUT Pompa Nutrisi (%) - Khusus Sawi
  // =======================
  FuzzyOutput* nutrisi = new FuzzyOutput(1);
  FuzzySet* nutOff = new FuzzySet(0, 0, 0, 10);          // Mati total
  FuzzySet* nutLow = new FuzzySet(5, 20, 30, 50);        // Dosis rendah (20-50%)
  FuzzySet* nutMedium = new FuzzySet(40, 60, 70, 80);    // Dosis sedang (60-80%)
  FuzzySet* nutHigh = new FuzzySet(75, 90, 100, 100);    // Dosis tinggi (90-100%)

  nutrisi->addFuzzySet(nutOff);
  nutrisi->addFuzzySet(nutLow);
  nutrisi->addFuzzySet(nutMedium);
  nutrisi->addFuzzySet(nutHigh);
  fuzzy->addFuzzyOutput(nutrisi);

  // =======================
  // 📤 OUTPUT Pompa Pendingin (%) - Khusus Sawi
  // =======================
  FuzzyOutput* pendingin = new FuzzyOutput(2);
  FuzzySet* fanOff = new FuzzySet(0, 0, 0, 30);          // Mati
  FuzzySet* fanLow = new FuzzySet(20, 40, 50, 70);       // Kecepatan rendah (40-70%)
  FuzzySet* fanHigh = new FuzzySet(60, 80, 100, 100);    // Kecepatan tinggi (80-100%)

  pendingin->addFuzzySet(fanOff);
  pendingin->addFuzzySet(fanLow);
  pendingin->addFuzzySet(fanHigh);
  fuzzy->addFuzzyOutput(pendingin);

  // =======================
  // 📜 RULES KHUSUS SAWI
  // =======================

  // 1. Jika TDS rendah -> Nutrisi tinggi (untuk percepat pertumbuhan)
  FuzzyRuleAntecedent *jikaTdsRendah = new FuzzyRuleAntecedent();
  jikaTdsRendah->joinSingle(tdsRendah);
  FuzzyRuleConsequent *makaNutrisiTinggi = new FuzzyRuleConsequent();
  makaNutrisiTinggi->addOutput(nutHigh);
  FuzzyRule *fuzzyRule1 = new FuzzyRule(1, jikaTdsRendah, makaNutrisiTinggi);
  fuzzy->addFuzzyRule(fuzzyRule1);

  // 2. Jika TDS sedang -> Nutrisi sedang (pertumbuhan stabil)
  FuzzyRuleAntecedent *jikaTdsSedang = new FuzzyRuleAntecedent();
  jikaTdsSedang->joinSingle(tdsSedang);
  FuzzyRuleConsequent *makaNutrisiSedang = new FuzzyRuleConsequent();
  makaNutrisiSedang->addOutput(nutMedium);
  FuzzyRule *fuzzyRule2 = new FuzzyRule(2, jikaTdsSedang, makaNutrisiSedang);
  fuzzy->addFuzzyRule(fuzzyRule2);

  // 3. Jika TDS tinggi -> Nutrisi rendah (cukup maintenance)
  FuzzyRuleAntecedent *jikaTdsTinggi = new FuzzyRuleAntecedent();
  jikaTdsTinggi->joinSingle(tdsTinggi);
  FuzzyRuleConsequent *makaNutrisiRendah = new FuzzyRuleConsequent();
  makaNutrisiRendah->addOutput(nutLow);
  FuzzyRule *fuzzyRule3 = new FuzzyRule(3, jikaTdsTinggi, makaNutrisiRendah);
  fuzzy->addFuzzyRule(fuzzyRule3);

  // 4. Jika TDS sangat tinggi -> Matikan nutrisi (hindari overnutrisi)
  FuzzyRuleAntecedent *jikaTdsSangatTinggi = new FuzzyRuleAntecedent();
  jikaTdsSangatTinggi->joinSingle(tdsSangatTinggi);
  FuzzyRuleConsequent *makaNutrisiMati = new FuzzyRuleConsequent();
  makaNutrisiMati->addOutput(nutOff);
  FuzzyRule *fuzzyRule4 = new FuzzyRule(4, jikaTdsSangatTinggi, makaNutrisiMati);
  fuzzy->addFuzzyRule(fuzzyRule4);

  // 5. Jika suhu panas -> Pendingin tinggi
  FuzzyRuleAntecedent *jikaSuhuPanas = new FuzzyRuleAntecedent();
  jikaSuhuPanas->joinSingle(suhuPanas);
  FuzzyRuleConsequent *makaPendinginTinggi = new FuzzyRuleConsequent();
  makaPendinginTinggi->addOutput(fanHigh);
  FuzzyRule *fuzzyRule5 = new FuzzyRule(5, jikaSuhuPanas, makaPendinginTinggi);
  fuzzy->addFuzzyRule(fuzzyRule5);

  // 6. Jika suhu optimal -> Pendingin rendah
  FuzzyRuleAntecedent *jikaSuhuOptimal = new FuzzyRuleAntecedent();
  jikaSuhuOptimal->joinSingle(suhuOptimal);
  FuzzyRuleConsequent *makaPendinginLow = new FuzzyRuleConsequent();
  makaPendinginLow->addOutput(fanLow);
  FuzzyRule *fuzzyRule6 = new FuzzyRule(6, jikaSuhuOptimal, makaPendinginLow);
  fuzzy->addFuzzyRule(fuzzyRule6);

  // 7. Jika suhu dingin -> Matikan pendingin
  FuzzyRuleAntecedent *jikaSuhuDingin = new FuzzyRuleAntecedent();
  jikaSuhuDingin->joinSingle(suhuDingin);
  FuzzyRuleConsequent *makaPendinginMati = new FuzzyRuleConsequent();
  makaPendinginMati->addOutput(fanOff);
  FuzzyRule *fuzzyRule7 = new FuzzyRule(7, jikaSuhuDingin, makaPendinginMati);
  fuzzy->addFuzzyRule(fuzzyRule7);

  // 8. Jika suhu panas DAN kelembapan rendah -> Pendingin maksimum
  FuzzyRuleAntecedent *jikaPanasDanKering = new FuzzyRuleAntecedent();
  jikaPanasDanKering->joinWithAND(suhuPanas, humKering);
  FuzzyRuleConsequent *makaPendinginMaks = new FuzzyRuleConsequent();
  makaPendinginMaks->addOutput(fanHigh);
  FuzzyRule *fuzzyRule8 = new FuzzyRule(8, jikaPanasDanKering, makaPendinginMaks);
  fuzzy->addFuzzyRule(fuzzyRule8);

  // 9. Jika suhu panas DAN kelembapan tinggi -> Pendingin sedang
  FuzzyRuleAntecedent *jikaPanasDanLembap = new FuzzyRuleAntecedent();
  jikaPanasDanLembap->joinWithAND(suhuPanas, humLembap);
  FuzzyRuleConsequent *makaPendinginSedang = new FuzzyRuleConsequent();
  makaPendinginSedang->addOutput(fanLow);
  FuzzyRule *fuzzyRule9 = new FuzzyRule(9, jikaPanasDanLembap, makaPendinginSedang);
  fuzzy->addFuzzyRule(fuzzyRule9);

  // 10. Jika TDS rendah DAN suhu tinggi -> Nutrisi tinggi + pendingin tinggi
  FuzzyRuleAntecedent *jikaTdsRendahDanSuhuTinggi = new FuzzyRuleAntecedent();
  jikaTdsRendahDanSuhuTinggi->joinWithAND(tdsRendah, suhuPanas);
  FuzzyRuleConsequent *makaNutrisiTinggiPendinginTinggi = new FuzzyRuleConsequent();
  makaNutrisiTinggiPendinginTinggi->addOutput(nutHigh);
  makaNutrisiTinggiPendinginTinggi->addOutput(fanHigh);
  FuzzyRule *fuzzyRule10 = new FuzzyRule(10, jikaTdsRendahDanSuhuTinggi, makaNutrisiTinggiPendinginTinggi);
  fuzzy->addFuzzyRule(fuzzyRule10);
}

void loop() {
  if (!mqttClient.isConnect()) {
    mqttClient.reconnect();
  }

  mqttClient.loop();

  sendDataRunner.update();
}