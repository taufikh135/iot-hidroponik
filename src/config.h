// WiFi Config
namespace WifiConfig {
    const char* SSID = "Redmi Note 8";
    const char* PASSWORD = "ewedulu123";
}

// MQTT Config
namespace MqttConfig {
    const char* SERVER = "192.168.249.135";
    const int PORT = 1884;
    const char* TOPIC = "test";
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

// Sensor pH
namespace PhSensorConfig {
    const int PH_PIN = 32;
    const int TEMP_PIN = 33;
}