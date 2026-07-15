#include "EICEG.h"
#include <Arduino.h>
#include <WiFi.h>
#include <string.h>
#include <PubSubClient.h>
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
String topicString = "sensordata";//发送主题
String theme_re = "/signboard/control";//接收主题
//String topicString = "ESP32/" + WiFi.macAddress() + "/sensordata ";//发送主题
//String theme_re = "ESP32/" + WiFi.macAddress() + "/signboard/control";//接收主题

int Instruction_Mode()
{
    String serialData = "";
    char *correct_instruction1 = "ok";
    char *correct_instruction2 = "a";
    char *correct_instruction3 = "+CME ERROR:58";
    int count_time = 0;
    int count_time_main = 0;
    do{
        serialData = "";
        while (strstr(serialData.c_str(),correct_instruction2) == NULL)
        {
            serialData = "";
            if(count_time < 10)
            {
                delay(200);
                count_time++;
            }else
            {
                delay(200);
                Serial.println("+++指令失败了！");
                return 0;//失败
            }
            Serial2.print("+++");
            delay(10);
            if (Serial2.available()){                      //当串口接收到信息后
                Serial2.print("a"); 
                delay(10);
                serialData = Serial2.readString();    //将接收到的信息使用readString()存储于serialData变量

                Serial.println("结果："+serialData);
                if(strstr(serialData.c_str(),correct_instruction1) != NULL)
                {
                    Serial.println("成功进入调试模式！");
                    return 1;
                }else if((strstr(serialData.c_str(),correct_instruction3) != NULL))
                {
                    //已经在调试模式中
                    Serial.println("已经在调试模式中！");
                    return 1;
                }
            }
        }
        count_time = 0;
        serialData = "";
        if(count_time_main < 10)
        {
            delay(200);
            count_time_main++;
        }else
        {
            Serial.println("+++成功后，a失败了！");
            return 0;
        }
        Serial2.print("a");
        delay(10);
    }while (strstr(serialData.c_str(),correct_instruction1) == NULL);
    Serial.println("成功进入调试模式！");
    return 1;
}
int dialog_Mode()
{
    String serialData = "";
    char *correct_instruction = "OK";
    int count_time = 0;
    do{
        serialData = "";
        Serial2.println("AT+ENTM");
        delay(500);
        if (Serial2.available()){                      //当串口接收到信息后
            serialData = Serial2.readString();    //将接收到的信息使用readString()存储于serialData变量
            //Serial.println("采集3:"+serialData);
        }else
        {
            if(count_time < 10)
            {
                delay(500);
                count_time++;
            }else
            {
                Serial.println("进入对话模式失败了！");
                return 0;
            }
        }
    }while (strstr(serialData.c_str(),correct_instruction) == NULL);
    Serial.println("进入对话模式成功");
    return 1;
}
int MQTT_Parameter_Settings()//MQTT参数设置
{
    String serialData = "";
    char *correct_instruction = "OK";
    int count_time = 0;
    do{
        serialData = "";
        Serial2.println("AT+WKMOD=MQTT,NOR");
        delay(10);
        if (Serial2.available()){                      //当串口接收到信息后
            serialData = Serial2.readString();    //将接收到的信息使用readString()存储于serialData变量
            //Serial.println("采集3:"+serialData);
        }else
        {
            if(count_time < 10)
            {
                delay(200);
                count_time++;
            }else
            {
                Serial.println("无法进入MQTT设置参数的模式！");
                return 0;
            }
        }

    }while (strstr(serialData.c_str(),correct_instruction) == NULL);
    Serial.println("已经进入MQTT设置参数设置模式！");
    return 1;
}
int Setting_MQTT_Version()//设置MQTT版本为v3.1.1
{

    String serialData = "";
    char *correct_instruction = "OK";
    int count_time = 0;
    do{
        serialData = "";
        Serial2.println("AT+MQTTVERSION=1");
        delay(10);
        if (Serial2.available()){                      //当串口接收到信息后
            serialData = Serial2.readString();    //将接收到的信息使用readString()存储于serialData变量
            Serial.println("设置MQTT版本结果:"+serialData);
        }else
        {
            if(count_time < 10)
            {
                delay(200);
                count_time++;
            }else
            {
                Serial.println("设置MQTT版本失败！");
                return 0;
            }
        }

    }while (strstr(serialData.c_str(),correct_instruction) == NULL);
    Serial.println("设置MQTT版本为v3.1.1成功！");
    return 1;
}
int Setting_MQTT_CleanSession()//设置CleanSession
{
    String serialData = "";
    char *correct_instruction = "OK";
    int count_time = 0;
    do{
        serialData = "";
        Serial2.println("AT+MQTTCLEAN=ON");
        delay(10);
        if (Serial2.available()){                      //当串口接收到信息后
            serialData = Serial2.readString();    //将接收到的信息使用readString()存储于serialData变量
            Serial.println("设置CleanSession结果:"+serialData);
        }else
        {
            if(count_time < 10)
            {
                delay(200);
                count_time++;
            }else
            {
                Serial.println("设置CleanSession失败！");
                return 0;
            }
        }

    }while (strstr(serialData.c_str(),correct_instruction) == NULL);
    Serial.println("设置CleanSession成功！");
    return 1;
}
int Setting_MQTT_server_parameters(String IP_Addr)//设置MQTT服务器参数
{
    String serialData = "";
    String send_data = "AT+MQTTSVR=" + IP_Addr + ",1883";
    char *correct_instruction = "OK";
    int count_time = 0;
    do{
        serialData = "";
        Serial2.println(send_data);
        delay(10);
        if (Serial2.available()){                      //当串口接收到信息后
            serialData = Serial2.readString();    //将接收到的信息使用readString()存储于serialData变量
            //Serial.println("采集3:"+serialData);
        }else
        {
            if(count_time < 10)
            {
                delay(200);
                count_time++;
            }else
            {
                Serial.println("设置MQTT服务器参数失败！");
                return 0;
            }
        }

    }while (strstr(serialData.c_str(),correct_instruction) == NULL);
    Serial.println("设置MQTT服务器参数成功！");
    return 1;
}
int Setting_the_ClientID_parameter(String ClientID)//设置ClientID参数
{
    String serialData = "";
    String send_data = "AT+MQTTCID=" + ClientID;
    char *correct_instruction = "OK";
    int count_time = 0;
    do{
        serialData = "";
        Serial2.println(send_data);
        delay(10);
        if (Serial2.available()){                      //当串口接收到信息后
            serialData = Serial2.readString();    //将接收到的信息使用readString()存储于serialData变量
            //Serial.println("采集3:"+serialData);
        }else
        {
            if(count_time < 10)
            {
                delay(200);
                count_time++;
            }else
            {
                Serial.println("设置ClientID参数失败！");
                return 0;
            }
        }

    }while (strstr(serialData.c_str(),correct_instruction) == NULL);
    Serial.println("设置ClientID参数成功！");
    return 1;
}
int Setting_the_User(String User)//设置User参数
{
    String serialData = "";
    String send_data = "AT+MQTTUSER=" + User;
    char *correct_instruction = "OK";
    int count_time = 0;
    do{
        serialData = "";
        Serial2.println(send_data);
        delay(10);
        if (Serial2.available()){                      //当串口接收到信息后
            serialData = Serial2.readString();    //将接收到的信息使用readString()存储于serialData变量
            //Serial.println("采集3:"+serialData);
        }else
        {
            if(count_time < 10)
            {
                delay(200);
                count_time++;
            }else
            {
                Serial.println("设置User参数失败！");
                return 0;
            }
        }

    }while (strstr(serialData.c_str(),correct_instruction) == NULL);
    Serial.println("设置User参数成功！");
    return 1;
}
int Setting_the_PSW(String PSW)//设置User参数
{
    String serialData = "";
    String send_data = "AT+MQTTPSW=" + PSW;
    char *correct_instruction = "OK";
    int count_time = 0;
    do{
        serialData = "";
        Serial2.println(send_data);
        delay(10);
        if (Serial2.available()){                      //当串口接收到信息后
            serialData = Serial2.readString();    //将接收到的信息使用readString()存储于serialData变量
            //Serial.println("采集3:"+serialData);
        }else
        {
            if(count_time < 10)
            {
                delay(200);
                count_time++;
            }else
            {
                Serial.println("设置密码参数失败！");
                return 0;
            }
        }

    }while (strstr(serialData.c_str(),correct_instruction) == NULL);
    Serial.println("设置密码参数成功！");
    return 1;
}
int Setting_Heartbeat_Packet_Enable()//设置心跳包使能-默认NET修改OFF
{
    String serialData = "";
    char *correct_instruction = "OK";
    int count_time = 0;
    do{
        serialData = "";
        Serial2.println("AT+HEARTEN=OFF");
        delay(10);
        if (Serial2.available()){                      //当串口接收到信息后
            serialData = Serial2.readString();    //将接收到的信息使用readString()存储于serialData变量
            //Serial.println("采集3:"+serialData);
        }else
        {
            if(count_time < 10)
            {
                delay(200);
                count_time++;
            }else
            {
                Serial.println("设置心跳包使能失败！");
                return 0;
            }
        }

    }while (strstr(serialData.c_str(),correct_instruction) == NULL);
    Serial.println("设置心跳包使能成功！");
    return 1;
}
int Setting_MQTTWILL()
{
    String serialData = "";
    char *correct_instruction = "OK";
    int count_time = 0;
    do{
        serialData = "";
        Serial2.println("AT+MQTTWILL=0");
        delay(10);
        if (Serial2.available()){                      //当串口接收到信息后
            serialData = Serial2.readString();    //将接收到的信息使用readString()存储于serialData变量
            //Serial.println("采集3:"+serialData);
        }else
        {
            if(count_time < 10)
            {
                delay(200);
                count_time++;
            }else
            {
                Serial.println("使能失败！");
                return 0;
            }
        }

    }while (strstr(serialData.c_str(),correct_instruction) == NULL);
    Serial.println("使能成功！");
    return 1;
}
int Setting_MQTTSUB()
{
    String serialData = "";
    String send_data = "AT+MQTTSUBTP=1,1," + theme_re + ",0";
    char *correct_instruction = "OK";
    int count_time = 0;
    do{
        serialData = "";
        Serial2.println(send_data);
        delay(10);
        if (Serial2.available()){                      //当串口接收到信息后
            serialData = Serial2.readString();    //将接收到的信息使用readString()存储于serialData变量
            //Serial.println("采集3:"+serialData);
        }else
        {
            if(count_time < 10)
            {
                delay(200);
                count_time++;
            }else
            {
                Serial.println("设置订阅主题失败！");
                return 0;
            }            
        }
    }while (strstr(serialData.c_str(),correct_instruction) == NULL);
    Serial.println("设置订阅主题成功！");
    return 1;
}
int Setting_MQTTPUB()
{
    String serialData = "";
    String send_data = "AT+MQTTPUBTP=1,1," + topicString + ",0,0";
    char *correct_instruction = "OK";
    int count_time = 0;
    do{
        serialData = "";
        Serial2.println(send_data);
        delay(10);
        if (Serial2.available()){                      //当串口接收到信息后
            serialData = Serial2.readString();    //将接收到的信息使用readString()存储于serialData变量
            //Serial.println("采集3:"+serialData);
        }else
        {
            if(count_time < 10)
            {
                delay(200);
                count_time++;
            }else
            {
                Serial.println("设置发布主题失败！");
                return 0;
            }            
        }
    }while (strstr(serialData.c_str(),correct_instruction) == NULL);
    Serial.println("设置发布主题成功！");
    return 1;
}
int Setting_SSLENOFF()//AT+SSLEN=OFF
{
    String serialData = "";
    char *correct_instruction = "OK";
    int count_time = 0;
    do{
        serialData = "";
        Serial2.println("AT+SSLEN=OFF");
        delay(10);
        if (Serial2.available()){                      //当串口接收到信息后
            serialData = Serial2.readString();    //将接收到的信息使用readString()存储于serialData变量
            //Serial.println("采集3:"+serialData);
        }else
        {
            if(count_time < 10)
            {
                delay(200);
                count_time++;
            }else
            {
                Serial.println("设置失败！");
                return 0;
            }            
        }
    }while (strstr(serialData.c_str(),correct_instruction) == NULL);
    Serial.println("设置成功！");
    return 1;
}
int Setting_reopen()//重启
{
    String serialData = "";
    char *correct_instruction = "OK";
    int count_time = 0;
    do{
        serialData = "";
        Serial2.println("AT+S");
        delay(10);
        if (Serial2.available()){                      //当串口接收到信息后
            serialData = Serial2.readString();    //将接收到的信息使用readString()存储于serialData变量
            //Serial.println("采集3:"+serialData);
        }else
        {
            if(count_time < 10)
            {
                delay(200);
                count_time++;
            }else
            {
                Serial.println("重启失败！");
                return 0;
            }            
        }
    }while (strstr(serialData.c_str(),correct_instruction) == NULL);
    Serial.println("开始重启！");
    while(strstr(serialData.c_str(),"WH-LTE-7S0") == NULL)//WH-LTE-7S0
    {
        //AT+S
        serialData = "";
        Serial2.println("AT+S");
        delay(10);
        if (Serial2.available()){                      //当串口接收到信息后
            serialData = Serial2.readString();    //将接收到的信息使用readString()存储于serialData变量
            //Serial.println("采集3:"+serialData);
        }else
        {
            delay(500);
        }
    }
    Serial.println("重启成功！");
    return 1;
}

void init_cat1_mqtt()
{
    load(0);
    Serial2.begin(115200, SERIAL_8N1, 35, 32);//不可调换

    String serialData = "";
    if(Instruction_Mode()){
        MQTT_Parameter_Settings();
        load(1);
        Setting_MQTT_Version();
        Setting_MQTT_CleanSession();
        //Setting_MQTT_server_parameters("124.220.31.151");
        Setting_MQTT_server_parameters("172.20.133.66");
        load(2);
        Setting_the_ClientID_parameter("test1" + WiFi.macAddress());
        load(3);
        Setting_the_User("");
        Setting_the_PSW("");
        load(4);
        Setting_Heartbeat_Packet_Enable();
        Setting_MQTTWILL();
        load(5);   
        Setting_MQTTSUB();
        Setting_MQTTPUB();
        load(6);
        Setting_SSLENOFF();
        Setting_reopen();
        load(7);
        Instruction_Mode();
        load(8);
        dialog_Mode();
    }else
    {
        load_fail();
        Serial.println("请检测设备连接！");
        delay(1000);
    }
}