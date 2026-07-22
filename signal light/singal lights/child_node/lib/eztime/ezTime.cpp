#include "ezTime.h"

ezTime::ezTime() : _epoch(0) {}

void ezTime::setTime(unsigned long epoch) {
    _epoch = epoch;
}

unsigned long ezTime::getEpoch() {
    return _epoch;
}

int ezTime::hour() {
    return (_epoch % 86400L) / 3600;
}

int ezTime::minute() {
    return ((_epoch % 86400L) % 3600) / 60;
}

int ezTime::second() {
    return _epoch % 60;
}

int ezTime::day() {
    return 1;
}

int ezTime::month() {
    return 1;
}

int ezTime::year() {
    return 1970;
}