#pragma once

// WiFi Config
namespace WifiConfig {
    const char* SSID = "aixbwis";
    const char* PASSWORD = "rbetopik";
}

// Sensor Deteksi Objek
namespace SensorDeteksiObjekConfig {
    const int TRIGER_PIN = 4;
    const int ECHO_PIN = 5;
}

// Sensor TDS
namespace SensorTDSConfig {
    const int TDS_PIN = 34;
}

// Sensor Suhu Air
namespace SensorSuhuAirConfig {
    const int DS18B20_PIN = 18;
}

// Sensor Kelembapan
namespace SensorKelembapanConfig {
    const int DHT_PIN = 13;
}

// Power
namespace PowerConfig {
    const int POWER_PIN = 2;
}

// Mqtt Config
namespace MqttConfig {
    const char* SERVER = "192.168.5.241";
    const int PORT = 1884;
    const char* USERNAME = ""; // Ganti dengan username Anda
    const char* PASSWORD = ""; // Ganti dengan password Anda
}