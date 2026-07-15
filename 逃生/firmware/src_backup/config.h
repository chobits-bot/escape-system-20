#ifndef CONFIG_H
#define CONFIG_H

// ==================== WiFi 配置 ====================
#define WIFI_SSID       "败家之眼"
#define WIFI_PASSWORD   "Tgs200410"

// ==================== MQTT 配置 ====================
#define MQTT_BROKER     "3.114.93.84"
#define MQTT_PORT       1883
#define MQTT_CLIENT_ID  "ESP32-CeilingLight-01"
#define MQTT_USERNAME   ""
#define MQTT_PASSWORD   ""
#define MQTT_QOS        0

// ==================== MQTT Topics ====================
#define TOPIC_SENSOR_DATA       "sensordata"
#define TOPIC_SIGNBOARD_HEARTBEAT "signboard/heartbeat"
#define TOPIC_SIGNBOARD_CONTROL "signboard/control"

// ==================== 引脚定义 ====================
#define PIN_TX0          1        // GPIO1 → CH340C RXD
#define PIN_RX0          3        // GPIO3 → CH340C TXD
#define PIN_CAT_TX       16       // ESP32 TX → Cat1 RX
#define PIN_CAT_RX       17       // ESP32 RX → Cat1 TX
#define PIN_SDA          21       // I2C SDA
#define PIN_SCL          22       // I2C SCL
#define PIN_PWM_LED      15       // PWM 灯带 → MOS Q7 栅极
#define PIN_BUZZER       2        // 蜂鸣器
#define PIN_FAN          5        // 散热风扇
#define PIN_LED_STATUS   19       // 状态指示灯
#define PIN_FLAME        4        // 火焰传感器
#define PIN_CURRENT      34       // 电流检测 ADC
#define PIN_DHT          26       // DHT 温湿度传感器

// ==================== PWM 配置 ====================
#define PWM_CHANNEL_LED    0
#define PWM_FREQ_LED       5000
#define PWM_RESOLUTION_LED 8

// ==================== 定时器配置 ====================
#define SENSOR_REPORT_INTERVAL  30000   // 传感器上报间隔 (ms)
#define HEARTBEAT_INTERVAL      60000   // 心跳上报间隔 (ms)
#define WIFI_CHECK_INTERVAL     10000   // WiFi检查间隔 (ms)

// ==================== 告警阈值 ====================
#define CURRENT_THRESHOLD   5.0     // 电流告警阈值 (A)
#define TEMP_THRESHOLD      50.0    // 温度告警阈值 (℃)

#endif
