
#include "EICEG.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <PubSubClient.h>
#include <string.h>
#include <ArduinoJson.h>
#include <stdio.h>
#include <string.h> // 为了 memcpy 函数
#include "ENS160.h"
#include <Wire.h>
#include <AHT20.h>
#include <Ticker.h>
#include <analogWrite.h>
#include <ESP32_Mesh.h>
AHT20 aht20;
DFRobot_ENS160_I2C ENS160(&Wire, /*I2CAddr*/ 0x53);
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

/**********************************************************************************************************************************************/
int split_line(const char *str, char ***str_lines, int *len) // 切分字符串
{
  char *s = "\n";
  char *b_str = (char *)malloc(strlen(str));
  memcpy(b_str, str, strlen(str));
  char *b_str_tmp = b_str;
  int cnt = 0;
  char *buf = strstr(b_str, s);
  while (buf != NULL)
  {
    cnt++;
    b_str = buf + strlen(s);
    buf = strstr(b_str, s);
  }
  *str_lines = (char **)malloc(sizeof(char *) * cnt);
  b_str = b_str_tmp;
  int i = 0;
  buf = strstr(b_str, s);
  while (buf != NULL)
  {
    buf[0] = '\0';
    (*str_lines)[i] = b_str;
    b_str = buf + strlen(s);
    buf = strstr(b_str, s);
    i++;
  }
  *len = cnt;
  return 0;
}



void setDirectionAndDynamic(String name, char *dynamic_static, char *direction_LED, char *DLED_Enable)
{
  *DLED_Enable = 1;
  if (name == "Left")
  {
    *dynamic_static = 1;
    *direction_LED = 1;
  }
  else if (name == "left")
  {
    *dynamic_static = 0;
    *direction_LED = 1;
  }
  else if (name == "Right")
  {
    *dynamic_static = 1;
    *direction_LED = 2;
  }
  else if (name == "right")
  {
    *dynamic_static = 0;
    *direction_LED = 2;
  }
  else if (name == "Down")
  {
    *dynamic_static = 1;
    *direction_LED = 3;
  }
  else if (name == "down")
  {
    *dynamic_static = 0;
    *direction_LED = 3;
  }
  else if (name == "Up")
  {
    *dynamic_static = 1;
    *direction_LED = 4;
  }
  else if (name == "up")
  {
    *dynamic_static = 0;
    *direction_LED = 4;
  }
  else if (name == "motifs")
  {
    *dynamic_static = 0;
    *direction_LED = 5;
  }
  else if (name == "style")
  {
    *dynamic_static = 0;
    *direction_LED = 6;
  }
  else
  {
    return;
  }
}
uint16_t myColor_left = dma_display->color565(0, 255, 0);
uint16_t myColor_mid = dma_display->color565(0, 255, 0);
uint16_t myColor_right = dma_display->color565(0, 255, 0);
void receiveCallback(byte *payload) // 收到信息后的回调函数
{
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, payload);
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }
  String my_identifier = "ESP32_";
  my_identifier += WiFi.macAddress();
  JsonObject ESP32_my = doc[my_identifier];
  // 电子屏图案
  String namedis_left = ESP32_my["display_left"].as<String>();   // "left"
  String namedis_mid = ESP32_my["display_mid"].as<String>();     // "left"
  String namedis_right = ESP32_my["display_right"].as<String>(); // "right"
  /*RGB*/
  JsonArray left_rgb = ESP32_my["RGB_left"];   // 0-255
  JsonArray mid_rgb = ESP32_my["RGB_mid"];     // 0-255
  JsonArray right_rgb = ESP32_my["RGB_right"]; // 0-255

  int brightness_1 = ESP32_my["brightness"].as<int>(); // 点阵屏亮度
  int led = ESP32_my["led"].as<int>();                 // 灯条亮度
  int buzzer_1 = ESP32_my["buzzer"].as<int>();         // 蜂鸣器
  String fire_data = ESP32_my["flame"].as<String>();   // 报警
  String electricity_data = ESP32_my["electricity_switch"].as<String>();//电流检测开关
  /*
   *direction_LED_left = 1 -----左箭头
   *direction_LED_left = 2 -----右箭头
   *direction_LED_left = 3 -----下箭头
   *direction_LED_left = 4 -----上箭头
   */
  unsigned char left_temporarily = dynamic_static_left;
  unsigned char right_temporarily = dynamic_static_right;
  unsigned char mid_temporarily = dynamic_static_mid;
  setDirectionAndDynamic(namedis_left, &dynamic_static_left, &direction_LED_left, &DLED_Enable_left);
  if((dynamic_static_left!=left_temporarily)&dynamic_static_left==0)
  {
    silky_left = 1;
  }
  setDirectionAndDynamic(namedis_mid, &dynamic_static_mid, &direction_LED_mid, &DLED_Enable_mid);
  if((dynamic_static_mid!=mid_temporarily)&dynamic_static_right==0)
  {
    silky_mid = 1;
  }
  setDirectionAndDynamic(namedis_right, &dynamic_static_right, &direction_LED_right, &DLED_Enable_right);
  if((dynamic_static_right!=right_temporarily)&dynamic_static_mid==0)
  {
    silky_right = 1;
  }
  /******************场效应管***********************/
  if(1<=led&led<=1024)
  {
    analogWrite(PWM,(led-1));
  }
  /************************************************/
  /*******************蜂鸣器***********************/
  if(buzzer_1 == 1 | buzzer_1 == 2)//1是关，2是开
  {
    if(buzzer_1 == 1)
    {
      buzzer_stop_alarms();
    }else if(buzzer_1 == 2)
    {
      buzzer_start_alarms();
    }
  }
  /************************************************/
  /********************报警************************/
  if(fire_data == "True")
  {
    start_alarms();
    // mesh.sendBroadcast(flame_mesh_start);//CAT1模式下不用mesh
  }else if(fire_data == "False")
  {
    stop_alarms();
    // mesh.sendBroadcast(flame_mesh_stop);//CAT1模式下不用mesh
  }
  /*************************************************/
  /****************电流检测开关*********************/
  if(electricity_data == "ON")
  {
    electricity_Current = true;
  }else if(electricity_data == "OFF")
  {
    electricity_Current = false;
  }
  /*************************************************/
  /*************************************************/
  if((left_rgb[0]<=255 & left_rgb[1] <= 255 & left_rgb[2] <= 255) && (left_rgb[0]>=0 & left_rgb[1] >= 0 & left_rgb[2] >= 0))
  {
    myColor_left = dma_display->color565(left_rgb[0], left_rgb[1], left_rgb[2]);
  }
  if((mid_rgb[0]<=255 & mid_rgb[1] <= 255 & mid_rgb[2] <= 255) && (mid_rgb[0]>=0 & mid_rgb[1] >= 0 & mid_rgb[2] >= 0))
  {
    myColor_mid = dma_display->color565(mid_rgb[0], mid_rgb[1], mid_rgb[2]);
  }
  if((right_rgb[0]<=255 & right_rgb[1] <= 255 & right_rgb[2] <= 255) && (right_rgb[0]>=0 & right_rgb[1] >= 0 & right_rgb[2] >= 0))
  {
    myColor_right = dma_display->color565(right_rgb[0], right_rgb[1], right_rgb[2]);
  }
  /*************************************************/
}

