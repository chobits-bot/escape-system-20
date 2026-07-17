#include "ens160.h"
#include "i2c_bus.h"
#include "esp_log.h"
#include <math.h>

static const char *TAG = "ENS160";

static i2c_master_dev_handle_t ENS160_devHandle = NULL;


/* ============================================================
 *  内部静态函数 (模块内使用, 前缀 ens160_)
 * ============================================================ */

static esp_err_t ens160_write_reg(uint8_t reg, uint8_t data)
{
	return I2c_Write_Reg(ENS160_devHandle, reg, data);
}

static esp_err_t ens160_read_reg(uint8_t reg, uint8_t *data)
{
	return I2c_Read_Reg(ENS160_devHandle, reg, data);
}

static esp_err_t ens160_read_bytes(uint8_t reg, uint8_t *buffer, size_t count)
{
	return I2c_Read_Bytes(ENS160_devHandle, reg, buffer, count);
}

static esp_err_t ens160_write_bytes(uint8_t reg, uint8_t *buffer, size_t count)
{
	return I2c_Write_Bytes(ENS160_devHandle, reg, buffer, count);
}

static esp_err_t ens160_set_opmode(uint8_t mode)
{
	esp_err_t ret = ens160_write_reg(ENS160_OPMODE_ADDR, mode);
	if (ret != ESP_OK)
	{
		ESP_LOGE(TAG, "设置 OPMODE 失败!");
	}
	return ret;
}


/* ============================================================
 *  公开 API 函数 (外部使用, 前缀 ENS160_)
 * ============================================================ */

esp_err_t ENS160_Init(i2c_master_bus_handle_t busHandle)
{
	ESP_LOGI(TAG, "正在初始化 ENS160...");

	/* 加入I2C总线 */
	esp_err_t ret = I2c_Add_Device(busHandle, ENS160_ADDR, ENS160_I2C_FREQ, &ENS160_devHandle);
	if (ret != ESP_OK)
	{
		ESP_LOGE(TAG, "ENS160 加入 I2C 总线失败!");
		return ret;
	}

	/* 验证 PART_ID */
	uint16_t partID = ENS160_Read_PartID();
	if (partID != ENS160_EXPECTED_PART_ID)
	{
		ESP_LOGE(TAG, "ENS160 PART_ID 不匹配! 期望: 0x%04X, 实际: 0x%04X",
				 ENS160_EXPECTED_PART_ID, partID);
		return ESP_ERR_NOT_FOUND;
	}
	ESP_LOGI(TAG, "ENS160 PART_ID 验证通过: 0x%04X", partID);

	/* 进入空闲模式 */
	ret = ens160_set_opmode(ENS160_OPMODE_IDLE);
	if (ret != ESP_OK)
	{
		return ret;
	}

	/* 进入标准气体传感模式 */
	ret = ens160_set_opmode(ENS160_OPMODE_STANDARD);
	if (ret != ESP_OK)
	{
		return ret;
	}

	ESP_LOGI(TAG, "ENS160 初始化完成, 进入标准模式");
	return ESP_OK;
}

uint16_t ENS160_Read_PartID(void)
{
	uint8_t data_buf[2] = {0};

	esp_err_t ret = ens160_read_bytes(ENS160_PART_ID_ADDR, data_buf, 2);
	if (ret != ESP_OK)
	{
		ESP_LOGE(TAG, "ENS160 读取部件 ID 失败!");
		return 0;
	}

	return (uint16_t)(data_buf[0] | (data_buf[1] << 8));
}

