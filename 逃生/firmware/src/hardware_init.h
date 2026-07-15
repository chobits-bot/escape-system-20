#ifndef HARDWARE_INIT_H
#define HARDWARE_INIT_H

#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include "ENS160.h"

// ==================== 引脚定义 (严格对照吸顶灯4.0电路原理图) ====================

// 串口0 - CH340C 下载串口 (Section 6.2)
#define PIN_TX0          1        // GPIO1 → CH340C RXD
#define PIN_RX0          3        // GPIO3 → CH340C TXD

// Cat1 4G 模块串口 (Section 6.2)
#define PIN_CAT_TX       16       // ESP32 TX → Cat1 RX
#define PIN_CAT_RX       17       // ESP32 RX → Cat1 TX

// I2C 总线 (Section 6.3)
#define PIN_SDA          21       // I2C SDA
#define PIN_SCL          22       // I2C SCL

// PWM LED 灯带调光 (Section 6.5)
// 原理图: IO15 → R26(2K) → R27(56R) → MOS管Q7(AOD4184L)栅极G
#define PIN_PWM_LED      15       // PWM灯带 → MOS Q7 栅极

// 蜂鸣器 (Section 6.6)
#define PIN_BUZZER       2        // IO2 → 有源蜂鸣器 CN3 控制脚

// 散热风扇 (Section 6.6)
#define PIN_FAN          5        // IO5 → 风扇端子 CN4 控制引脚

// 状态指示灯 (Section 6.6)
#define PIN_LED_STATUS   19       // IO19 → R1(1K)限流 → 3.3V

// 火焰传感器 (Section 6.4)
#define PIN_FLAME        4        // IO4 → 火焰传感器 (低电平=检测到火焰)

// 电流检测 ADC (Section 6.4)
#define PIN_CURRENT      34       // GPIO34 → SENSOR_VP (仅输入, 0-3.3V)

// DHT22 温湿度传感器
#define PIN_DHT          26       // IO26 → DHT22 数据脚

// ==================== PWM 配置 ====================
#define PWM_CHANNEL_LED    0
#define PWM_FREQ_LED       5000
#define PWM_RESOLUTION_LED 8

// ==================== 全局对象声明 ====================
extern DHT dht;
extern DFRobot_ENS160_I2C ens160;
extern bool buzzerState;
extern bool fanState;
extern uint8_t ledBrightness;

// ==================== 函数声明 ====================
void initAll();
void initSerial();
void initGPIO();
void initPWM();
void initADC();
void initDHT();
void initENS160();

void setLED(bool state);
void setLEDBrightness(uint8_t brightness);
void setBuzzer(bool state);
void setFan(bool state);

void hardwareTest();

#endif
