#pragma once

#include <Arduino.h>

// ========== USB/串口 ==========
#define PIN_USB_TX0        1   // GPIO1  TX0 → CH340C RXD
#define PIN_USB_RX0        3   // GPIO3  RX0 → CH340C TXD

// ========== LED灯带 PWM 调光 ==========
#define PIN_LED_PWM        15  // IO15 → R26(2K) → R27(56R) → Q7(AOD4184L) G极
#define LED_PWM_CHANNEL    0
#define LED_PWM_FREQ_HZ    1000
#define LED_PWM_RESOLUTION 8   // 8bit: 0~255

// ========== 蜂鸣器 ==========
#define PIN_BUZZER         2   // IO2 → CN3 有源蜂鸣器

// ========== 散热风扇 ==========
#define PIN_FAN            5   // IO5 → CN4 风扇控制

// ========== Cat1 4G 模块 ==========
#define PIN_CAT_TX         16  // GPIO16 → Cat1 RX
#define PIN_CAT_RX         17  // GPIO17 → Cat1 TX
#define PIN_CAT_STATE      4   // GPIO4  → Cat1 STATE (ADC)

// ========== I2C 总线 ==========
#define PIN_SDA            21  // GPIO21 → CN7/H2 SDA
#define PIN_SCL            22  // GPIO22 → CN7/H2 SCL

// ========== ADC 传感器 ==========
#define PIN_SENSOR_VP      34  // GPIO34 → H2 传感器拓展 (仅输入)
#define PIN_SENSOR_VN      35  // GPIO35 → H2 传感器拓展 (仅输入)

// ========== 状态指示灯 ==========
#define PIN_LED_STATUS     19  // IO19 → R1(1K) → LED1

// ========== 外置LED点阵/数码管驱动 ==========
#define PIN_LED_MATRIX_CLK 12  // IO12 → 时钟
#define PIN_LED_MATRIX_LAT 13  // IO13 → 锁存
#define PIN_LED_MATRIX_OE  14  // IO14 → 输出使能
#define PIN_LED_MATRIX_D   18  // IO18 → 数据

// ========== 预留备用GPIO ==========
// IO23, IO25, IO26, IO27, IO32, IO33 (空引脚)
