# 吸顶灯 4.0 固件

基于 ESP32-WROOM-32UE-N16 的吸顶灯控制系统固件，使用 PlatformIO + Arduino 框架开发。

## 功能特性

- DHT22 温湿度传感器读取
- 火焰检测报警
- 电流异常监测
- PWM LED 灯带调光控制
- 蜂鸣器报警控制
- 散热风扇控制
- MQTT 数据上报到 EMQX Broker
- 远程控制指令接收
- 串口调试命令

## 硬件连接

| 功能 | ESP32 引脚 | 说明 |
|------|-----------|------|
| LED PWM | GPIO13 | MOS管栅极驱动 |
| 蜂鸣器 | GPIO12 | 有源蜂鸣器控制 |
| 风扇 | GPIO14 | 散热风扇控制 |
| 火焰传感器 | GPIO27 | 低电平触发 |
| 电流检测 | GPIO34 | ADC模拟输入 |
| DHT22 | GPIO26 | 温湿度数据 |

## 快速开始

### 1. 安装 PlatformIO

- VS Code 插件: PlatformIO IDE
- 或独立安装: https://platformio.org/install

### 2. 配置 WiFi 和 MQTT

编辑 `src/config.h`:

```cpp
#define WIFI_SSID       "your_wifi_ssid"
#define WIFI_PASSWORD   "your_wifi_password"
#define MQTT_BROKER     "127.0.0.1"
#define MQTT_PORT       1883
```

### 3. 编译烧录

```bash
# 编译
pio run

# 烧录
pio run -t upload

# 串口监控
pio device monitor
```

## MQTT 通信协议

### 上报数据

- Topic: `sensordata`
- 格式:

```json
{
  "AA:BB:CC:DD:EE:FF": {
    "temp": 26.5,
    "hum": 65.3,
    "AQI": 45.2,
    "TVOC": 120.0,
    "ECO2": 650.0,
    "electricity": 0.52,
    "electricity_Current": false,
    "ESP32_fires_flag": false,
    "ESP32_electricity_flag": false,
    "buzzer": 0
  }
}
```

### 控制指令

- Topic: `signboard/control`
- 格式:

```json
{
  "AA:BB:CC:DD:EE:FF": {
    "led": 1,
    "buzzer": 0,
    "electricity_switch": "ON"
  }
}
```

## 串口命令

连接串口后可发送以下命令:

| 命令 | 说明 |
|------|------|
| `led on` | 打开LED |
| `led off` | 关闭LED |
| `led 128` | 设置LED亮度50% |
| `buzzer on` | 打开蜂鸣器 |
| `buzzer off` | 关闭蜂鸣器 |
| `fan on` | 打开风扇 |
| `fan off` | 关闭风扇 |
| `status` | 显示传感器数据 |
| `help` | 显示帮助信息 |

## 依赖库

- PubSubClient (MQTT)
- ArduinoJson (JSON解析)
- DHT sensor library (温湿度传感器)

## 注意事项

1. 首次烧录需按住 BOOT 按键
2. CH340 供电必须为 3.3V，接5V会烧毁
3. 所有模块需共地连接
4. MOS管栅极下拉电阻不可省略
