#pragma once

#include <stdint.h>

struct WheelInputState
{
    int16_t angle = 0;
    int16_t angular_velocity = 0;
    int16_t angular_acceleration = 0;
    uint16_t throttle = 0;
    uint16_t brake = 0;
    uint16_t clutch = 0;
    uint16_t buttons = 0;
    uint32_t sample_time_us = 0;
};

struct FfbDeviceState
{
    uint8_t device_gain = 100;
    bool ffb_enabled = false;
    bool host_connected = false;
    uint32_t last_ffb_packet_ms = 0;
};

struct MotorCommand
{
    int16_t target_torque = 0;
    int8_t output_percent = 0;
    bool saturated = false;
    bool safety_limited = false;
};
