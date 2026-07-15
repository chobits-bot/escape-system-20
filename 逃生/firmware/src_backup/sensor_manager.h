#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <DHT.h>

struct SensorData {
    float temperature;
    float humidity;
    float aqi;
    float tvoc;
    float eco2;
    float current;
    bool electricityCurrent;
    bool firesFlag;
    bool electricityFlag;
    int buzzerState;
};

class SensorManager {
public:
    SensorManager();
    
    void begin();
    void update();
    
    SensorData getData();
    
    void setBuzzer(bool state);
    void setFan(bool state);
    void setLED(bool state);
    void setLEDBrightness(uint8_t brightness);

private:
    DHT dht;
    SensorData data;
    bool buzzerState;
    bool fanState;
    
    void readDHT();
    void readCurrent();
    void readFlame();
};

#endif
