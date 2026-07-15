#pragma once

#include <Arduino.h>
#include <HardwareSerial.h>
#include "pins.h"

class Cat1Module {
public:
    void begin(unsigned long baud = 115200) {
        _serial.begin(baud, SERIAL_8N1, PIN_CAT_RX, PIN_CAT_TX);
        pinMode(PIN_CAT_STATE, INPUT);
    }

    // 发送AT指令并等待响应
    bool sendAT(const char* cmd, const char* expect = "OK", uint32_t timeout_ms = 2000) {
        _serial.println(cmd);
        String resp = readResponse(timeout_ms);
        return resp.indexOf(expect) >= 0;
    }

    // 模块是否在线
    bool isReady() { return sendAT("AT", "OK", 1000); }

    // 检测模块注册状态
    bool isRegistered() {
        _serial.println("AT+CEREG?");
        String resp = readResponse(2000);
        return resp.indexOf(",1") >= 0 || resp.indexOf(",5") >= 0;
    }

    // 读取STATE引脚 (ADC)
    uint16_t readState() { return analogRead(PIN_CAT_STATE); }

    // 获取信号强度
    int getSignalStrength() {
        _serial.println("AT+CSQ");
        String resp = readResponse(2000);
        int idx = resp.indexOf("+CSQ: ");
        if (idx < 0) return -1;
        int val = resp.substring(idx + 6, resp.indexOf('\r', idx)).toInt();
        return (val == 99) ? -1 : val;
    }

    // 通过模块发送TCP数据
    bool sendTCPData(const char* host, uint16_t port, const uint8_t* data, size_t len) {
        char buf[128];
        snprintf(buf, sizeof(buf), "AT+CIPOPEN=0,\"TCP\",\"%s\",%d", host, port);
        if (!sendAT(buf, "CONNECT", 5000)) return false;

        char cmd[64];
        snprintf(cmd, sizeof(cmd), "AT+CIPSEND=0,%d", len);
        if (!sendAT(cmd, ">", 2000)) return false;

        _serial.write(data, len);
        return sendAT("", "SEND OK", 5000);
    }

    void loop() {
        while (_serial.available()) {
            char c = _serial.read();
            _buffer += c;
            if (_buffer.length() > 512) _buffer.remove(0, 256);
        }
    }

    String getBuffer() { return _buffer; }
    void clearBuffer() { _buffer = ""; }

private:
    HardwareSerial _serial = HardwareSerial(2);
    String _buffer;

    String readResponse(uint32_t timeout_ms) {
        String resp = "";
        uint32_t start = millis();
        while (millis() - start < timeout_ms) {
            while (_serial.available()) {
                resp += (char)_serial.read();
            }
            if (resp.length() > 0 && millis() - start > 100) break;
            delay(1);
        }
        return resp;
    }
};
