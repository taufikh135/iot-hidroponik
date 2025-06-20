#pragma once
#include <Preferences.h>

class PowerControl {
    private:
        int powerPin;
        int isOn;
        Preferences prefs;
        String ns = "power";
        String key = "isOn";

        void save();
        void load();

    public:
        PowerControl(int powerPin = 2);
        void begin();
        void on();
        void off();
        int getState();
};