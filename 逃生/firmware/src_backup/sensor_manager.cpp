#include "sensor_manager.h"
#include "config.h"

SensorManager::SensorManager() : dht(PIN_DHT, DHT22), buzzerState(false), fanState(false) {
    memset(&data, 0, sizeof(data));
}

void SensorManager::begin() {
    dht.begin();
    
    pinMode(PIN_BUZZER, OUTPUT);
    pinMode(PIN_FAN, OUTPUT);
    pinMode(PIN_LED_STATUS, OUTPUT);
    pinMode(PIN_FLAME, INPUT);
    pinMode(PIN_CURRENT, INPUT);
    
    digitalWrite(PIN_BUZZER, LOW);
    digitalWrite(PIN_FAN, HIGH);  // 默认关闭
    digitalWrite(PIN_LED_STATUS, LOW);
    
    ledcSetup(PWM_CHANNEL_LED, PWM_FREQ_LED, PWM_RESOLUTION_LED);
    ledcAttachPin(PIN_PWM_LED, PWM_CHANNEL_LED);
    ledcWrite(PWM_CHANNEL_LED, 255);  // P-MOS默认关闭
    
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
    
    Serial.println("[SENSOR] Initialized");
}

void SensorManager::update() {
    readDHT();
    readCurrent();
    readFlame();
}

void SensorManager::readDHT() {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    
    if (!isnan(h) && !isnan(t)) {
        data.temperature = t;
        data.humidity = h;
    }
}

void SensorManager::readCurrent() {
    int rawValue = analogRead(PIN_CURRENT);
    data.current = (rawValue / 4095.0) * 3.3 * 5;
    data.electricityCurrent = (data.current > CURRENT_THRESHOLD);
    data.electricityFlag = data.electricityCurrent;
}

void SensorManager::readFlame() {
    data.firesFlag = (digitalRead(PIN_FLAME) == LOW);
}

SensorData SensorManager::getData() {
    data.buzzerState = buzzerState ? 1 : 0;
    return data;
}

void SensorManager::setBuzzer(bool state) {
    digitalWrite(PIN_BUZZER, state ? HIGH : LOW);
    buzzerState = state;
    Serial.printf("[SENSOR] Buzzer: %s\n", state ? "ON" : "OFF");
}

void SensorManager::setFan(bool state) {
    digitalWrite(PIN_FAN, state ? LOW : HIGH);  // P-MOS: LOW=ON
    fanState = state;
    Serial.printf("[SENSOR] Fan: %s\n", state ? "ON" : "OFF");
}

void SensorManager::setLED(bool state) {
    ledcWrite(PWM_CHANNEL_LED, state ? 0 : 255);  // P-MOS: 0=ON
    Serial.printf("[SENSOR] LED: %s\n", state ? "ON" : "OFF");
}

void SensorManager::setLEDBrightness(uint8_t brightness) {
    uint8_t pwm = 255 - brightness;
    ledcWrite(PWM_CHANNEL_LED, pwm);
}
