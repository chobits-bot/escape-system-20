#pragma once

#include <Arduino.h>
#include "pins.h"

class FanControl {
public:
    void begin() {
        pinMode(PIN_FAN, OUTPUT);
        off();
    }

    void on()  { digitalWrite(PIN_FAN, HIGH); _running = true; }
    void off() { digitalWrite(PIN_FAN, LOW);  _running = false; }
    bool isRunning() const { return _running; }

    void toggle() { _running ? off() : on(); }

private:
    bool _running = false;
};