void show_left()
{
  if (DLED_Enable_left == 1)
  {
    if (dynamic_static_left == 1|silky_left == 1) // 动态
    {

      switch (direction_LED_left)
      {
      case 1:
        dma_display->drawBitmap(0, 0, dynamic_left, 32, 32, myColor_left, myBLACK);
        break;
      case 2:
        dma_display->drawBitmap(0, 0, dynamic_right, 32, 32, myColor_left, myBLACK);
        break;
      case 3:
        dma_display->drawBitmap(0, 0, dynamic_ldown, 32, 32, myColor_left, myBLACK);
        break;
      case 4:
        dma_display->drawBitmap(0, 0, dynamic_lup, 32, 32, myColor_left, myBLACK);
        break;
      default:
        break;
      }
    }
    else // 静态
    {
      if (scanning == 1)
      {
        switch (direction_LED_left)
        {
        case 1:
          dma_display->drawBitmap(0, 0, static_left, 32, 32, myColor_left, myBLACK);
          break;
        case 2:
          dma_display->drawBitmap(0, 0, static_right, 32, 32, myColor_left, myBLACK);
          break;
        case 3:
          dma_display->drawBitmap(0, 0, static_ldown, 32, 32, myColor_left, myBLACK);
          break;
        case 4:
          dma_display->drawBitmap(0, 0, static_lup, 32, 32, myColor_left, myBLACK);
          break;
        case 5:
          dma_display->drawBitmap(0, 0, static_icon, 32, 32, myColor_left, myBLACK);
          break;
        case 6:
          dma_display->drawBitmap(0, 0, writing_style, 32, 32, myColor_left, myBLACK);
          break;
        default:
          break;
        }
      }
    }
  }
}
void show_mid()
{
  if (DLED_Enable_mid == 1)
  {
    if (dynamic_static_mid == 1|silky_mid == 1) // 动态
    {
      switch (direction_LED_mid)
      {
      case 1:
        dma_display->drawBitmap(32, 0, dynamic_left, 32, 32, myColor_mid, myBLACK);
        break;
      case 2:
        dma_display->drawBitmap(32, 0, dynamic_right, 32, 32, myColor_mid, myBLACK);
        break;
      case 3:
        dma_display->drawBitmap(32, 0, dynamic_ldown, 32, 32, myColor_mid, myBLACK);
        break;
      case 4:
        dma_display->drawBitmap(32, 0, dynamic_lup, 32, 32, myColor_mid, myBLACK);
        break;
      default:
        break;
      }
    }
    else // 静态
    {
      if (scanning == 1)
      {
        switch (direction_LED_mid)
        {
        case 1:
          dma_display->drawBitmap(32, 0, static_left, 32, 32, myColor_mid, myBLACK);
          break;
        case 2:
          dma_display->drawBitmap(32, 0, static_right, 32, 32, myColor_mid, myBLACK);
          break;
        case 3:
          dma_display->drawBitmap(32, 0, static_ldown, 32, 32, myColor_mid, myBLACK);
          break;
        case 4:
          dma_display->drawBitmap(32, 0, static_lup, 32, 32, myColor_mid, myBLACK);
          break;
        case 5:
          dma_display->drawBitmap(32, 0, static_icon, 32, 32, myColor_mid, myBLACK);
          break;
        case 6:
          dma_display->drawBitmap(32, 0, writing_style, 32, 32, myColor_mid, myBLACK);
          break;
        default:
          break;
        }
      }
    }
  }
}

