#include "control/control_loop.h"

#include <Arduino.h>

namespace
{
    int16_t g_last_angle = 0;
    int16_t g_last_velocity = 0;
    uint32_t g_last_sample_us = 0;
}

void setup_control_loop()
{
    g_last_angle = 0;
    g_last_velocity = 0;
    g_last_sample_us = 0;
}

WheelInputState control_build_input_state(
    int16_t angle,
    uint16_t throttle,
    uint16_t brake,
    uint16_t clutch,
    uint16_t buttons,
    uint32_t now_us)
{
    WheelInputState state;
    state.angle = angle;
    state.throttle = throttle;
    state.brake = brake;
    state.clutch = clutch;
    state.buttons = buttons;
    state.sample_time_us = now_us;

    if (g_last_sample_us != 0 && now_us > g_last_sample_us)
    {
        const int32_t delta_angle = static_cast<int32_t>(angle) - g_last_angle;
        const uint32_t delta_us = now_us - g_last_sample_us;
        const int16_t angular_velocity = static_cast<int16_t>((delta_angle * 1000L) / static_cast<int32_t>(delta_us));
        state.angular_velocity = angular_velocity;
        state.angular_acceleration = static_cast<int16_t>(
            ((static_cast<int32_t>(angular_velocity) - g_last_velocity) * 1000L) / static_cast<int32_t>(delta_us));
        g_last_velocity = angular_velocity;
    }

    g_last_angle = angle;
    g_last_sample_us = now_us;
    return state;
}
