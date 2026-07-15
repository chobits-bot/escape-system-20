#include "signboard.h"
#include "config.h"
#include <WiFi.h>
#include <ArduinoJson.h>

SignboardController::SignboardController(SensorManager* sensorMgr) : sensor(sensorMgr) {
}

void SignboardController::begin() {
    Serial.println("[SIGNBOARD] Initialized");
}

void SignboardController::processCommand(String jsonPayload) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonPayload);
    
    if (error) {
        Serial.printf("[SIGNBOARD] JSON parse error: %s\n", error.c_str());
        return;
    }
    
    String mac = WiFi.macAddress();
    
    if (!doc[mac].is<JsonObject>()) {
        Serial.println("[SIGNBOARD] Command not for this device");
        return;
    }
    
    JsonObject cmd = doc[mac];
    Serial.println("[SIGNBOARD] Processing command:");
    
    if (cmd["led"].is<int>()) {
        handleLED(cmd["led"].as<int>());
    }
    
    if (cmd["buzzer"].is<int>()) {
        handleBuzzer(cmd["buzzer"].as<int>());
    }
    
    if (cmd["electricity_switch"].is<String>()) {
        handleFan(cmd["electricity_switch"].as<String>());
    }
    
    if (cmd["display_left"].is<String>()) {
        handleDisplay("left", cmd["display_left"].as<String>());
    }
    if (cmd["display_mid"].is<String>()) {
        handleDisplay("mid", cmd["display_mid"].as<String>());
    }
    if (cmd["display_right"].is<String>()) {
        handleDisplay("right", cmd["display_right"].as<String>());
    }
    
    if (cmd["RGB_left"].is<String>()) {
        handleRGB("left", cmd["RGB_left"].as<String>());
    }
    if (cmd["RGB_mid"].is<String>()) {
        handleRGB("mid", cmd["RGB_mid"].as<String>());
    }
    if (cmd["RGB_right"].is<String>()) {
        handleRGB("right", cmd["RGB_right"].as<String>());
    }
    
    Serial.println("[SIGNBOARD] Command processed");
}

void SignboardController::handleLED(int value) {
    if (value == 1) {
        sensor->setLED(true);
        Serial.println("[SIGNBOARD] LED ON");
    } else {
        sensor->setLED(false);
        Serial.println("[SIGNBOARD] LED OFF");
    }
}

void SignboardController::handleBuzzer(int value) {
    if (value == 1) {
        sensor->setBuzzer(true);
        Serial.println("[SIGNBOARD] Buzzer ON");
    } else {
        sensor->setBuzzer(false);
        Serial.println("[SIGNBOARD] Buzzer OFF");
    }
}

void SignboardController::handleFan(String value) {
    if (value == "ON") {
        sensor->setFan(true);
        Serial.println("[SIGNBOARD] Fan ON");
    } else {
        sensor->setFan(false);
        Serial.println("[SIGNBOARD] Fan OFF");
    }
}

void SignboardController::handleDisplay(String position, String content) {
    Serial.printf("[SIGNBOARD] Display %s: %s\n", position.c_str(), content.c_str());
}

void SignboardController::handleRGB(String position, String color) {
    Serial.printf("[SIGNBOARD] RGB %s: %s\n", position.c_str(), color.c_str());
}
