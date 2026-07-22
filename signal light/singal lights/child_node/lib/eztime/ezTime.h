#ifndef ezTime_h
#define ezTime_h

#include "Arduino.h"

class ezTime {
public:
    ezTime();
    void setTime(unsigned long epoch);
    unsigned long getEpoch();
    int hour();
    int minute();
    int second();
    int day();
    int month();
    int year();

private:
    unsigned long _epoch;
};

#endif