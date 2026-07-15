#include "hardware_init.h"

// ==================== 全局变量定义 ====================
DHT dht(PIN_DHT, DHT22);
bool buzzerState = false;
bool fanState = false;

// ==================== 串口初始化 ====================
void initSerial() {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n[INIT] Serial OK - 115200bps");
}

// ==================== GPIO 初始化 ====================
void initGPIO() {
    pinMode(PIN_BUZZER, OUTPUT);
    pinMode(PIN_FAN, OUTPUT);
    pinMode(PIN_LED_STATUS, OUTPUT);
    
    pinMode(PIN_FLAME, INPUT);
    pinMode(PIN_CURRENT, INPUT);
    
    digitalWrite(PIN_BUZZER, LOW);
    digitalWrite(PIN_FAN, HIGH);  // 风扇默认关闭(高电平截止)
    digitalWrite(PIN_LED_STATUS, LOW);
    
    Serial.println("[INIT] GPIO OK");
    Serial.printf("  PWM LED:  GPIO%d\n", PIN_PWM_LED);
    Serial.printf("  Buzzer:   GPIO%d\n", PIN_BUZZER);
    Serial.printf("  Fan:      GPIO%d\n", PIN_FAN);
    Serial.printf("  Status:   GPIO%d\n", PIN_LED_STATUS);
    Serial.printf("  Flame:    GPIO%d\n", PIN_FLAME);
    Serial.printf("  Current:  GPIO%d (ADC)\n", PIN_CURRENT);
    Serial.printf("  DHT:      GPIO%d\n", PIN_DHT);
}

// ==================== PWM 初始化 ====================
void initPWM() {
    ledcSetup(PWM_CHANNEL_LED, PWM_FREQ_LED, PWM_RESOLUTION_LED);
    ledcAttachPin(PIN_PWM_LED, PWM_CHANNEL_LED);
    ledcWrite(PWM_CHANNEL_LED, 255);  // P-MOS: 255=截止=灯灭
    
    Serial.println("[INIT] PWM OK - LED strip ready");
}

// ==================== ADC 初始化 ====================
void initADC() {
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
    
    Serial.println("[INIT] ADC OK");
    Serial.printf("  Current ADC: GPIO%d (0-3.3V range)\n", PIN_CURRENT);
}

// ==================== DHT 初始化 ====================
void initDHT() {
    dht.begin();
    Serial.println("[INIT] DHT22 OK");
}

// ==================== LED 控制 ====================
// P-MOS: 低电平导通，高电平截止
// 占空比越大，MOS截止时间越长，灯越暗
// 占空比越小，MOS导通时间越长，灯越亮
void setLED(bool state) {
    ledcWrite(PWM_CHANNEL_LED, state ? 0 : 255);
    Serial.printf("[LED] %s\n", state ? "ON" : "OFF");
}

void setLEDBrightness(uint8_t brightness) {
    // P-MOS反向控制: 255-brightness
    uint8_t pwm = 255 - brightness;
    ledcWrite(PWM_CHANNEL_LED, pwm);
    Serial.printf("[LED] Brightness: %d/255 (%d%%)\n", brightness, (brightness * 100) / 255);
}

// ==================== 蜂鸣器控制 ====================
void setBuzzer(bool state) {
    digitalWrite(PIN_BUZZER, state ? HIGH : LOW);
    buzzerState = state;
    Serial.printf("[BUZZER] %s\n", state ? "ON" : "OFF");
}

// ==================== 风扇控制 ====================
// P-MOS 或 PNP 驱动: 低电平导通, 高电平截止
void setFan(bool state) {
    digitalWrite(PIN_FAN, state ? LOW : HIGH);
    fanState = state;
    Serial.printf("[FAN] %s (GPIO%d=%s)\n", state ? "ON" : "OFF", PIN_FAN, state ? "LOW" : "HIGH");
}

// ==================== 硬件测试 ====================
void hardwareTest() {
    Serial.println("\n========== HARDWARE TEST ==========");
    
    Serial.println("\n[TEST] LED strip...");
    setLEDBrightness(128);
    delay(500);
    setLEDBrightness(0);
    Serial.println("[TEST] LED OK");
    
    Serial.println("\n[TEST] Buzzer...");
    setBuzzer(true);
    delay(200);
    setBuzzer(false);
    Serial.println("[TEST] Buzzer OK");
    
    Serial.println("\n[TEST] Fan...");
    setFan(true);
    delay(500);
    setFan(false);
    Serial.println("[TEST] Fan OFF");
    
    Serial.println("\n[TEST] Status LED...");
    digitalWrite(PIN_LED_STATUS, HIGH);
    delay(500);
    digitalWrite(PIN_LED_STATUS, LOW);
    Serial.println("[TEST] Status LED OK");
    
    Serial.println("\n[TEST] DHT22...");
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (isnan(t) || isnan(h)) {
        Serial.println("[TEST] DHT22 read FAILED!");
    } else {
        Serial.printf("[TEST] DHT22 OK - Temp: %.1fC, Hum: %.1f%%\n", t, h);
    }
    
    Serial.println("\n[TEST] Flame sensor...");
    int flameState = digitalRead(PIN_FLAME);
    Serial.printf("[TEST] Flame sensor: %s\n", flameState == LOW ? "DETECTED" : "Normal");
    
    Serial.println("\n[TEST] Current sensor...");
    int rawCurrent = analogRead(PIN_CURRENT);
    float voltage = (rawCurrent / 4095.0) * 3.3;
    Serial.printf("[TEST] Current ADC: %d, Voltage: %.2fV\n", rawCurrent, voltage);
    
    Serial.println("\n========== TEST COMPLETE ==========\n");
}

// ==================== 完整初始化 ====================
void initAll() {
    Serial.println("\n==============================");
    Serial.println("  Ceiling Light 4.0 - Hardware Init");
    Serial.println("==============================\n");
    
    initSerial();
    initGPIO();
    initPWM();
    initADC();
    initDHT();
    
    Serial.println("\n[INIT] All hardware initialized!");
    Serial.println("[INIT] Running hardware test...\n");
    
    hardwareTest();
}
