#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

class MQTTHandler {
public:
    MQTTHandler();
    
    void begin();
    void update();
    void reconnect();
    
    bool isConnected();
    
    // 发布传感器数据
    void publishSensorData(float temp, float hum, float aqi, float tvoc, float eco2,
                           float current, bool currentFlag, bool fireFlag, int buzzer);
    
    // 发布心跳
    void publishHeartbeat();
    
    // 处理控制消息
    void processControlMessage(String payload);
    
    // 获取控制指令
    bool hasNewCommand();
    String getLastCommand();

private:
    WiFiClient espClient;
    PubSubClient mqtt;
    String clientId;
    String macAddress;
    
    bool newCommand;
    String lastCommand;
    
    static void callback(char* topic, byte* payload, unsigned int length);
    void handleControlMessage(String message);
    
    String getMacAddress();
};

#endif
