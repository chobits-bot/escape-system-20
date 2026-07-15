#pragma once

#include <Arduino.h>
#include "pins.h"

// 单色LED点阵/数码管 驱动 (74HC595类移位寄存器)
class LedMatrix {
public:
    void begin() {
        pinMode(PIN_LED_MATRIX_CLK, OUTPUT);
        pinMode(PIN_LED_MATRIX_LAT, OUTPUT);
        pinMode(PIN_LED_MATRIX_OE,  OUTPUT);
        pinMode(PIN_LED_MATRIX_D,   OUTPUT);
        digitalWrite(PIN_LED_MATRIX_OE, HIGH); // OE高=禁用输出
        _data = 0;
    }

    void enable()  { digitalWrite(PIN_LED_MATRIX_OE, LOW);  }
    void disable() { digitalWrite(PIN_LED_MATRIX_OE, HIGH); }

    void writeByte(uint8_t b) {
        for (int i = 7; i >= 0; i--) {
            digitalWrite(PIN_LED_MATRIX_D, (b >> i) & 1);
            digitalWrite(PIN_LED_MATRIX_CLK, HIGH);
            digitalWrite(PIN_LED_MATRIX_CLK, LOW);
        }
    }

    void latch() {
        digitalWrite(PIN_LED_MATRIX_LAT, HIGH);
        digitalWrite(PIN_LED_MATRIX_LAT, LOW);
    }

    void setData(uint8_t* data, uint8_t len) {
        for (uint8_t i = 0; i < len; i++) {
            writeByte(data[i]);
        }
        latch();
    }

    void clear() {
        writeByte(0x00);
        latch();
    }

private:
    uint8_t _data;
};
