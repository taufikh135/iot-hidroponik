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
int tds = 0;
int kelembapan = 0;
int suhu = 0;

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
  docSensor["tds"] = tds;
  docSensor["suhu_air"] = suhu;
  docSensor["kelembapan"] = kelembapan;
  
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
    tds = 0;
    kelembapan = 0;
    suhu = 0;

    // Matikan pompa jika power mati
    pompaNutrisiControl.turnOff();
    pompaPendinginControl.turnOff();
    pompaNutrisi = "OFF";
  } else {
    tds = (int) sensorTds.readTDSPpm();
    kelembapan = (int) sensorKelembapan.readHumidity();
    suhu = (int) sensorSuhuAir.readTemperature();

    // Fuzzy
    fuzzy->setInput(1, tds);
    fuzzy->setInput(2, suhu);
    fuzzy->setInput(3, kelembapan);
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
  // 📥 INPUT TDS (ppm)
  // =======================
  FuzzyInput* tds = new FuzzyInput(1);

  // Range untuk tanaman hidroponik umum:
  FuzzySet* tdsVeryLow = new FuzzySet(0, 0, 150, 300);    // Untuk tanaman muda/stek
  FuzzySet* tdsLow = new FuzzySet(250, 350, 400, 450);    // Tanaman pertumbuhan vegetatif
  FuzzySet* tdsMedium = new FuzzySet(400, 450, 475, 500); // Tanaman berbunga/berbuah
  FuzzySet* tdsHigh = new FuzzySet(475, 500, 500, 500);   // Batas atas 500ppm

  tds->addFuzzySet(tdsVeryLow);
  tds->addFuzzySet(tdsLow);
  tds->addFuzzySet(tdsMedium);
  tds->addFuzzySet(tdsHigh);
  fuzzy->addFuzzyInput(tds);

  // =======================
  // 📥 INPUT SUHU AIR (°C)
  // =======================
  FuzzyInput* suhu = new FuzzyInput(2);

  // Suhu optimal untuk hidroponik:
  FuzzySet* suhuDingin = new FuzzySet(0, 10, 15, 18);    // Terlalu dingin
  FuzzySet* suhuOptimal = new FuzzySet(17, 20, 22, 25);   // Range optimal
  FuzzySet* suhuPanas = new FuzzySet(24, 26, 30, 35);     // Terlalu panas

  suhu->addFuzzySet(suhuDingin);
  suhu->addFuzzySet(suhuOptimal);
  suhu->addFuzzySet(suhuPanas);
  fuzzy->addFuzzyInput(suhu);

  // =======================
  // 📥 INPUT KELEMBAPAN (%)
  // =======================
  FuzzyInput* kelembapan = new FuzzyInput(3);

  // Kelembapan udara untuk hidroponik:
  FuzzySet* humKering = new FuzzySet(0, 30, 40, 50);      // Terlalu kering
  FuzzySet* humOptimal = new FuzzySet(45, 55, 65, 75);    // Range optimal
  FuzzySet* humLembap = new FuzzySet(70, 80, 90, 100);    // Terlalu lembap

  kelembapan->addFuzzySet(humKering);
  kelembapan->addFuzzySet(humOptimal);
  kelembapan->addFuzzySet(humLembap);
  fuzzy->addFuzzyInput(kelembapan);

  // =======================
  // 📤 OUTPUT Pompa Nutrisi (%)
  // =======================
  FuzzyOutput* nutrisi = new FuzzyOutput(1);
  FuzzySet* nutOff = new FuzzySet(0, 0, 0, 10);          // Mati total
  FuzzySet* nutLow = new FuzzySet(5, 20, 30, 50);        // Dosis rendah
  FuzzySet* nutMedium = new FuzzySet(40, 60, 70, 80);    // Dosis sedang
  FuzzySet* nutHigh = new FuzzySet(75, 90, 100, 100);    // Dosis tinggi

  nutrisi->addFuzzySet(nutOff);
  nutrisi->addFuzzySet(nutLow);
  nutrisi->addFuzzySet(nutMedium);
  nutrisi->addFuzzySet(nutHigh);
  fuzzy->addFuzzyOutput(nutrisi);

  // =======================
  // 📤 OUTPUT Pompa Pendingin (%)
  // =======================
  FuzzyOutput* pendingin = new FuzzyOutput(2);
  FuzzySet* fanOff = new FuzzySet(0, 0, 0, 30);          // Mati
  FuzzySet* fanLow = new FuzzySet(20, 40, 50, 70);       // Kecepatan rendah
  FuzzySet* fanHigh = new FuzzySet(60, 80, 100, 100);    // Kecepatan tinggi

  pendingin->addFuzzySet(fanOff);
  pendingin->addFuzzySet(fanLow);
  pendingin->addFuzzySet(fanHigh);
  fuzzy->addFuzzyOutput(pendingin);

  // =======================
  // 📜 RULES YANG BENAR
  // =======================

  // 1. Jika TDS sangat rendah -> Nutrisi tinggi
  FuzzyRuleAntecedent *jikaTdsSangatRendah = new FuzzyRuleAntecedent();
  jikaTdsSangatRendah->joinSingle(tdsVeryLow);
  FuzzyRuleConsequent *makaNutrisiTinggi = new FuzzyRuleConsequent();
  makaNutrisiTinggi->addOutput(nutHigh);
  FuzzyRule *fuzzyRule1 = new FuzzyRule(1, jikaTdsSangatRendah, makaNutrisiTinggi);
  fuzzy->addFuzzyRule(fuzzyRule1);

  // 2. Jika TDS rendah -> Nutrisi sedang
  FuzzyRuleAntecedent *jikaTdsRendah = new FuzzyRuleAntecedent();
  jikaTdsRendah->joinSingle(tdsLow);
  FuzzyRuleConsequent *makaNutrisiSedang = new FuzzyRuleConsequent();
  makaNutrisiSedang->addOutput(nutMedium);
  FuzzyRule *fuzzyRule2 = new FuzzyRule(2, jikaTdsRendah, makaNutrisiSedang);
  fuzzy->addFuzzyRule(fuzzyRule2);

  // 3. Jika TDS medium -> Nutrisi rendah
  FuzzyRuleAntecedent *jikaTdsMedium = new FuzzyRuleAntecedent();
  jikaTdsMedium->joinSingle(tdsMedium);
  FuzzyRuleConsequent *makaNutrisiRendah = new FuzzyRuleConsequent();
  makaNutrisiRendah->addOutput(nutLow);
  FuzzyRule *fuzzyRule3 = new FuzzyRule(3, jikaTdsMedium, makaNutrisiRendah);
  fuzzy->addFuzzyRule(fuzzyRule3);

  // 4. Jika TDS tinggi -> Matikan nutrisi
  FuzzyRuleAntecedent *jikaTdsTinggi = new FuzzyRuleAntecedent();
  jikaTdsTinggi->joinSingle(tdsHigh);
  FuzzyRuleConsequent *makaNutrisiMati = new FuzzyRuleConsequent();
  makaNutrisiMati->addOutput(nutOff);
  FuzzyRule *fuzzyRule4 = new FuzzyRule(4, jikaTdsTinggi, makaNutrisiMati);
  fuzzy->addFuzzyRule(fuzzyRule4);

  // 5. Jika suhu panas -> Pendingin tinggi
  FuzzyRuleAntecedent *jikaSuhuPanas = new FuzzyRuleAntecedent();
  jikaSuhuPanas->joinSingle(suhuPanas);
  FuzzyRuleConsequent *makaPendinginTinggi = new FuzzyRuleConsequent();
  makaPendinginTinggi->addOutput(fanHigh);
  FuzzyRule *fuzzyRule5 = new FuzzyRule(5, jikaSuhuPanas, makaPendinginTinggi);
  fuzzy->addFuzzyRule(fuzzyRule5);

  // Rule 6: Jika TDS rendah DAN suhu panas -> Nutrisi tinggi
  // FuzzyRuleAntecedent *jikaTdsRendahDanSuhuPanas = new FuzzyRuleAntecedent();
  // jikaTdsRendahDanSuhuPanas->joinWithAND(tdsLow, suhuPanas);
  // FuzzyRuleConsequent *makaNutrisiTinggi = new FuzzyRuleConsequent();
  // makaNutrisiTinggi->addOutput(nutHigh);
  // FuzzyRule *fuzzyRule6 = new FuzzyRule(6, jikaTdsRendahDanSuhuPanas, makaNutrisiTinggi);
  // fuzzy->addFuzzyRule(fuzzyRule6);

  // 7. Jika suhu panas DAN kelembapan rendah -> Pendingin maksimum
  FuzzyRuleAntecedent *jikaPanasDanKering = new FuzzyRuleAntecedent();
  jikaPanasDanKering->joinWithAND(suhuPanas, humKering);
  FuzzyRuleConsequent *makaPendinginMaks = new FuzzyRuleConsequent();
  makaPendinginMaks->addOutput(fanHigh);
  FuzzyRule *fuzzyRule7 = new FuzzyRule(7, jikaPanasDanKering, makaPendinginMaks);
  fuzzy->addFuzzyRule(fuzzyRule7);

  // 8. Jika suhu optimal -> Pendingin rendah
  FuzzyRuleAntecedent *jikaSuhuOptimal = new FuzzyRuleAntecedent();
  jikaSuhuOptimal->joinSingle(suhuOptimal);
  FuzzyRuleConsequent *makaPendinginLow = new FuzzyRuleConsequent();
  makaPendinginLow->addOutput(fanLow);
  FuzzyRule *fuzzyRule8 = new FuzzyRule(8, jikaSuhuOptimal, makaPendinginLow);
  fuzzy->addFuzzyRule(fuzzyRule8);

  // 9. Jika suhu dingin -> Matikan pendingin
  FuzzyRuleAntecedent *jikaSuhuDingin = new FuzzyRuleAntecedent();
  jikaSuhuDingin->joinSingle(suhuDingin);
  FuzzyRuleConsequent *makaPendinginMati = new FuzzyRuleConsequent();
  makaPendinginMati->addOutput(fanOff);
  FuzzyRule *fuzzyRule9 = new FuzzyRule(9, jikaSuhuDingin, makaPendinginMati);
  fuzzy->addFuzzyRule(fuzzyRule9);

  // 10. Jika suhu panas DAN kelembapan tinggi -> Pendingin sedang
  FuzzyRuleAntecedent *jikaPanasDanLembap = new FuzzyRuleAntecedent();
  jikaPanasDanLembap->joinWithAND(suhuPanas, humLembap);
  FuzzyRuleConsequent *makaPendinginSedang = new FuzzyRuleConsequent();
  makaPendinginSedang->addOutput(fanLow);
  FuzzyRule *fuzzyRule10 = new FuzzyRule(10, jikaPanasDanLembap, makaPendinginSedang);
  fuzzy->addFuzzyRule(fuzzyRule10);
}

void loop() {
  if (!mqttClient.isConnect()) {
    mqttClient.reconnect();
  }

  mqttClient.loop();

  sendDataRunner.update();
}