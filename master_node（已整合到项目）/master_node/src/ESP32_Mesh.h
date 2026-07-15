/*主节点的Mesh*/
#include "Cat-WH-LTE-7S0.h"
#include <painlessMesh.h>
#include <ArduinoJson.h>
/*********************Mesh&WIFI**********************/
#define MESH_PREFIX "linyuting" 
#define MESH_PASSWORD "lianjiemima" 
#define MESH_PORT 5555
painlessMesh mesh;
Scheduler userScheduler;
uint32_t mesh_noID[10];//存储meshID
String mesh_dat[10];//储存数据
int mesh_number,mesh_onID_re;//mesh_number---Mesh连接的数量
/*********************************************************/
#define PWM 33//场效应管
#define buzzer 16//16蜂鸣器
#define electricity_pin 36//监测电流
boolean electricity_flag = NULL;//电流标志位
#define MQTT_Count 5    //往MQTT服务器发送消息间隔--5秒发送一次
int MQTT_send_count = 0;//往MQTT服务器发送消息间隔变量
int scanning = 0; // 扫描
int buzzer_count = 0;//蜂鸣器声音延时计数

char silky_left = 0;
char silky_mid = 0;
char silky_right = 0;
uint8_t Status,AQI;
uint16_t TVOC,ECO2;
float temp = 20,hum = 50;
int electricity_data = 0;//电流值
int buzzer_flag = 1;//蜂鸣器标志位-1---关闭2---打开
boolean electricity_Current = true;//电流使能位
boolean ESP32_electricity_flag = 0;//当前状态 1-报警 0-无报警 -电
boolean ESP32_fires_flag = 0;//当前状态 1-报警 0-无报警 -火
/*********************************************************/
void buzzer_start_alarms()
{
  buzzer_flag = 2;
  buzzer_count = 0;
}
void buzzer_stop_alarms()
{
  buzzer_flag = 1;
  buzzer_count = 0;
  digitalWrite(buzzer,LOW);
}

void electricity_start_alarms()
{
  ESP32_electricity_flag = 1;
  buzzer_start_alarms();
  dynamic_static_left = 1;
  dynamic_static_mid = 1;
  dynamic_static_right = 1;
  analogWrite(PWM,250,1024);
}

void electricity_stop_alarms()
{
  ESP32_electricity_flag = 0;
  buzzer_stop_alarms();
  if(dynamic_static_left==1)
  {
    silky_left = 1;
  }
  if(dynamic_static_mid == 1)
  {
    silky_mid = 1;
  }
    if(dynamic_static_right == 1)
  {
    silky_right = 1;
  }
  dynamic_static_left = 0;
  dynamic_static_mid = 0;
  dynamic_static_right = 0;
  analogWrite(PWM,0,1024);
}

void start_alarms()
{
  ESP32_fires_flag = 1;
  buzzer_start_alarms();
  dynamic_static_left = 1;
  dynamic_static_mid = 1;
  dynamic_static_right = 1;
  analogWrite(PWM,500,1024);
}

void stop_alarms()
{
  ESP32_fires_flag = 0;
  buzzer_stop_alarms();
  if(dynamic_static_left==1)
  {
    silky_left = 1;
  }
  if(dynamic_static_mid == 1)
  {
    silky_mid = 1;
  }
  if(dynamic_static_right == 1)
  {
    silky_right = 1;
  }
  dynamic_static_left = 0;
  dynamic_static_mid = 0;
  dynamic_static_right = 0;
  analogWrite(PWM,0,1024);
}
void clean_mesh()//清除储存在Mesh的数据
{
  for(int i=0;i<mesh_number;i++)
  {
    mesh_noID[i]=NULL;
  }
  mesh_number=0;
}
char * flame_mesh_stop  = "{\"ESP32_fires\": 0 }";
char * flame_mesh_start = "{\"ESP32_fires\": 1 }";
void receivedCallback_main_mesh( uint32_t from, String &msg )//mesh接收
{     
  if(strstr(msg.c_str(),flame_mesh_start) != NULL)//检测到标志位为1时报警-其他设备报警
  {
    //报警
    buzzer_start_alarms();
    dynamic_static_left = 1;
    dynamic_static_mid = 1;
    dynamic_static_right = 1;
    analogWrite(33,500,1024);
  }else if(strstr(msg.c_str(),flame_mesh_stop) != NULL)//检测到标志位符合时报警，mesh接收到的mqtt整体控制
  {
    buzzer_stop_alarms();
    if(dynamic_static_left==1)
    {
      silky_left = 1;
    }
    if(dynamic_static_mid == 1)
    {
      silky_mid = 1;
    }
    if(dynamic_static_right == 1)
    {
      silky_right = 1;
    }
    dynamic_static_left = 0;
    dynamic_static_mid = 0;
    dynamic_static_right = 0;
    analogWrite(PWM,0,1024);
  }else
  {
    if(mesh_number==0)
    {
      mesh_noID[0]=from;
      mesh_dat[0]=msg.c_str();
      mesh_number++;
    }else
    {
      int mesh_number_t=mesh_number;
      while(mesh_number_t--)
        {
        if(mesh_noID[mesh_number_t]==from)
        {
          mesh_onID_re=1;
          mesh_dat[mesh_number_t]=msg.c_str();
          break;
        }
      }
      if(mesh_onID_re==0)
      {
        mesh_noID[mesh_number]=from;
        mesh_dat[mesh_number]=msg.c_str();
        mesh_number++;
      }else
      {
        mesh_onID_re=0;
      }
    }    
  }
  Serial.println(msg.c_str());
  Serial.print("当前连接的设备有：");
  Serial.print(mesh_number);
  Serial.println("个");
}

void send_mesh()//子节点使用来进行发送
{
  //建立发送内容
  String messageString = "{\"ESP32_" + WiFi.macAddress() + "\":";
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


void mesh_init()
{
  mesh.setDebugMsgTypes( ERROR | STARTUP ); 
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT ); 
  mesh.onReceive(&receivedCallback_main_mesh);
}