esp_err_t ENS160_Set_Env_Comp(float tempCelsius, float rhPercent)
{
	uint16_t temp_val;
	uint16_t rh_val;
	uint8_t data_buf[2];

	/* 温度转换: (Celsius + 273.15) * 64 */
	temp_val = (uint16_t)((tempCelsius + 273.15f) * 64.0f);

	/* 湿度转换: % * 512 */
	rh_val = (uint16_t)(rhPercent * 512.0f);

	/* 写入温度补偿 (先低后高, 小端) */
	data_buf[0] = (uint8_t)(temp_val & 0xFF);
	data_buf[1] = (uint8_t)((temp_val >> 8) & 0xFF);
	esp_err_t ret = ens160_write_bytes(ENS160_TEMP_IN_ADDR, data_buf, 2);
	if (ret != ESP_OK)
	{
		ESP_LOGE(TAG, "写入温度补偿失败!");
		return ret;
	}

	/* 写入湿度补偿 (先低后高, 小端) */
	data_buf[0] = (uint8_t)(rh_val & 0xFF);
	data_buf[1] = (uint8_t)((rh_val >> 8) & 0xFF);
	ret = ens160_write_bytes(ENS160_RH_IN_ADDR, data_buf, 2);
	if (ret != ESP_OK)
	{
		ESP_LOGE(TAG, "写入湿度补偿失败!");
		return ret;
	}

	ESP_LOGI(TAG, "温湿度补偿已设置: Temp=%.1f°C (0x%04X), RH=%.1f%% (0x%04X)",
			 tempCelsius, temp_val, rhPercent, rh_val);

	return ESP_OK;
}

esp_err_t ENS160_Read_Status(ENS160_Status_t *status)
{
	if (status == NULL)
	{
		return ESP_ERR_INVALID_ARG;
	}

	uint8_t raw_status = 0;
	esp_err_t ret = ens160_read_reg(ENS160_DEVICE_STATUS_ADDR, &raw_status);
	if (ret != ESP_OK)
	{
		ESP_LOGE(TAG, "读取设备状态失败!");
		return ret;
	}

	status->is_running   = (bool)(raw_status & ENS160_STATUS_STATAS_BIT);
	status->has_error    = (bool)(raw_status & ENS160_STATUS_STATER_BIT);
	status->validity     = (ENS160_Validity_t)((raw_status & ENS160_STATUS_VALIDITY_MASK) >> 2);
	status->has_new_data = (bool)(raw_status & ENS160_STATUS_NEWDAT_BIT);
	status->has_new_gpr  = (bool)(raw_status & ENS160_STATUS_NEWGPR_BIT);

	return ESP_OK;
}

esp_err_t ENS160_Read_AQI(uint8_t *aqi)
{
	if (aqi == NULL)
	{
		return ESP_ERR_INVALID_ARG;
	}

	return ens160_read_reg(ENS160_DATA_AQI_ADDR, aqi);
}

esp_err_t ENS160_Read_TVOC(uint16_t *tvoc)
{
	if (tvoc == NULL)
	{
		return ESP_ERR_INVALID_ARG;
	}

	uint8_t data_buf[2] = {0};
	esp_err_t ret = ens160_read_bytes(ENS160_DATA_TVOC_ADDR, data_buf, 2);
	if (ret != ESP_OK)
	{
		return ret;
	}

	*tvoc = (uint16_t)(data_buf[0] | (data_buf[1] << 8));
	return ESP_OK;
}

esp_err_t ENS160_Read_eCO2(uint16_t *eco2)
{
	if (eco2 == NULL)
	{
		return ESP_ERR_INVALID_ARG;
	}

	uint8_t data_buf[2] = {0};
	esp_err_t ret = ens160_read_bytes(ENS160_DATA_ECO2_ADDR, data_buf, 2);
	if (ret != ESP_OK)
	{
		return ret;
	}

	*eco2 = (uint16_t)(data_buf[0] | (data_buf[1] << 8));
	return ESP_OK;
}

esp_err_t ENS160_Set_OpMode(uint8_t mode)
{
	return ens160_set_opmode(mode);
}

esp_err_t ENS160_Read_All(ENS160_Data_t *data)
{
	if (data == NULL)
	{
		return ESP_ERR_INVALID_ARG;
	}

	/* 读取 AQI(0x21) + TVOC(0x22~0x23) + eCO2(0x24~0x25) 共5字节 */
	uint8_t data_buf[5] = {0};
	esp_err_t ret = ens160_read_bytes(ENS160_DATA_AQI_ADDR, data_buf, 5);
	if (ret != ESP_OK)
	{
		ESP_LOGE(TAG, "连续读取空气质量数据失败!");
		return ret;
	}

	data->aqi  = data_buf[0];
	data->tvoc = (uint16_t)(data_buf[1] | (data_buf[2] << 8));
	data->eco2 = (uint16_t)(data_buf[3] | (data_buf[4] << 8));

	return ESP_OK;
}