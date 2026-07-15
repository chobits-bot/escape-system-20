#ifndef CONFIG_H
#define CONFIG_H

// ==================== WiFi 配置 ====================
#define WIFI_SSID       "败家之眼"
#define WIFI_PASSWORD   "Tgs200410"

// ==================== MQTT 配置 ====================
// 本地开发: "172.20.133.66"  云服务器: 改为你的公网IP
#define MQTT_BROKER     "42.193.218.29"
#define MQTT_PORT       1883
#define MQTT_CLIENT_ID  "ESP32-CeilingLight-01"
#define MQTT_USERNAME   ""
#define MQTT_PASSWORD   ""
#define MQTT_QOS        0

// ==================== MQTT Topics ====================
#define TOPIC_SENSOR_DATA        "sensordata"
#define TOPIC_SIGNBOARD_HEARTBEAT "signboard/heartbeat"
#define TOPIC_SIGNBOARD_CONTROL  "signboard/control"

// ==================== 引脚定义 (严格对照电路原理图) ====================
// Section 6.2 - 硬件UART串口
#define PIN_TX0          1        // GPIO1 → CH340C RXD (调试串口TX)
#define PIN_RX0          3        // GPIO3 → CH340C TXD (调试串口RX)
#define PIN_CAT_TX       16       // ESP32 TX → Cat1 RX (4G模块)
#define PIN_CAT_RX       17       // ESP32 RX → Cat1 TX (4G模块)

// Section 6.3 - I2C通信
#define PIN_SDA          21       // I2C SDA (Cat1模块 + 拓展座H2)
#define PIN_SCL          22       // I2C SCL (Cat1模块 + 拓展座H2)

// Section 6.5 - PWM LED灯带调光 (关键修正: 原config.h错误写了GPIO23)
// 原理图: IO15 → R26(2K) → R27(56R) → MOS管Q7(AOD4184L)栅极G
// 100K下拉电阻R28保证无信号时MOS完全关断
#define PIN_PWM_LED      15       // IO15 → MOS Q7 栅极 (P-MOS: 占空比大=灯暗)

// Section 6.6 - 声光、风扇外设
#define PIN_BUZZER       2        // IO2 → 有源蜂鸣器 CN3 控制脚 (高电平响)
#define PIN_FAN          5        // IO5 → 风扇端子 CN4 控制引脚 (P-MOS: LOW=开)
#define PIN_LED_STATUS   19       // IO19 → R1(1K)限流 → 3.3V (LOW=亮)

// Section 6.4 - ADC模拟采集
#define PIN_FLAME        4        // IO4 → 火焰传感器 (低电平=检测到火焰)
#define PIN_CURRENT      34       // GPIO34 → SENSOR_VP → H2拓展座 (电流检测ADC, 仅输入)
#define PIN_CURRENT_2    35       // GPIO35 → SENSOR_VN → H2拓展座 (备用ADC, 仅输入)

// DHT22温湿度传感器 (使用预留GPIO)
#define PIN_DHT          26       // IO26 → DHT22 数据脚

// Section 6.7 - 预留拓展 (外置LED点阵/数码管)
#define PIN_EXT_CLK      12       // IO12 → CLK
#define PIN_EXT_LAT      13       // IO13 → LAT锁存
#define PIN_EXT_OE       14       // IO14 → OE输出使能
#define PIN_EXT_DATA     18       // IO18 → D数据

// Section 6.8 - 预留备用 (IO23/25/26/27/32/33 无默认接线)
// IO26已分配给DHT22, IO27/32/33可二次开发

// ==================== PWM 配置 ====================
#define PWM_CHANNEL_LED    0
#define PWM_FREQ_LED       5000   // 5kHz调光频率
#define PWM_RESOLUTION_LED 8      // 8位分辨率(0-255)

// ==================== 定时器配置 (ms) ====================
#define SENSOR_REPORT_INTERVAL  30000   // 传感器上报间隔 30s
#define HEARTBEAT_INTERVAL      60000   // 心跳上报间隔 60s
#define WIFI_CHECK_INTERVAL     10000   // WiFi检查间隔 10s
#define SERIAL_CHECK_INTERVAL   100     // 串口命令检查间隔

// ==================== 告警阈值 ====================
#define CURRENT_THRESHOLD   5.0     // 电流告警阈值 (A)
#define TEMP_THRESHOLD      50.0    // 温度告警阈值 (℃)
#define TVOC_THRESHOLD      500     // TVOC告警阈值 (ppb) - 烟雾/有害气体
#define ECO2_THRESHOLD      1000    // CO2告警阈值 (ppm) - 烟雾浓度

// ==================== PWM 默认值 ====================
#define LED_DEFAULT_BRIGHTNESS  128     // 默认亮度 50%
#define LED_DEFAULT_STATE       false   // 默认关灯

#endif