void show_right()
{
  if (DLED_Enable_right == 1)
  {
    if((dynamic_static_right == 1)|silky_right == 1)// 动态
    {
      switch (direction_LED_right)
      {
      case 1:
        dma_display->drawBitmap(64, 0, dynamic_left, 32, 32, myColor_right, myBLACK);
        break;
      case 2:
        dma_display->drawBitmap(64, 0, dynamic_right, 32, 32, myColor_right, myBLACK);
        break;
      case 3:
        dma_display->drawBitmap(64, 0, dynamic_ldown, 32, 32, myColor_right, myBLACK);
        break;
      case 4:
        dma_display->drawBitmap(64, 0, dynamic_lup, 32, 32, myColor_right, myBLACK);
        break;
      default:
        break;
      }
    }
    else // 静态
    {
      if (scanning == 1)
      {
        switch (direction_LED_right)
        {
        case 1:
          dma_display->drawBitmap(64, 0, static_left, 32, 32, myColor_right, myBLACK);
          break;
        case 2:
          dma_display->drawBitmap(64, 0, static_right, 32, 32, myColor_right, myBLACK);
          break;
        case 3:
          dma_display->drawBitmap(64, 0, static_ldown, 32, 32, myColor_right, myBLACK);
          break;
        case 4:
          dma_display->drawBitmap(64, 0, static_lup, 32, 32, myColor_right, myBLACK);
          break;
        case 5:
          dma_display->drawBitmap(64, 0, static_icon, 32, 32, myColor_right, myBLACK);
          break;
        case 6:
          dma_display->drawBitmap(64, 0, writing_style, 32, 32, myColor_right, myBLACK);
          break;
        default:
          break;
        }
      }
    }
  }
}
void sensor()
{
  // 传感器不在ESP32上，使用默认值
  // 实际数据应从CAT1模块或服务器获取
  AQI = 1;
  TVOC = 0;
  ECO2 = 400;
  temp = 25.0;
  hum = 50.0;
}
int ticker_count = 0;
void tickerCount()//定时采集数据
{  
  sensor();
}
Ticker tasker=Ticker(tickerCount,250);//定时间隔，和回调函数采集传感器消息

