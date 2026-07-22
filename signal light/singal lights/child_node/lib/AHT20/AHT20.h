#ifndef AHT20_H
#define AHT20_H

#include "Arduino.h"
#include "Wire.h"

#define AHT20_ADDRESS       0x38
#define AHT20_CMD_INIT      0xBE
#define AHT20_CMD_MEASURE   0xAC
#define AHT20_CMD_RESET     0xBA

class AHT20
{
public:
    AHT20(TwoWire *wire = &Wire);
    bool begin();
    bool isConnected();
    float readTemperature();
    float readHumidity();

private:
    TwoWire *_wire;
    bool _readData();
    uint32_t _rawTemperature;
    uint32_t _rawHumidity;
};

#endif