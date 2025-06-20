#include "WifiControl.h"

WifiControl::WifiControl(const char* ssid, const char* password) {
    this->ssid = ssid;
    this->password = password;
}

void WifiControl::connect() {
    WiFi.begin(this->ssid, this->password);
    Serial.print("Menghubungkan ke WiFi");

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }

    Serial.println("\nTerhubung ke WiFi!");
    Serial.print("Alamat IP: ");
    Serial.println(WiFi.localIP());
}

bool WifiControl::isConnect() {
    return WiFi.status() == WL_CONNECTED;
}