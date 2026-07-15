#pragma once

#include <Arduino.h>
#include "pins.h"

class LedDriver {
public:
    void begin() {
        ledcSetup(LED_PWM_CHANNEL, LED_PWM_FREQ_HZ, LED_PWM_RESOLUTION);
        ledcAttachPin(PIN_LED_PWM, LED_PWM_CHANNEL);
        off();
    }

    void setBrightness(uint8_t brightness) {
        _brightness = brightness;
        ledcWrite(LED_PWM_CHANNEL, brightness);
    }

    void off() {
        _brightness = 0;
        ledcWrite(LED_PWM_CHANNEL, 0);
    }

    void on() {
        if (_brightness == 0) _brightness = 255;
        ledcWrite(LED_PWM_CHANNEL, _brightness);
    }

    uint8_t getBrightness() const { return _brightness; }

    void fadeTo(uint8_t target, uint16_t duration_ms) {
        uint8_t start = _brightness;
        int16_t diff = (int16_t)target - (int16_t)start;
        uint16_t steps = duration_ms / 10;
        if (steps < 1) steps = 1;
        float increment = (float)diff / (float)steps;

        for (uint16_t i = 0; i < steps; i++) {
            uint8_t val = (uint8_t)(start + (int16_t)(increment * i));
            ledcWrite(LED_PWM_CHANNEL, val);
            delay(10);
        }
        setBrightness(target);
    }

private:
    uint8_t _brightness = 0;
};
