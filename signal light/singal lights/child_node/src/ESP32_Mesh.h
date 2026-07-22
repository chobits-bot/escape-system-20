/*主节点的Mesh*/
// #include <painlessMesh.h>  // CAT1板子不需要Mesh
/*********************WiFi&MQTT**********************/
// WiFi 路由器配置
#define WIFI_SSID "CMCC-g2Wd"        // WiFi名称
#define WIFI_PASSWORD "wdmy3ya9"     // WiFi密码
// MQTT 服务器配置
#define MQTT_SERVER "42.193.218.29"  // 你的MQTT服务器地址
#define MQTT_PORT 1883               // MQTT端口
#define MQTT_USER "indicator_light"             // MQTT用户名
#define MQTT_PASSWORD "indicator_light"         // MQTT密码
// MQTT 主题配置
#define MQTT_PUBLISH_TOPIC "sensordata"    // 发布主题
#define MQTT_SUBSCRIBE_TOPIC "indicator_light/command" // 订阅主题
#define MQTT_PUBLISH_INTERVAL_MS 10000  // 发布间隔10秒
/*********************************************************/
#define PWM 33//场效应管
#define buzzer 32//蜂鸣器
#define electricity_pin 36//监测电流
boolean electricity_flag = NULL;//电流标志位
#define Mesh_Count 2    //往MQTT服务器发送消息间隔--5秒发送一次
int MQTT_send_count = 0;//往MQTT服务器发送消息间隔变量
int scanning = 0; // 扫描
int buzzer_count = 0;//蜂鸣器声音延时计数
/**********************************************************************/
char direction_LED_left = 0;  // 显示的方式
char dynamic_static_left = 0; // 1--动态  0--静态
char DLED_Enable_left = 0;    // 1--使能--开启默认的箭头指示  0--不使能
/**********************************************************************/
char direction_LED_mid = 0;  // 显示的方式
char dynamic_static_mid = 0; // 1--动态  0--静态
char DLED_Enable_mid = 0;    // 1--使能--开启默认的箭头指示  0--不使能
/**********************************************************************/
char direction_LED_right = 0;  // 显示的方式
char dynamic_static_right = 0; // 1--动态  0--静态
char DLED_Enable_right = 0;    // 1--使能--开启默认的箭头指示  0--不使能
/**********************************************************************/
char silky_left = 0;
char silky_mid = 0;
char silky_right = 0;
uint8_t Status,AQI;
bool ens160_available = false;
uint16_t TVOC,ECO2;
float temp = 20,hum = 50;
int electricity_data = 0;//电流值
int buzzer_flag = 1;//蜂鸣器标志位-1---关闭2---打开
boolean electricity_Current = true;//电流使能位
boolean ESP32_electricity_flag = 0;//当前状态 1-报警 0-无报警 -电
boolean ESP32_fires_flag = 0;//当前状态 1-报警 0-无报警 -火
/*********************************************************/
// WiFi定位逃生功能
WebServer server(80);  // Web服务器端口80
String escapeDirection = "right";  // 逃生方向：left/right/up/down
String escapeStatus = "normal";    // 状态：normal/fire
int connectedDevices = 0;          // 连接的设备数
// MQTT主题
String topicString = "";
String theme_re = "";

void receiveCallback(byte *payload);
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
  analogWrite(PWM,250);
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
  analogWrite(PWM,0);
}

void start_alarms()
{
  ESP32_fires_flag = 1;
  buzzer_start_alarms();
  dynamic_static_left = 1;
  dynamic_static_mid = 1;
  dynamic_static_right = 1;
  analogWrite(PWM,500);
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
  analogWrite(PWM,0);
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
    analogWrite(33,500);
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
    analogWrite(PWM,0);
  }else
  {
    Serial.println(msg);
    receiveCallback((byte*)msg.c_str());    
  }

}

void main_mesh_init()
{
  // CAT1模式：不需要Mesh初始化
  // Mesh初始化已移除，使用CAT1 4G通信
  Serial.println("CAT1模式：跳过Mesh初始化");
}