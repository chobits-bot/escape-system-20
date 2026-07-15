#ifndef SIGNBOARD_H
#define SIGNBOARD_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "sensor_manager.h"

class SignboardController {
public:
    SignboardController(SensorManager* sensorMgr);
    
    void begin();
    void processCommand(String jsonPayload);

private:
    SensorManager* sensor;
    
    void handleLED(int value);
    void handleBuzzer(int value);
    void handleFan(String value);
    void handleDisplay(String position, String content);
    void handleRGB(String position, String color);
};

#endif
