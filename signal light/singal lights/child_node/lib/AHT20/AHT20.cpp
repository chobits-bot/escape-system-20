#include "AHT20.h"

AHT20::AHT20(TwoWire *wire)
{
    _wire = wire;
}

bool AHT20::begin()
{
    _wire->begin();
    delay(100);
    
    // 初始化AHT20
    _wire->beginTransmission(AHT20_ADDRESS);
    _wire->write(AHT20_CMD_INIT);
    _wire->write(0x08);
    _wire->write(0x00);
    _wire->endTransmission();
    delay(10);
    
    return isConnected();
}

bool AHT20::isConnected()
{
    _wire->beginTransmission(AHT20_ADDRESS);
    return (_wire->endTransmission() == 0);
}

bool AHT20::_readData()
{
    // 发送测量命令
    _wire->beginTransmission(AHT20_ADDRESS);
    _wire->write(AHT20_CMD_MEASURE);
    _wire->write(0x33);
    _wire->write(0x00);
    _wire->endTransmission();
    
    delay(80); // 等待测量完成
    
    // 读取数据
    if (_wire->requestFrom(AHT20_ADDRESS, 7) != 7) return false;
    
    uint8_t data[7];
    for (int i = 0; i < 7; i++) {
        data[i] = _wire->read();
    }
    
    // 检查状态
    if ((data[0] & 0x18) != 0x18) return false;
    
    // 解析湿度和温度
    _rawHumidity = ((uint32_t)(data[1] << 12) | (uint32_t)(data[2] << 4) | (data[3] >> 4));
    _rawTemperature = ((uint32_t)(data[3] & 0x0F) << 16) | (uint32_t)(data[4] << 8) | data[5];
    
    return true;
}

float AHT20::readTemperature()
{
    if (!_readData()) return NAN;
    return ((float)_rawTemperature / 1048576) * 200 - 50;
}

float AHT20::readHumidity()
{
    if (!_readData()) return NAN;
    return ((float)_rawHumidity / 1048576) * 100;
}