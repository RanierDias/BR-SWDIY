#pragma once

#include <stdint.h>

struct PedalCalibration
{
    uint16_t min_raw = 0;
    uint16_t max_raw = 1023;
};

struct InputCalibration
{
    PedalCalibration throttle;
    PedalCalibration brake;
    PedalCalibration clutch;
    bool invert_pedals = false;
};
