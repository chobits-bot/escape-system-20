#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c_bus.h"
#include "ens160.h"
#include "aht2x.h"
#include "wifi.h"
#include "mqtt.h"

static char TAG[] = "main";
static i2c_master_bus_handle_t busHandle = NULL;

static int Get_Timestamp(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int)tv.tv_sec;
}

static const char* Determine_Status(uint8_t aqi, uint16_t tvoc)
{
    if (aqi >= 4 || tvoc > 1000)
    {
        return "emergency";
    }
    else if (aqi >= 3 || tvoc > 500)
    {
        return "warning";
    }
    return "normal";
}

static int Map_Smoke_Level(uint16_t tvoc)
{
    int smokeLevel = (int)(tvoc / 10);
    if (smokeLevel > 100) smokeLevel = 100;
    return smokeLevel;
}

#define MQTT_TOPIC_FIRE_ALERT   "ceiling_light/data"
#define JSON_BUFFER_SIZE        512
#define DEVICE_ID               "ceiling_001"
#define DEVICE_TYPE             "ceiling_light"
#define LOCATION_FLOOR          1
#define LOCATION_ZONE           "A"


static void Sensor_Test(void)
{
	esp_err_t ret;

	/* ========== 1. 初始化 AHT2x ========== */
	ESP_LOGI(TAG, "--- 初始化 AHT2x ---");
	ret = AHT2x_Init(busHandle);
	if (ret != ESP_OK)
	{
		ESP_LOGE(TAG, "AHT2x 初始化失败! err=0x%X", ret);
		return;
	}

	/* ========== 2. 读取 AHT2x 温湿度数据 ========== */
	AHT2x_Data_t aht2xData;
	ret = AHT2x_Read_Data(&aht2xData);
	if (ret != ESP_OK)
	{
		ESP_LOGE(TAG, "AHT2x 读取数据失败! err=0x%X", ret);
		return;
	}
	ESP_LOGI(TAG, "AHT2x DATA:  Temperature=%.2f°C, Humidity=%.2f%%RH",
			 aht2xData.temperature, aht2xData.humidity);

	/* ========== 3. 初始化 ENS160 ========== */
	ESP_LOGI(TAG, "--- 初始化 ENS160 ---");
	ret = ENS160_Init(busHandle);
	if (ret != ESP_OK)
	{
		ESP_LOGE(TAG, "ENS160 初始化失败! err=0x%X", ret);
		return;
	}

	uint16_t partID = ENS160_Read_PartID();
	if (partID != 0)
	{
		ESP_LOGI(TAG, "ENS160 PART_ID: 0x%04X", partID);
	}
	else
	{
		ESP_LOGE(TAG, "ENS160 读取 PART_ID 失败!");
		return;
	}

	/* ========== 4. 使用 AHT2x 数据校准 ENS160 ========== */
	ret = ENS160_Set_Env_Comp(aht2xData.temperature, aht2xData.humidity);
	if (ret != ESP_OK)
	{
		ESP_LOGW(TAG, "ENS160 温湿度补偿设置失败, 继续测试...");
	}
	else
	{
		ESP_LOGI(TAG, "ENS160 温湿度补偿已设置: T=%.2f°C, RH=%.2f%%",
				 aht2xData.temperature, aht2xData.humidity);
	}

	/* ========== 5. 循环读取传感器数据 ========== */
	ENS160_Status_t ens160_status;
	ENS160_Data_t   ens160Data;
	AHT2x_Status_t  aht2xStatus;
	int loopCount = 0;

	bool aht2xValid = false;

	while (loopCount < 20)
	{
		/* 读取 AHT2x 温湿度并更新 ENS160 补偿 */
		ret = AHT2x_Read_Data(&aht2xData);
		if (ret == ESP_OK)
		{
			aht2xValid = true;
			ESP_LOGI(TAG, "AHT2x: T=%.2f°C, RH=%.2f%%",
					 aht2xData.temperature, aht2xData.humidity);

			/* 将实时温湿度写入 ENS160 进行补偿校准 */
			ENS160_Set_Env_Comp(aht2xData.temperature, aht2xData.humidity);
		}
		else
		{
			aht2xValid = false;
			ESP_LOGW(TAG, "AHT2x 读取失败, 跳过本次补偿");
		}

		/* 读取 ENS160 设备状态 */
		ret = ENS160_Read_Status(&ens160_status);
		if (ret == ESP_OK)
		{
			ESP_LOGI(TAG, "ENS160 STATUS: running=%d err=%d valid=%d new=%d gpr=%d",
					 ens160_status.is_running, ens160_status.has_error,
					 ens160_status.validity, ens160_status.has_new_data,
					 ens160_status.has_new_gpr);
		}

		/* 读取 ENS160 空气质量数据 */
		ret = ENS160_Read_All(&ens160Data);
		if (ret == ESP_OK)
		{
			ESP_LOGI(TAG, "ENS160 DATA:  AQI=%d, TVOC=%d ppb, eCO2=%d ppm",
					 ens160Data.aqi, ens160Data.tvoc, ens160Data.eco2);

			/* 当 AHT2x 和 ENS160 数据均有效时, 通过 MQTT 上传 JSON 数据 */
			if (aht2xValid && Mqtt_Is_Connected())
			{
				char jsonBuffer[JSON_BUFFER_SIZE];
				int smokeLevel = Map_Smoke_Level(ens160Data.tvoc);
				bool flameDetected = (ens160Data.aqi >= 4);
				const char* status = Determine_Status(ens160Data.aqi, ens160Data.tvoc);
				int timestamp = Get_Timestamp();

				int jsonLen = snprintf(jsonBuffer, sizeof(jsonBuffer),
					"{\"device_id\":\"%s\","
					"\"type\":\"%s\","
					"\"location\":{\"floor\":%d,\"zone\":\"%s\"},"
					"\"sensors\":{\"smoke_level\":%d,\"flame_detected\":%s,\"temperature\":%.1f},"
					"\"status\":\"%s\","
					"\"timestamp\":%d}",
					DEVICE_ID, DEVICE_TYPE,
					LOCATION_FLOOR, LOCATION_ZONE,
					smokeLevel, flameDetected ? "true" : "false", aht2xData.temperature,
					status, timestamp);

				if (jsonLen > 0 && jsonLen < (int)sizeof(jsonBuffer))
				{
					Mqtt_Publish(MQTT_TOPIC_FIRE_ALERT, jsonBuffer, jsonLen);
				}
				else
				{
					ESP_LOGE(TAG, "JSON 缓冲区不足, 需要 %d 字节", jsonLen);
				}
			}
		}
		else
		{
			ESP_LOGW(TAG, "ENS160 读取数据失败, 等待传感器就绪...");
		}

		loopCount++;
		vTaskDelay(pdMS_TO_TICKS(2000));
	}

	/* ========== 6. 测试完毕 ========== */
	ENS160_Set_OpMode(ENS160_OPMODE_DEEP_SLEEP);
	ESP_LOGI(TAG, "传感器测试完毕, ENS160 进入深度睡眠");
}


void app_main(void)
{
	esp_err_t initRet = I2c_Init_Bus(I2C_PORT, I2C_SDA_GPIO, I2C_SCL_GPIO, I2C_FREQ, &busHandle);
	if (initRet != ESP_OK)
	{
		ESP_LOGE(TAG, "I2C 总线初始化失败!");
		return;
	}

	esp_err_t nvsRet = nvs_flash_init();
	if (nvsRet == ESP_ERR_NVS_NO_FREE_PAGES || nvsRet == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		nvsRet = nvs_flash_init();
	}
	ESP_ERROR_CHECK(nvsRet);

	Wifi_Init();
	ESP_LOGI(TAG, "等待 WiFi 连接...");
	while (!Wifi_Is_Connected())
	{
		vTaskDelay(pdMS_TO_TICKS(500));
	}
	ESP_LOGI(TAG, "WiFi 已连接");

	Mqtt_Init();
	ESP_LOGI(TAG, "等待 MQTT 连接...");
	while (!Mqtt_Is_Connected())
	{
		vTaskDelay(pdMS_TO_TICKS(500));
	}
	ESP_LOGI(TAG, "MQTT 已连接");

	Sensor_Test();

	while (1)
	{
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}