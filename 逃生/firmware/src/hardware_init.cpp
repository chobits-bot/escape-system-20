#include "hardware_init.h"
#include "config.h"

// ==================== 全局变量定义 ====================
DHT dht(PIN_DHT, DHT22);
DFRobot_ENS160_I2C ens160(&Wire, 0x53);
bool buzzerState = false;
bool fanState = false;
uint8_t ledBrightness = 128;

// ==================== 串口初始化 ====================
void initSerial() {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n[INIT] Serial OK - 115200bps");
}

// ==================== GPIO 初始化 ====================
void initGPIO() {
    // 输出引脚
    pinMode(PIN_BUZZER, OUTPUT);
    pinMode(PIN_FAN, OUTPUT);
    pinMode(PIN_LED_STATUS, OUTPUT);

    // 输入引脚
    pinMode(PIN_FLAME, INPUT);
    pinMode(PIN_CURRENT, INPUT);

    // 默认状态: 蜂鸣器关, 风扇关(P-MOS高电平截止), 指示灯灭
    digitalWrite(PIN_BUZZER, LOW);
    digitalWrite(PIN_FAN, HIGH);
    digitalWrite(PIN_LED_STATUS, LOW);

    Serial.println("[INIT] GPIO OK");
    Serial.printf("  PWM LED:  IO%d (MOS Q7)\n", PIN_PWM_LED);
    Serial.printf("  Buzzer:   IO%d (CN3)\n", PIN_BUZZER);
    Serial.printf("  Fan:      IO%d (CN4)\n", PIN_FAN);
    Serial.printf("  Status:   IO%d (LED1)\n", PIN_LED_STATUS);
    Serial.printf("  Flame:    IO%d (ADC)\n", PIN_FLAME);
    Serial.printf("  Current:  IO%d (ADC)\n", PIN_CURRENT);
    Serial.printf("  DHT22:    IO%d\n", PIN_DHT);
}

// ==================== PWM 初始化 ====================
// P-MOS调光: IO15 → R26(2K) → R27(56R) → Q7栅极
// 占空比越大 → MOS截止时间越长 → 灯越暗
// 占空比越小 → MOS导通时间越长 → 灯越亮
// 100K下拉电阻保证无信号时MOS完全关断
void initPWM() {
    ledcSetup(PWM_CHANNEL_LED, PWM_FREQ_LED, PWM_RESOLUTION_LED);
    ledcAttachPin(PIN_PWM_LED, PWM_CHANNEL_LED);
    // 默认关灯: 占空比255(MOS完全截止)
    ledcWrite(PWM_CHANNEL_LED, 255);
    Serial.println("[INIT] PWM OK - LED strip ready (P-MOS, inverted duty)");
}

// ==================== ADC 初始化 ====================
void initADC() {
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
    Serial.println("[INIT] ADC OK");
    Serial.printf("  Current ADC: IO%d (0-3.3V)\n", PIN_CURRENT);
}

// ==================== DHT 初始化 ====================
void initDHT() {
    dht.begin();
    Serial.println("[INIT] DHT22 OK");
}

// ==================== ENS160 初始化 ====================
void initENS160() {
    Wire.begin(PIN_SDA, PIN_SCL);
    int status = ens160.begin();
    if (status == 0) {
        Serial.println("[INIT] ENS160 OK - Air quality sensor ready");
    } else if (status == -1) {
        Serial.println("[INIT] ENS160 FAILED - Data bus error");
    } else if (status == -2) {
        Serial.println("[INIT] ENS160 FAILED - Chip version error");
    }
}

// ==================== LED 控制 ====================
// P-MOS: 低电平导通, 高电平截止
// 占空比越大 → MOS截止时间越长 → 灯越暗
void setLED(bool state) {
    ledcWrite(PWM_CHANNEL_LED, state ? (255 - ledBrightness) : 255);
    Serial.printf("[LED] %s (brightness=%d)\n", state ? "ON" : "OFF", ledBrightness);
}

void setLEDBrightness(uint8_t brightness) {
    ledBrightness = brightness;
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
// P-MOS驱动: LOW=导通(开), HIGH=截止(关)
void setFan(bool state) {
    digitalWrite(PIN_FAN, state ? LOW : HIGH);
    fanState = state;
    Serial.printf("[FAN] %s\n", state ? "ON" : "OFF");
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

    Serial.println("\n[TEST] ENS160 air quality...");
    int ensStatus = ens160.getENS160Status();
    Serial.printf("[TEST] ENS160 status: %d (0=normal, 1=warmup, 2=startup, 3=invalid)\n", ensStatus);
    Serial.printf("[TEST] AQI: %d, TVOC: %d ppb, ECO2: %d ppm\n",
                  ens160.getAQI(), ens160.getTVOC(), ens160.getECO2());

    Serial.println("\n========== TEST COMPLETE ==========\n");
}

// ==================== 完整初始化 ====================
void initAll() {
    Serial.println("\n==============================");
    Serial.println("  Ceiling Light 4.0 - Hardware Init");
    Serial.println("  Board: ESP32-WROOM-32UE-N16");
    Serial.println("==============================\n");

    initSerial();
    initGPIO();
    initPWM();
    initADC();
    initDHT();
    initENS160();

    Serial.println("\n[INIT] All hardware initialized!");
    Serial.println("[INIT] Running hardware test...\n");

    hardwareTest();
}
