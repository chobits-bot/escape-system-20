#include "mqtt_handler.h"
#include "config.h"

static MQTTHandler* mqttHandlerInstance = nullptr;

MQTTHandler::MQTTHandler() : mqtt(espClient), newCommand(false) {
    mqttHandlerInstance = this;
}

void MQTTHandler::begin() {
    macAddress = getMacAddress();
    clientId = String(MQTT_CLIENT_ID) + "-" + macAddress;
    clientId.replace(":", "");
    
    mqtt.setServer(MQTT_BROKER, MQTT_PORT);
    mqtt.setCallback(callback);
    mqtt.setBufferSize(1024);
    
    Serial.printf("[MQTT] Client ID: %s\n", clientId.c_str());
    Serial.printf("[MQTT] MAC: %s\n", macAddress.c_str());
}

void MQTTHandler::update() {
    if (!mqtt.connected()) {
        reconnect();
    }
    mqtt.loop();
}

void MQTTHandler::reconnect() {
    if (mqtt.connected()) return;
    
    Serial.print("[MQTT] Connecting to broker...");
    
    if (mqtt.connect(clientId.c_str(), MQTT_USERNAME, MQTT_PASSWORD)) {
        Serial.println(" connected!");
        
        mqtt.subscribe(TOPIC_SIGNBOARD_CONTROL);
        Serial.printf("[MQTT] Subscribed to: %s\n", TOPIC_SIGNBOARD_CONTROL);
        
        mqtt.subscribe(TOPIC_SENSOR_DATA);
        Serial.printf("[MQTT] Subscribed to: %s\n", TOPIC_SENSOR_DATA);
    } else {
        Serial.printf(" failed (rc=%d), retrying in 5s\n", mqtt.state());
        delay(5000);
    }
}

bool MQTTHandler::isConnected() {
    return mqtt.connected();
}

void MQTTHandler::publishSensorData(float temp, float hum, float aqi, float tvoc, float eco2,
                                     float current, bool currentFlag, bool fireFlag, int buzzer) {
    if (!mqtt.connected()) return;
    
    JsonDocument doc;
    JsonObject data = doc[macAddress].to<JsonObject>();
    
    data["temp"] = temp;
    data["hum"] = hum;
    data["AQI"] = aqi;
    data["TVOC"] = tvoc;
    data["ECO2"] = eco2;
    data["electricity"] = current;
    data["electricity_Current"] = currentFlag;
    data["ESP32_fires_flag"] = fireFlag;
    data["ESP32_electricity_flag"] = currentFlag;
    data["buzzer"] = buzzer;
    
    char buffer[512];
    serializeJson(doc, buffer);
    
    if (mqtt.publish(TOPIC_SENSOR_DATA, buffer)) {
        Serial.printf("[MQTT] Published sensor data: %s\n", buffer);
    } else {
        Serial.println("[MQTT] Failed to publish sensor data");
    }
}

void MQTTHandler::publishHeartbeat() {
    if (!mqtt.connected()) return;
    
    JsonDocument doc;
    doc["board_mac"] = macAddress;
    doc["timestamp"] = millis() / 1000;
    
    JsonArray signboards = doc["signboards"].to<JsonArray>();
    signboards.add(macAddress);
    
    char buffer[256];
    serializeJson(doc, buffer);
    
    if (mqtt.publish(TOPIC_SIGNBOARD_HEARTBEAT, buffer)) {
        Serial.printf("[MQTT] Published heartbeat: %s\n", buffer);
    } else {
        Serial.println("[MQTT] Failed to publish heartbeat");
    }
}

void MQTTHandler::callback(char* topic, byte* payload, unsigned int length) {
    if (mqttHandlerInstance) {
        String message;
        for (unsigned int i = 0; i < length; i++) {
            message += (char)payload[i];
        }
        Serial.printf("[MQTT] Received [%s]: %s\n", topic, message.c_str());
        
        if (String(topic) == TOPIC_SIGNBOARD_CONTROL) {
            mqttHandlerInstance->handleControlMessage(message);
        }
    }
}

void MQTTHandler::handleControlMessage(String message) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, message);
    
    if (error) {
        Serial.printf("[MQTT] JSON parse error: %s\n", error.c_str());
        return;
    }
    
    if (doc[macAddress].is<JsonObject>()) {
        newCommand = true;
        lastCommand = message;
        Serial.println("[MQTT] Received control command for this device");
    }
}

bool MQTTHandler::hasNewCommand() {
    return newCommand;
}

String MQTTHandler::getLastCommand() {
    newCommand = false;
    return lastCommand;
}

String MQTTHandler::getMacAddress() {
    return WiFi.macAddress();
}
