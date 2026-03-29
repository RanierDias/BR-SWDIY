#pragma once

#include <stdint.h>

struct PersistedConfig
{
    uint16_t magic = 0x4252;
    uint8_t version = 2;
    uint16_t max_angle = 1080;
    uint8_t gain = 25;
    uint8_t damper = 0;
    uint8_t friction = 0;
    uint8_t inertia = 0;
    uint8_t spring = 0;
    uint8_t output_limit = 25;
    bool safe_start = true;
    uint16_t watchdog_ms = 250;
    uint16_t throttle_min = 0;
    uint16_t throttle_max = 1023;
    uint16_t brake_min = 0;
    uint16_t brake_max = 1023;
    uint16_t clutch_min = 0;
    uint16_t clutch_max = 1023;
    bool invert_pedals = false;
};
