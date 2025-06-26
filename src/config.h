#pragma once

// WiFi Config
namespace WifiConfig {
    const char* SSID = "zan29";
    const char* PASSWORD = "iri12345";
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
    const char* SERVER = "192.168.80.135";
    const int PORT = 1884;
    const char* USERNAME = ""; // Ganti dengan username Anda
    const char* PASSWORD = ""; // Ganti dengan password Anda
}

// Pompa Config
namespace PompaConfig {
    const int NUTRISI_PIN = 15; // Pin untuk mengontrol pompa
    const int PENDINGIN_PIN = 12; // Pin untuk mengontrol pompa
}