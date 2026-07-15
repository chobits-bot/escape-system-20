#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "hardware_init.h"
#include "mqtt_handler.h"

// ==================== 全局对象 ====================
MQTTHandler mqttHandler;

// ==================== 定时器 ====================
unsigned long lastSensorReport = 0;
unsigned long lastHeartbeat = 0;
unsigned long lastWifiCheck = 0;
unsigned long lastSerialCheck = 0;

// ==================== WiFi 连接 ====================
void connectWiFi() {
    if (WiFi.status() == WL_CONNECTED) return;

    Serial.printf("[WIFI] Connecting to %s", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < 30) {
        delay(500);
        Serial.print(".");
        retry++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("\n[WIFI] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("[WIFI] MAC: %s\n", WiFi.macAddress().c_str());
    } else {
        Serial.println("\n[WIFI] Connection FAILED, will retry...");
    }
}

// ==================== 串口命令处理 ====================
void handleSerialCommands() {
    if (!Serial.available()) return;

    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    cmd.toLowerCase();

    if (cmd == "led on") {
        setLED(true);
    } else if (cmd == "led off") {
        setLED(false);
    } else if (cmd.startsWith("led ")) {
        int val = cmd.substring(4).toInt();
        if (val >= 0 && val <= 255) {
            setLEDBrightness(val);
        } else {
            Serial.println("[CMD] Usage: led <0-255>");
        }
    } else if (cmd == "buzzer on") {
        setBuzzer(true);
    } else if (cmd == "buzzer off") {
        setBuzzer(false);
    } else if (cmd == "fan on") {
        setFan(true);
    } else if (cmd == "fan off") {
        setFan(false);
    } else if (cmd == "status") {
        float t = dht.readTemperature();
        float h = dht.readHumidity();
        int rawCurrent = analogRead(PIN_CURRENT);
        float voltage = (rawCurrent / 4095.0) * 3.3;
        int flame = digitalRead(PIN_FLAME);

        Serial.println("\n========== SENSOR STATUS ==========");
        Serial.printf("  Temperature:  %.1f C\n", isnan(t) ? -1 : t);
        Serial.printf("  Humidity:     %.1f %%\n", isnan(h) ? -1 : h);
        Serial.printf("  AQI:          %d\n", ens160.getAQI());
        Serial.printf("  TVOC:         %d ppb\n", ens160.getTVOC());
        Serial.printf("  ECO2:         %d ppm\n", ens160.getECO2());
        Serial.printf("  Flame:        %s\n", flame == LOW ? "DETECTED" : "Normal");
        Serial.printf("  Current ADC:  %d (%.2fV)\n", rawCurrent, voltage);
        Serial.printf("  Buzzer:       %s\n", buzzerState ? "ON" : "OFF");
        Serial.printf("  Fan:          %s\n", fanState ? "ON" : "OFF");
        Serial.printf("  LED Bright:   %d/255\n", ledBrightness);
        Serial.printf("  WiFi:         %s\n", WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
        Serial.printf("  MQTT:         %s\n", mqttHandler.isConnected() ? "Connected" : "Disconnected");
        Serial.printf("  Free Heap:    %d bytes\n", ESP.getFreeHeap());
        Serial.println("====================================\n");
    } else if (cmd == "reboot") {
        Serial.println("[CMD] Rebooting...");
        ESP.restart();
    } else if (cmd == "help") {
        Serial.println("\n========== COMMANDS ==========");
        Serial.println("  led on        - LED开");
        Serial.println("  led off       - LED关");
        Serial.println("  led <0-255>   - LED亮度");
        Serial.println("  buzzer on/off - 蜂鸣器开关");
        Serial.println("  fan on/off    - 风扇开关");
        Serial.println("  status        - 传感器状态");
        Serial.println("  reboot        - 重启");
        Serial.println("  help          - 帮助");
        Serial.println("==============================\n");
    } else if (cmd.length() > 0) {
        Serial.printf("[CMD] Unknown: '%s' (type 'help')\n", cmd.c_str());
    }
}

// ==================== 传感器数据采集 ====================
void collectAndPublishSensorData() {
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    // ENS160 空气质量读取
    uint16_t tvoc = ens160.getTVOC();
    uint16_t eco2 = ens160.getECO2();
    uint8_t aqi = ens160.getAQI();

    // 用温湿度补偿 ENS160
    if (!isnan(t) && !isnan(h)) {
        ens160.setTempAndHum(t, h);
    }

    // 电流检测: ADC原始值 → 电压 (0-3.3V)
    int rawCurrent = analogRead(PIN_CURRENT);
    float currentVoltage = (rawCurrent / 4095.0) * 3.3;
    bool currentFlag = (currentVoltage > CURRENT_THRESHOLD);

    // 火焰检测: 低电平=检测到火焰
    bool fireFlag = (digitalRead(PIN_FLAME) == LOW);

    // 温度告警
    if (!isnan(t) && t > TEMP_THRESHOLD) {
        fireFlag = true;
    }

    // 烟雾/气体告警: TVOC过高或CO2过高判定为烟雾
    if (tvoc > TVOC_THRESHOLD || eco2 > ECO2_THRESHOLD) {
        fireFlag = true;
        Serial.printf("[ALERT] Smoke detected! TVOC=%d ppb, ECO2=%d ppm\n", tvoc, eco2);
    }

    float temp = isnan(t) ? 0 : t;
    float hum = isnan(h) ? 0 : h;

    // MQTT发布
    mqttHandler.publishSensorData(
        temp, hum,
        aqi, tvoc, eco2,
        currentVoltage,
        currentFlag,
        fireFlag,
        buzzerState ? 1 : 0
    );

    // 状态指示灯: MQTT连接时常亮
    digitalWrite(PIN_LED_STATUS, mqttHandler.isConnected() ? LOW : HIGH);
}

// ==================== 处理MQTT控制指令 ====================
void processControlCommand(String payload) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
        Serial.printf("[CTRL] JSON parse error: %s\n", error.c_str());
        return;
    }

    String mac = WiFi.macAddress();
    if (!doc[mac].is<JsonObject>()) return;

    JsonObject cmd = doc[mac];
    Serial.println("[CTRL] Processing command...");

    // LED亮度 (1-1024映射到0-255)
    if (cmd["led"].is<int>()) {
        int ledVal = cmd["led"].as<int>();
        // led=1024最亮, led=1最暗; 映射到PWM 0-255
        uint8_t brightness = map(ledVal, 1, 1024, 0, 255);
        setLEDBrightness(brightness);
    }

    // 蜂鸣器
    if (cmd["buzzer"].is<int>()) {
        setBuzzer(cmd["buzzer"].as<int>() == 1);
    }

    // 风扇 (electricity_switch: "ON"/"OFF")
    if (cmd["electricity_switch"].is<String>()) {
        setFan(cmd["electricity_switch"].as<String>() == "ON");
    }

    // 火焰检测开关
    if (cmd["flame"].is<String>()) {
        Serial.printf("[CTRL] Flame detect: %s\n", cmd["flame"].as<String>().c_str());
    }

    // 显示内容 (预留, 无硬件)
    if (cmd["display_left"].is<String>()) {
        Serial.printf("[CTRL] Display left: %s\n", cmd["display_left"].as<String>().c_str());
    }
    if (cmd["display_mid"].is<String>()) {
        Serial.printf("[CTRL] Display mid: %s\n", cmd["display_mid"].as<String>().c_str());
    }
    if (cmd["display_right"].is<String>()) {
        Serial.printf("[CTRL] Display right: %s\n", cmd["display_right"].as<String>().c_str());
    }

    // RGB颜色 (预留, 无硬件)
    if (cmd["RGB_left"].is<String>()) {
        Serial.printf("[CTRL] RGB left: %s\n", cmd["RGB_left"].as<String>().c_str());
    }
    if (cmd["RGB_mid"].is<String>()) {
        Serial.printf("[CTRL] RGB mid: %s\n", cmd["RGB_mid"].as<String>().c_str());
    }
    if (cmd["RGB_right"].is<String>()) {
        Serial.printf("[CTRL] RGB right: %s\n", cmd["RGB_right"].as<String>().c_str());
    }

    Serial.println("[CTRL] Command processed");
}

