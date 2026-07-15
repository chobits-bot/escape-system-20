#include <Arduino.h>
#include <Wire.h>

#include "pins.h"
#include "led_driver.h"
#include "buzzer.h"
#include "fan_control.h"
#include "cat1_module.h"
#include "led_matrix.h"

LedDriver   ledDriver;
Buzzer      buzzer;
FanControl  fan;
Cat1Module  cat1;
LedMatrix   ledMatrix;

// 状态指示灯
static bool statusLedState = false;

void setupStatusLed() {
    pinMode(PIN_LED_STATUS, OUTPUT);
    digitalWrite(PIN_LED_STATUS, LOW);
}

void toggleStatusLed() {
    statusLedState = !statusLedState;
    digitalWrite(PIN_LED_STATUS, statusLedState ? LOW : HIGH); // 阴极驱动，LOW=亮
}

// ADC传感器读取
struct SensorData {
    float voltageVP;
    float voltageVN;
};

SensorData readSensors() {
    SensorData data;
    uint32_t rawVP = analogRead(PIN_SENSOR_VP);
    uint32_t rawVN = analogRead(PIN_SENSOR_VN);
    // ESP32 ADC 12bit (0~4095), 参考电压3.3V
    data.voltageVP = (rawVP / 4095.0f) * 3.3f;
    data.voltageVN = (rawVN / 4095.0f) * 3.3f;
    return data;
}

// 自动亮度控制 (根据传感器ADC值调节LED)
void autoBrightnessControl(uint16_t threshold, uint8_t minBrightness, uint8_t maxBrightness) {
    SensorData sensors = readSensors();
    float vp = sensors.voltageVP;
    // 简单映射: 0~3.3V → min~max亮度
    uint8_t brightness = (uint8_t)(minBrightness + (vp / 3.3f) * (maxBrightness - minBrightness));
    ledDriver.setBrightness(brightness);
}

// 串口命令解析 (调试用)
void processSerialCommand() {
    if (!Serial.available()) return;

    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "led_on") {
        ledDriver.on();
        Serial.println("[LED] ON");
    }
    else if (cmd == "led_off") {
        ledDriver.off();
        Serial.println("[LED] OFF");
    }
    else if (cmd.startsWith("led ")) {
        int val = cmd.substring(4).toInt();
        ledDriver.setBrightness(constrain(val, 0, 255));
        Serial.printf("[LED] Brightness: %d\n", val);
    }
    else if (cmd == "fan_on") {
        fan.on();
        Serial.println("[FAN] ON");
    }
    else if (cmd == "fan_off") {
        fan.off();
        Serial.println("[FAN] OFF");
    }
    else if (cmd == "beep") {
        buzzer.beep(200);
        Serial.println("[BUZZER] beep");
    }
    else if (cmd == "alarm") {
        buzzer.alarm();
        Serial.println("[BUZZER] alarm");
    }
    else if (cmd == "sensor") {
        SensorData s = readSensors();
        Serial.printf("[SENSOR] VP=%.2fV  VN=%.2fV\n", s.voltageVP, s.voltageVN);
    }
    else if (cmd == "cat1") {
        Serial.printf("[CAT1] Ready=%d  Signal=%d  State=%d\n",
                      cat1.isReady(), cat1.getSignalStrength(), cat1.readState());
    }
    else if (cmd == "matrix_on") {
        ledMatrix.enable();
        Serial.println("[MATRIX] ON");
    }
    else if (cmd == "matrix_off") {
        ledMatrix.disable();
        Serial.println("[MATRIX] OFF");
    }
    else if (cmd == "help") {
        Serial.println("=== 吸顶灯4.0 调试命令 ===");
        Serial.println("led_on / led_off      - LED开关");
        Serial.println("led <0-255>           - LED亮度");
        Serial.println("fan_on / fan_off      - 风扇开关");
        Serial.println("beep / alarm          - 蜂鸣器");
        Serial.println("sensor                - 读取传感器");
        Serial.println("cat1                  - Cat1模块状态");
        Serial.println("matrix_on/matrix_off  - 点阵使能");
    }
}

// ======================== setup ========================
void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n===== 吸顶灯 4.0 =====");

    // 状态指示灯
    setupStatusLed();
    toggleStatusLed(); // 开机闪烁

    // I2C初始化
    Wire.begin(PIN_SDA, PIN_SCL);
    Serial.println("[I2C] Bus initialized");

    // LED调光
    ledDriver.begin();
    Serial.println("[LED] PWM driver ready");

    // 蜂鸣器
    buzzer.begin();
    Serial.println("[BUZZER] Ready");

    // 风扇
    fan.begin();
    Serial.println("[FAN] Ready");

    // Cat1 4G模块
    cat1.begin(115200);
    if (cat1.isReady()) {
        Serial.println("[CAT1] Module online");
    } else {
        Serial.println("[CAT1] Module offline");
    }

    // LED点阵
    ledMatrix.begin();
    Serial.println("[MATRIX] Ready");

    // 开机提示
    buzzer.beep(100);
    toggleStatusLed();

    // 默认: LED开到50%亮度
    ledDriver.setBrightness(128);

    Serial.println("[SYS] Boot complete. Type 'help' for commands.");
}

// ======================== loop ========================
static unsigned long lastSensorRead = 0;
static unsigned long lastCat1Check  = 0;
static unsigned long lastStatusBlink = 0;
const unsigned long SENSOR_INTERVAL   = 1000;  // 1秒读一次传感器
const unsigned long CAT1_INTERVAL     = 30000; // 30秒检查一次模块
const unsigned long BLINK_INTERVAL    = 1000;  // 1秒闪烁状态灯

void loop() {
    unsigned long now = millis();

    // 串口调试命令
    processSerialCommand();

    // Cat1模块接收数据
    cat1.loop();

    // 周期性传感器读取
    if (now - lastSensorRead >= SENSOR_INTERVAL) {
        lastSensorRead = now;
        // autoBrightnessControl(100, 10, 255); // 取消注释启用自动亮度
    }

    // 周期性检查Cat1模块
    if (now - lastCat1Check >= CAT1_INTERVAL) {
        lastCat1Check = now;
        if (cat1.isReady()) {
            Serial.printf("[CAT1] Signal: %d\n", cat1.getSignalStrength());
        }
    }

    // 状态指示灯闪烁
    if (now - lastStatusBlink >= BLINK_INTERVAL) {
        lastStatusBlink = now;
        toggleStatusLed();
    }
}
