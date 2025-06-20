#pragma once
#include <Arduino.h>

class IntervalRunner {
  private:
    unsigned long lastRun = 0;
    unsigned long interval;
    void (*callback)();

  public:
    IntervalRunner(void (*cb)(), unsigned long i) {
      callback = cb;
      interval = i;
    }

    void update() {
      if (millis() - lastRun >= interval) {
        lastRun = millis();
        callback();
      }
    }
};
