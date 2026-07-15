#ifndef HARDWARE_INIT_H
#define HARDWARE_INIT_H

#include <Arduino.h>
#include <DHT.h>

// ==================== 引脚定义 ====================
// 根据吸顶灯4.0电路原理图 (完整GPIO分配手册)

// 串口0 - CH340C 下载串口
#define PIN_TX0          1        // GPIO1 → CH340C RXD
#define PIN_RX0          3        // GPIO3 → CH340C TXD

// Cat1 4G 模块串口
#define PIN_CAT_TX       16       // ESP32 TX → Cat1 RX
#define PIN_CAT_RX       17       // ESP32 RX → Cat1 TX

// I2C 总线
#define PIN_SDA          21       // I2C SDA
#define PIN_SCL          22       // I2C SCL

// PWM LED 灯带调光 (Section 6.5)
#define PIN_PWM_LED      27       // 测试用 GPIO27

// 蜂鸣器 (Section 6.6)
#define PIN_BUZZER       2        // IO2 → 有源蜂鸣器 CN3 控制脚

// 散热风扇 (Section 6.6)
#define PIN_FAN          5        // IO5 → 风扇端子 CN4 控制引脚

// 状态指示灯 (Section 6.6)
#define PIN_LED_STATUS   19       // IO19 → R1(1K) → 3.3V

// 传感器引脚
#define PIN_FLAME        4        // IO4 → CN7 Cat1 STATE (火焰检测)
#define PIN_CURRENT      34       // GPIO34 → SENSOR_VP (电流检测 ADC)
#define PIN_DHT          26       // IO26 → DHT 温湿度传感器 (预留GPIO)

// 预留拓展 GPIO (Section 6.7/6.8)
// IO12=CLK, IO13=LAT, IO14=OE, IO18=D, IO23/25/26/27/32/33 预留

// ==================== PWM 配置 ====================
#define PWM_CHANNEL_LED    0
#define PWM_FREQ_LED       5000
#define PWM_RESOLUTION_LED 8

// ==================== 全局对象声明 ====================
extern DHT dht;
extern bool buzzerState;
extern bool fanState;

// ==================== 函数声明 ====================
void initAll();
void initSerial();
void initGPIO();
void initPWM();
void initADC();
void initDHT();

void setLED(bool state);
void setLEDBrightness(uint8_t brightness);
void setBuzzer(bool state);
void setFan(bool state);

void hardwareTest();

#endif
