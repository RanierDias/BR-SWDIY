#include "control/control_loop.h"

#include <Arduino.h>

namespace
{
    constexpr uint32_t MANUAL_OUTPUT_TIMEOUT_MS = 250;

    int8_t g_manual_output = 0;
    uint32_t g_last_manual_output_ms = 0;

    int16_t g_last_angle = 0;
    uint32_t g_last_sample_us = 0;
}

void setup_control_loop()
{
    g_manual_output = 0;
    g_last_manual_output_ms = 0;
    g_last_angle = 0;
    g_last_sample_us = 0;
}

void control_set_manual_output(int8_t output_percent, uint32_t now_ms)
{
    g_manual_output = constrain(output_percent, -100, 100);
    g_last_manual_output_ms = now_ms;
}

int8_t control_get_manual_output(uint32_t now_ms)
{
    if ((now_ms - g_last_manual_output_ms) > MANUAL_OUTPUT_TIMEOUT_MS)
    {
        return 0;
    }

    return g_manual_output;
}

WheelInputState control_build_input_state(int16_t angle,
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
        state.angular_velocity = static_cast<int16_t>((delta_angle * 1000L) / static_cast<int32_t>(delta_us));
    }

    g_last_angle = angle;
    g_last_sample_us = now_us;
    return state;
}