/* ═══ WiFi MQTT 功能 ═══ */
// MQTT回调函数
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println("MQTT收到: " + message);
  
  // 解析逃生方向
  if (message.indexOf("left") > 0) {
    escapeDirection = "left";
    direction_LED_left = 1; direction_LED_mid = 1; direction_LED_right = 1;
  } else if (message.indexOf("right") > 0) {
    escapeDirection = "right";
    direction_LED_left = 2; direction_LED_mid = 2; direction_LED_right = 2;
  } else if (message.indexOf("up") > 0) {
    escapeDirection = "up";
    direction_LED_left = 4; direction_LED_mid = 4; direction_LED_right = 4;
  } else if (message.indexOf("down") > 0) {
    escapeDirection = "down";
    direction_LED_left = 3; direction_LED_mid = 3; direction_LED_right = 3;
  }
  dynamic_static_left = 1; dynamic_static_mid = 1; dynamic_static_right = 1;
  DLED_Enable_left = 1; DLED_Enable_mid = 1; DLED_Enable_right = 1;
  silky_left = 1; silky_mid = 1; silky_right = 1;
}

// MQTT重连
void mqttReconnect() {
  if (mqtt.connected()) return;
  
  String clientId = "ESP32_" + WiFi.macAddress();
  Serial.print("连接MQTT服务器: ");
  Serial.println(MQTT_SERVER);
  
  // 使用用户名密码连接
  if (mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
    Serial.println("MQTT连接成功!");
    mqtt.subscribe(theme_re.c_str());
    Serial.println("已订阅主题: " + theme_re);
  } else {
    Serial.println("MQTT连接失败, 错误码: " + String(mqtt.state()));
  }
}

// MQTT发布消息
void mqttPublish(String topic, String payload) {
  if (!mqtt.connected()) {
    mqttReconnect();
  }
  if (mqtt.connected()) {
    mqtt.publish(topic.c_str(), payload.c_str());
    Serial.println("MQTT发布: " + topic);
  }
}

void send_buzzer()//定时发送数据且蜂鸣器响应
{
  /************蜂鸣器报警************/
  if(buzzer_flag==2)
  {
    buzzer_count++;
    if(buzzer_count == 1 | buzzer_count == 2)
    {
      digitalWrite(buzzer,HIGH);
    }else
    {
      digitalWrite(buzzer,LOW);
      buzzer_count = 0;
    }
  }
  /*************发送数据**************/
  if(MQTT_send_count == Mesh_Count)
  {
    String messageString = "";
    MQTT_send_count = 0;
    // 建立发送内容（匹配后端期望的格式）
    messageString += "{\"" + WiFi.macAddress() + "\":";
    messageString += "{\"temp\":";
    messageString += String(temp, 1);
    messageString += ",\"hum\":";
    messageString += String(hum, 1);
    messageString += ",\"AQI\":";
    messageString += String(AQI);
    messageString += ",\"TVOC\":";
    messageString += String(TVOC);
    messageString += ",\"ECO2\":";
    messageString += String(ECO2);
    messageString += ",\"electricity\":";
    messageString += String(electricity_data);
    messageString += ",\"ESP32_fires_flag\":";
    messageString += ESP32_fires_flag ? "true" : "false";
    messageString += ",\"ESP32_electricity_flag\":";
    messageString += ESP32_electricity_flag ? "true" : "false";
    messageString += "}}";
    
    // 通过WiFi MQTT发送数据
    if (mqtt.connected()) {
      mqttPublish(topicString, messageString);
      Serial.println("数据已上报: " + messageString);
    } else {
      Serial.println("MQTT未连接，尝试重连...");
      mqttReconnect();
    }
  }
  MQTT_send_count++;
}
Ticker tasker1=Ticker(send_buzzer,1000);//定时间隔，和回调函数采集传感器消息
boolean check()//监测是否断电
{
    int sum = 0;
    for(int count = 0; count < 8; count++)
    {
      sum += analogRead(electricity_pin);
    }
    electricity_data = (sum/8);
    return (sum/8) > 400 ? true : false ;//true-有电
}