// ==================== setup ====================
void setup() {
    // 1. 硬件初始化 (串口/GPIO/PWM/ADC/DHT)
    initAll();

    // 2. WiFi连接
    connectWiFi();

    // 3. MQTT初始化
    mqttHandler.begin();

    // 4. 打印启动信息
    Serial.println("\n==============================");
    Serial.println("  Ceiling Light 4.0 - RUNNING");
    Serial.println("  Firmware: v4.0.0-production");
    Serial.println("  Board: ESP32-WROOM-32UE-N16");
    Serial.printf("  MAC: %s\n", WiFi.macAddress().c_str());
    Serial.printf("  Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.println("==============================\n");
}

// ==================== loop ====================
void loop() {
    unsigned long now = millis();

    // WiFi保活
    if (now - lastWifiCheck >= WIFI_CHECK_INTERVAL) {
        lastWifiCheck = now;
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("[WIFI] Disconnected, reconnecting...");
            connectWiFi();
        }
    }

    // MQTT保活 + 消息处理
    mqttHandler.update();

    // 传感器数据采集 + MQTT上报 (每30秒)
    if (now - lastSensorReport >= SENSOR_REPORT_INTERVAL) {
        lastSensorReport = now;
        if (mqttHandler.isConnected()) {
            collectAndPublishSensorData();
        }
    }

    // 心跳上报 (每60秒)
    if (now - lastHeartbeat >= HEARTBEAT_INTERVAL) {
        lastHeartbeat = now;
        if (mqttHandler.isConnected()) {
            mqttHandler.publishHeartbeat();
        }
    }

    // 处理MQTT控制指令
    if (mqttHandler.hasNewCommand()) {
        String cmd = mqttHandler.getLastCommand();
        processControlCommand(cmd);
    }

    // 串口命令 (每100ms检查)
    if (now - lastSerialCheck >= SERIAL_CHECK_INTERVAL) {
        lastSerialCheck = now;
        handleSerialCommands();
    }

    // 首次MQTT连接成功后立即上报一次
    static bool firstReport = false;
    if (!firstReport && mqttHandler.isConnected()) {
        firstReport = true;
        delay(1000);
        collectAndPublishSensorData();
        mqttHandler.publishHeartbeat();
        Serial.println("[SYS] First data reported");
    }

    delay(10);
}
