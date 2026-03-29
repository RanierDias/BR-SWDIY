#pragma once

#include "types/device_state.h"
#include "stdint.h"

struct DeviceStatus
{
    DeviceState state;
    int16_t angle = 0;
    int16_t center = 0;
    int8_t output = 0;
    uint8_t limit = 0;
    uint16_t throttle = 0;
    uint16_t brake = 0;
    uint16_t clutch = 0;
    uint8_t buttons = 0;
    bool fault = false;
    bool motor_enabled = false;
    bool calibrated = false;
    bool config_saved = false;
};