/* ═══ WiFi定位逃生 Web 服务器 ═══ */
// 首页 - 显示逃生方向
void handleRoot() {
  connectedDevices++;
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
  html += "<title>Escape Guide</title>";
  html += "<style>body{font-family:Arial;text-align:center;padding:20px;background:#1a1a2e;color:white;}";
  html += ".arrow{font-size:120px;margin:20px 0;animation:pulse 1s infinite;}";
  html += "@keyframes pulse{0%,100%{transform:scale(1)}50%{transform:scale(1.1)}}";
  html += ".status{font-size:24px;padding:10px;border-radius:10px;margin:10px 0;}";
  html += ".normal{background:#22c55e;}.fire{background:#ef4444;}</style></head><body>";
  html += "<h1>Emergency Escape</h1>";
  
  if(escapeStatus == "fire") {
    html += "<div class='status fire'>WARNING: FIRE DETECTED</div>";
  } else {
    html += "<div class='status normal'>System Normal</div>";
  }
  
  // 显示逃生方向箭头
  if(escapeDirection == "left") {
    html += "<div class='arrow'>&#x2190;</div>";
    html += "<h2>Exit is to the LEFT</h2>";
  } else if(escapeDirection == "right") {
    html += "<div class='arrow'>&#x2192;</div>";
    html += "<h2>Exit is to the RIGHT</h2>";
  } else if(escapeDirection == "up") {
    html += "<div class='arrow'>&#x2191;</div>";
    html += "<h2>Exit is STRAIGHT AHEAD</h2>";
  } else if(escapeDirection == "down") {
    html += "<div class='arrow'>&#x2193;</div>";
    html += "<h2>Exit is BEHIND YOU</h2>";
  }
  
  html += "<p>Connected devices: " + String(connectedDevices) + "</p>";
  html += "<p>Device: " + WiFi.macAddress() + "</p>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
  Serial.println("设备连接: " + WiFi.macAddress());
}

// 获取逃生方向API
void handleEscape() {
  String json = "{\"direction\":\"" + escapeDirection + "\",";
  json += "\"status\":\"" + escapeStatus + "\",";
  json += "\"devices\":" + String(connectedDevices) + "}";
  server.send(200, "application/json", json);
}

// 设置逃生方向API（服务器调用）
void handleDirection() {
  if(server.hasArg("dir")) {
    escapeDirection = server.arg("dir");
    // 更新LED显示
    if(escapeDirection == "left") {
      direction_LED_left = 1;
      direction_LED_mid = 1;
      direction_LED_right = 1;
    } else if(escapeDirection == "right") {
      direction_LED_left = 2;
      direction_LED_mid = 2;
      direction_LED_right = 2;
    } else if(escapeDirection == "up") {
      direction_LED_left = 4;
      direction_LED_mid = 4;
      direction_LED_right = 4;
    } else if(escapeDirection == "down") {
      direction_LED_left = 3;
      direction_LED_mid = 3;
      direction_LED_right = 3;
    }
    // 设置为动态箭头
    dynamic_static_left = 1;
    dynamic_static_mid = 1;
    dynamic_static_right = 1;
    DLED_Enable_left = 1;
    DLED_Enable_mid = 1;
    DLED_Enable_right = 1;
    silky_left = 1;
    silky_mid = 1;
    silky_right = 1;
    
    server.send(200, "application/json", "{\"ok\":true}");
    Serial.println("逃生方向已更新: " + escapeDirection);
  } else {
    server.send(400, "application/json", "{\"error\":\"missing dir parameter\"}");
  }
}

void xTaskOne(void *xTask1)
{
  while (1)
  {
    if ((dynamic_static_left | dynamic_static_mid | dynamic_static_right) | scanning != 1)
    {
      if (scanning == 32)
      {
        scanning = 0;
        silky_left = 0;
        silky_right = 0;
        silky_mid = 0;
      }
      shift_pixels_left(dynamic_left);
      shift_pixels_right(dynamic_right);
      shift_pixels_down(dynamic_ldown);
      shift_pixels_down(dynamic_rdown);
      shift_pixels_up(dynamic_lup);
      shift_pixels_up(dynamic_rup);
      scanning++; // 每移动一个像素点加一次
    }
    show_left();
    show_mid();
    show_right();
    //check();
    boolean flag = check();
    if(electricity_Current == true)
    {
      if(electricity_flag != flag)
      {
        Serial.println("电流状态变化");
        electricity_flag = flag;
        if(electricity_flag)
        {
          electricity_stop_alarms();
          Serial.println("停止报警");
        }else
        {
          Serial.println("开始报警");
          electricity_start_alarms();
        }
      }
    }else
    {
      electricity_flag = true;
    }

    //sensor();
    vTaskDelay(100);
  }
}

