#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c_bus.h"
#include "RTOS_Communicate.h"
#include "ens160.h"
#include "aht2x.h"
#include "MLX90640.h"
#include "SensorFusion.h"
#include "wifi.h"
#include "mqtt.h"


static char TAG[] = "main";
static i2c_master_bus_handle_t busHandle = NULL;


void app_main(void)
{
	esp_err_t nvsRet = nvs_flash_init();
	if (nvsRet == ESP_ERR_NVS_NO_FREE_PAGES || nvsRet == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		nvsRet = nvs_flash_init();
	}
	ESP_ERROR_CHECK(nvsRet);

	esp_err_t initRet = I2c_Init_Bus(I2C_PORT, I2C_SDA_GPIO, I2C_SCL_GPIO, I2C_FREQ, &busHandle);
	if (initRet != ESP_OK)
	{
		ESP_LOGE(TAG, "I2C 总线初始化失败!");
		return;
	}

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

	/* 初始化 RTOS 通信模块 (数据中枢) */
	ESP_ERROR_CHECK(Comm_Init());

	/* 初始化传感器融合模块 (状态变量/互斥锁) */
	ESP_ERROR_CHECK(SensorFusion_Init(busHandle));

	/* 创建传感器任务, 优先级: MLX90640(5) > ENS160(4) > AHT2x(3) */
	xTaskCreatePinnedToCore(MLX90640_Task, "MLX90640_Task", 8192, busHandle, 5, NULL, 0);
	xTaskCreatePinnedToCore(ENS160_Task,    "ENS160_Task",    4096, busHandle, 4, NULL, 0);
	xTaskCreatePinnedToCore(AHT2x_Task,     "AHT2x_Task",     4096, busHandle, 3, NULL, 0);

	/* 创建传感器融合任务 (优先级 2, 低于传感器任务, 确保数据新鲜) */
	xTaskCreatePinnedToCore(SensorFusion_Task, "SensorFusion", 10240, busHandle, 2, NULL, 0);

	while (1)
	{
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}