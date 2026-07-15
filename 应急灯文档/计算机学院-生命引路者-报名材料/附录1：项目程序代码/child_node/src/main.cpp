
#include "EICEG.h"
#include <Arduino.h>
#include <WiFi.h>
#include <string.h>
#include <ArduinoJson.h>
#include <stdio.h>
#include <string.h> // 为了 memcpy 函数
#include "ENS160.h"
#include "DHT20.h"
#include <Ticker.h>
#include <analogWrite.h>
#include <ESP32_Mesh.h>
DHT20 DHT;
DFRobot_ENS160_I2C ENS160(&Wire, /*I2CAddr*/ 0x53);

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
    analogWrite(PWM,(led-1),1024);
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
    mesh.sendBroadcast(flame_mesh_start);//mesh发送
  }else if(fire_data == "False")
  {
    stop_alarms();
    mesh.sendBroadcast(flame_mesh_stop);//mesh发送
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
  /**
   * 获取传感器运行状态
   * 返回值：0-正常运行、 
   * 1-预热阶段，通电后的前 3 分钟。
   * 2-初始启动阶段，首次通电后运行的第一个小时。在传感器的生命周期内仅有一次。
   * 注：请注意，只有在连续运行 24 小时后，状态才会存储在非易失性存储器中。
   * 运行。如果在该期间结束前未通电，ENS160 将在重新通电后恢复 "初始启动 "模式。
   * 重新通电后，ENS160 将恢复 "初始启动 "模式。
   */
  Status = ENS160.getENS160Status();
  //Serial.print("传感器工作状态: ");
  //Serial.println(Status);
  if(Status == 0)
  {
    ENS160.setPWRMode(ENS160_STANDARD_MODE);
    /**
     * 获取空气质量指数
     * 返回值：1-优，2-良，3-中，4-差，5-不健康
     */
    int AQI_casual = ENS160.getAQI();
    if(AQI_casual != 0)
    {
      AQI = AQI_casual;
      // Serial.print("空气质量指数: ");
      // Serial.println(AQI);      
    }

    /**
     * 获取 TVOC 浓度
     * 返回值范围： 0-65000，单位：ppb
     */
    int TVOC_casual = ENS160.getTVOC();
    if(TVOC_casual != 0)
    {
      TVOC = TVOC_casual;
      // Serial.print("TVOC的浓度 : ");
      // Serial.print(TVOC);
      // Serial.println(" ppb");      
    }
    /**
     * 根据检测到的挥发性有机化合物和氢气数据计算出二氧化碳当量浓度（eCO2 - CO2 当量）。
     * 返回值范围： 400-65000，单位：ppm
     * 五个等级： 优（400 - 600）、良（600 - 800）、中（800 - 1000）、 
     * 差（1000 - 1500），不健康（大于 1500）
     */
    int ECO2_casual = ENS160.getECO2();
    if(ECO2_casual != 0)
    {
      ECO2 = ECO2_casual;
      // Serial.print("二氧化碳当量浓度 : ");
      // Serial.print(ECO2);
      // Serial.println(" ppm");      
    }
    //Serial.println();
  }else
  {
    /**
     * 获取空气质量指数
     * 返回值：1-优，2-良，3-中，4-差，5-不健康
     */
    AQI = ENS160.getAQI();
    /**
     * 获取 TVOC 浓度
     * 返回值范围： 0-65000，单位：ppb
     */
    TVOC = ENS160.getTVOC();
    /**
     * 根据检测到的挥发性有机化合物和氢气数据计算出二氧化碳当量浓度（eCO2 - CO2 当量）。
     * 返回值范围： 400-65000，单位：ppm
     * 五个等级： 优（400 - 600）、良（600 - 800）、中（800 - 1000）、 
     * 差（1000 - 1500），不健康（大于 1500）
     */
    ECO2 = ENS160.getECO2();

  }
  int status = DHT.read();
  hum = DHT.getHumidity();
  temp = DHT.getTemperature();  
}
int ticker_count = 0;
void tickerCount()//定时采集数据
{  
  sensor();
}
Ticker tasker=Ticker(tickerCount,250);//定时间隔，和回调函数采集传感器消息

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
    //建立发送内容
    messageString += "{\"ESP32_" + WiFi.macAddress() + "\":";
    messageString += "{\"AQI\": ";
    messageString += String(AQI);
    messageString += " ,\"TVOC\": ";
    messageString += String(TVOC);
    messageString += " ,\"ECO2\": ";
    messageString += String(ECO2);
    messageString += " ,\"temp\": ";
    messageString += String(temp);
    messageString += " ,\"hum\": ";
    messageString += String(hum);
    messageString += " ,\"electricity_Current\": ";
    messageString += String(electricity_Current);
    messageString += " ,\"electricity\": ";
    messageString += String(electricity_data);
    messageString += " ,\"ESP32_fires_flag\": ";
    messageString += String(ESP32_fires_flag);
    messageString += " ,\"ESP32_electricity_flag\": ";
    messageString += String(ESP32_electricity_flag);
    messageString += " ,\"buzzer\": ";
    messageString += String(buzzer_flag);
    messageString += "}}";
    mesh.sendBroadcast(messageString);//mesh发送
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
        Serial2.println("有变化");
        electricity_flag = flag;
        if(electricity_flag)
        {
          electricity_stop_alarms();
          Serial2.println("停止报警");
        }else
        {
          Serial2.println("开始报警");
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
    mesh.update();
    vTaskDelay(1);    
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println(__FILE__);
  pinMode(PWM, OUTPUT);
  analogWrite(PWM,0,1024);//最高建议不超过500
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer,LOW);
  Wire.begin();
  //DHT.begin();    // ESP32 默认引脚 21 22
  //启动传感器
  while( NO_ERR != ENS160.begin() ){
    Serial.println("请检查传感器！");
    delay(1000);
  }
  ENS160.setPWRMode(ENS160_STANDARD_MODE);
  ENS160.setTempAndHum(/*temperature=*/25.0, /*humidity=*/50.0);
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
  main_mesh_init();
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
}