void xTaskTwo(void *xTask2)
{
  tasker.start();
  tasker1.start();
  while (1)
  {
    tasker.update();
    tasker1.update();
    // CAT1模式下不需要mesh.update()
    vTaskDelay(1);
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println(__FILE__);
  pinMode(PWM, OUTPUT);
  analogWrite(PWM,0);//最高建议不超过500
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer,LOW);

  // ═══ WiFi STA+AP 模式 ═══
  // 同时连接路由器(上网) + 作为热点(手机连接)
  WiFi.mode(WIFI_AP_STA);

  // 1. 连接WiFi路由器（STA模式）
  Serial.print("正在连接WiFi: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 30) {
    delay(500);
    Serial.print(".");
    retry++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi连接成功!");
    Serial.print("IP地址: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi连接失败，继续运行...");
  }

  // 2. 启动AP热点（用于手机WiFi定位逃生）
  WiFi.softAP("ESCAPE_" + WiFi.macAddress(), "escape123");
  delay(500);
  IPAddress localIP(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.softAPConfig(localIP, gateway, subnet);
  Serial.print("设备MAC地址: ");
  Serial.println(WiFi.macAddress());
  Serial.print("WiFi热点已启动: ESCAPE_");
  Serial.println(WiFi.macAddress());
  Serial.print("热点IP: ");
  Serial.println(WiFi.softAPIP());
  // 传感器不在ESP32上，跳过初始化
  Serial.println("传感器不在ESP32 I2C总线上，使用默认值");
  AQI = 1;
  TVOC = 0;
  ECO2 = 400;
  temp = 25.0;
  hum = 50.0;
  electricity_flag = true;
  /**********************************************************************/
  direction_LED_left = 1;  // 显示的方式
  dynamic_static_left = 0; // 1--动态  0--静态
  DLED_Enable_left = 1;    // 1--使能--开启默认的箭头指示  0--不使能
  /**********************************************************************/
  direction_LED_mid = 5;  // 显示的方式
  dynamic_static_mid = 0; // 1--动态  0--静态
  DLED_Enable_mid = 1;    // 1--使能--开启默认的箭头指示  0--不使能
  /**********************************************************************/
  direction_LED_right = 2;  // 显示的方式
  dynamic_static_right = 1; // 1--动态  0--静态
  DLED_Enable_right = 1;    // 1--使能--开启默认的箭头指示  0--不使能
  /**********************************************************************/
  init_EICEG();
  // ═══ WiFi MQTT 初始化 ═══
  if (WiFi.status() == WL_CONNECTED) {
    // 同步NTP时间（中国时区 UTC+8）
    configTime(8 * 3600, 0, "ntp.aliyun.com", "ntp.tencent.com");
    Serial.println("NTP时间同步中...");
    delay(2000);
    Serial.println("WiFi已连接，准备连接MQTT服务器...");
    // 主题设置
    topicString = MQTT_PUBLISH_TOPIC;  // 发布主题
    theme_re = MQTT_SUBSCRIBE_TOPIC;   // 订阅主题
    Serial.print("MQTT主题(发布): ");
    Serial.println(topicString);
    Serial.print("MQTT主题(订阅): ");
    Serial.println(theme_re);
    
    // 初始化PubSubClient
    mqtt.setServer(MQTT_SERVER, MQTT_PORT);
    mqtt.setCallback(mqttCallback);
    mqttReconnect();
  } else {
    Serial.println("WiFi未连接，MQTT功能不可用");
  }

  // 启动Web服务器（用于WiFi定位逃生）
  server.on("/", handleRoot);
  server.on("/escape", handleEscape);
  server.on("/direction", handleDirection);
  server.begin();
  Serial.println("Web服务器已启动: http://192.168.4.1");
  xTaskCreatePinnedToCore(
      xTaskOne,   // 任务对应的函数
      "xTaskOne", // 任务名
      4096,       // 栈大小。(This stack size can be checked & adjusted by reading the Stack Highwater)
      NULL,       // 传给任务函数的参数
      2,          // 任务优先级。(Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.)
      NULL,       // 用来返回任务handle
      0           // 指定CPU核心,tskNO_AFFINITY表示不指定，数字代表核心ID
  );
  // 指定在核心1上执行Task2_print任务
  xTaskCreatePinnedToCore(
      xTaskTwo,   // 任务对应的函数
      "xTaskTwo", // 任务名
      10000,      // 栈大小。(This stack size can be checked & adjusted by reading the Stack Highwater)
      NULL,       // 传给任务函数的参数
      1,          // 任务优先级。(Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.)
      NULL,       // 用来返回任务handle
      1           // 指定CPU核心,tskNO_AFFINITY表示不指定，数字代表核心ID
  );
}

void loop()
{
  server.handleClient();  // 处理Web请求
  mqtt.loop();            // 维持MQTT连接，接收消息
}
