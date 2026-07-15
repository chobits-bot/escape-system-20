#pragma once

#include <Arduino.h>
#include "pins.h"

class Buzzer {
public:
    void begin() {
        pinMode(PIN_BUZZER, OUTPUT);
        off();
    }

    void on()  { digitalWrite(PIN_BUZZER, HIGH); }
    void off() { digitalWrite(PIN_BUZZER, LOW); }

    void beep(uint16_t duration_ms) {
        on();
        delay(duration_ms);
        off();
    }

    void beepPattern(const uint16_t pattern[], uint8_t count, uint16_t pause_ms = 100) {
        for (uint8_t i = 0; i < count; i++) {
            if (i % 2 == 0) {
                on();
            } else {
                off();
            }
            delay(pattern[i / 2 < count / 2 ? i : i]);
        }
        off();
    }

    // 简单告警：3短声
    void alarm() {
        for (int i = 0; i < 3; i++) {
            on();  delay(150);
            off(); delay(150);
        }
    }
};
