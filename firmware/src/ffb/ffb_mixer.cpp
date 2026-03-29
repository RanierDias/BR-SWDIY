#include "ffb/ffb_mixer.h"

#include <Arduino.h>

#include "ffb/ffb_effects.h"

namespace
{
    constexpr int8_t OUTPUT_SLEW_STEP = 18;
    constexpr int8_t OUTPUT_CROSSOVER_STEP = 28;
    constexpr int8_t MIN_ACTIVE_FORCE = 6;
    constexpr int8_t CENTER_DAMPING_FORCE = 4;
    constexpr int8_t CENTER_BLEND_WINDOW = 12;
    constexpr int16_t CENTER_ANGLE_WINDOW = 120;
    constexpr int16_t CENTER_VELOCITY_THRESHOLD = 2;
    int8_t g_last_output_percent = 0;

    int16_t compute_baseline_support(const WheelInputState &input, const DeviceConfig &config)
    {
        int16_t support_force = 0;

        if (config.damper > 0)
        {
            support_force += static_cast<int16_t>(
                -(static_cast<int32_t>(input.angular_velocity) * config.damper) / 8L);
        }

        if (config.inertia > 0)
        {
            support_force += static_cast<int16_t>(
                -(static_cast<int32_t>(input.angular_acceleration) * config.inertia) / 16L);
        }

        if (config.friction > 0)
        {
            if (input.angular_velocity > CENTER_VELOCITY_THRESHOLD)
            {
                support_force -= static_cast<int16_t>(config.friction);
            }
            else if (input.angular_velocity < -CENTER_VELOCITY_THRESHOLD)
            {
                support_force += static_cast<int16_t>(config.friction);
            }
        }

        return constrain(support_force, -12, 12);
    }

    int8_t apply_slew_limit(int8_t target_output)
    {
        const bool direction_change = target_output != 0 &&
                                      g_last_output_percent != 0 &&
                                      ((target_output > 0) != (g_last_output_percent > 0));

        if (direction_change)
        {
            const int16_t delta = static_cast<int16_t>(target_output) - g_last_output_percent;
            if (delta > OUTPUT_CROSSOVER_STEP)
            {
                target_output = static_cast<int8_t>(g_last_output_percent + OUTPUT_CROSSOVER_STEP);
            }
            else if (delta < -OUTPUT_CROSSOVER_STEP)
            {
                target_output = static_cast<int8_t>(g_last_output_percent - OUTPUT_CROSSOVER_STEP);
            }
        }

        const int16_t delta = static_cast<int16_t>(target_output) - g_last_output_percent;
        if (!direction_change && delta > OUTPUT_SLEW_STEP)
        {
            target_output = static_cast<int8_t>(g_last_output_percent + OUTPUT_SLEW_STEP);
        }
        else if (!direction_change && delta < -OUTPUT_SLEW_STEP)
        {
            target_output = static_cast<int8_t>(g_last_output_percent - OUTPUT_SLEW_STEP);
        }

        g_last_output_percent = target_output;
        return target_output;
    }

    int8_t apply_motor_presence(int8_t output_percent)
    {
        if (output_percent == 0)
        {
            return 0;
        }

        const int8_t sign = (output_percent > 0) ? 1 : -1;
        int8_t magnitude = abs(output_percent);
        magnitude = static_cast<int8_t>(
            MIN_ACTIVE_FORCE + ((static_cast<int16_t>(magnitude) * (100 - MIN_ACTIVE_FORCE)) / 100));
        if (magnitude < MIN_ACTIVE_FORCE)
        {
            magnitude = MIN_ACTIVE_FORCE;
        }

        return static_cast<int8_t>(sign * magnitude);
    }

    int8_t apply_center_continuity(int8_t output_percent, const WheelInputState &input)
    {
        if (output_percent != 0)
        {
            return output_percent;
        }

        if (abs(input.angle) > CENTER_ANGLE_WINDOW ||
            abs(input.angular_velocity) < CENTER_VELOCITY_THRESHOLD)
        {
            return 0;
        }

        if (abs(g_last_output_percent) <= CENTER_BLEND_WINDOW)
        {
            return (input.angular_velocity > 0) ? -CENTER_DAMPING_FORCE : CENTER_DAMPING_FORCE;
        }

        return static_cast<int8_t>(g_last_output_percent / 3);
    }
}

MotorCommand ffb_mix_motor_command(const WheelInputState &input,
                                   const DeviceConfig &config)
{
    MotorCommand command;
    int16_t requested_force = 0;
    const bool ffb_running = ffb_has_active_effects() || ffb_is_enabled();

    if (ffb_has_active_effects())
    {
        requested_force = ffb_compute_base_force(input);
    }

    if (ffb_running)
    {
        requested_force = static_cast<int16_t>(
            requested_force + compute_baseline_support(input, config));
    }

    requested_force = static_cast<int16_t>(
        (static_cast<int32_t>(requested_force) * config.gain) / 100);

    const int16_t limited_force = constrain(requested_force, -config.output_limit, config.output_limit);
    int8_t output_percent = static_cast<int8_t>(limited_force);

    if (ffb_running)
    {
        output_percent = apply_motor_presence(output_percent);
        output_percent = apply_center_continuity(output_percent, input);
    }

    output_percent = apply_slew_limit(output_percent);

    command.target_torque = requested_force;
    command.output_percent = output_percent;
    command.saturated = (limited_force != requested_force);
    return command;
}
