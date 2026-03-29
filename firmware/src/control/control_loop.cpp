#include "control/control_loop.h"

#include <Arduino.h>

namespace
{
    constexpr uint8_t VELOCITY_FILTER_TAPS = 4;
    constexpr uint8_t ACCEL_FILTER_TAPS = 4;
    int16_t g_last_angle = 0;
    int16_t g_last_velocity = 0;
    uint32_t g_last_sample_us = 0;
    int16_t g_velocity_samples[VELOCITY_FILTER_TAPS] = {};
    int16_t g_accel_samples[ACCEL_FILTER_TAPS] = {};
    int32_t g_velocity_sum = 0;
    int32_t g_accel_sum = 0;
    uint8_t g_velocity_index = 0;
    uint8_t g_accel_index = 0;
    uint8_t g_velocity_count = 0;
    uint8_t g_accel_count = 0;

    int16_t push_filtered_sample(int16_t sample,
                                 int16_t *buffer,
                                 uint8_t tap_count,
                                 uint8_t &index,
                                 uint8_t &count,
                                 int32_t &sum)
    {
        sum -= buffer[index];
        buffer[index] = sample;
        sum += sample;

        index = static_cast<uint8_t>((index + 1U) % tap_count);
        if (count < tap_count)
        {
            ++count;
        }

        return static_cast<int16_t>(sum / count);
    }
}

void setup_control_loop()
{
    g_last_angle = 0;
    g_last_velocity = 0;
    g_last_sample_us = 0;
    g_velocity_sum = 0;
    g_accel_sum = 0;
    g_velocity_index = 0;
    g_accel_index = 0;
    g_velocity_count = 0;
    g_accel_count = 0;

    for (uint8_t i = 0; i < VELOCITY_FILTER_TAPS; ++i)
    {
        g_velocity_samples[i] = 0;
    }

    for (uint8_t i = 0; i < ACCEL_FILTER_TAPS; ++i)
    {
        g_accel_samples[i] = 0;
    }
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
        const int16_t raw_velocity = static_cast<int16_t>((delta_angle * 1000L) / static_cast<int32_t>(delta_us));
        const int16_t filtered_velocity = push_filtered_sample(raw_velocity,
                                                               g_velocity_samples,
                                                               VELOCITY_FILTER_TAPS,
                                                               g_velocity_index,
                                                               g_velocity_count,
                                                               g_velocity_sum);
        state.angular_velocity = filtered_velocity;

        const int16_t raw_acceleration = static_cast<int16_t>(
            ((static_cast<int32_t>(filtered_velocity) - g_last_velocity) * 1000L) / static_cast<int32_t>(delta_us));
        state.angular_acceleration = push_filtered_sample(raw_acceleration,
                                                          g_accel_samples,
                                                          ACCEL_FILTER_TAPS,
                                                          g_accel_index,
                                                          g_accel_count,
                                                          g_accel_sum);
        g_last_velocity = filtered_velocity;
    }

    g_last_angle = angle;
    g_last_sample_us = now_us;
    return state;
}
