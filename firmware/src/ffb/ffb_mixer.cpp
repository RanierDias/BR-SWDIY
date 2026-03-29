#include "ffb/ffb_mixer.h"

#include <Arduino.h>

#include "ffb/ffb_effects.h"

MotorCommand ffb_mix_motor_command(const WheelInputState &input,
                                   const DeviceConfig &config,
                                   int8_t manual_output_percent)
{
    (void)input;

    MotorCommand command;

    int16_t requested_force = manual_output_percent;

    if (ffb_has_active_effects())
    {
        requested_force = ffb_compute_base_force(input);
    }

    requested_force = static_cast<int16_t>(
        (static_cast<int32_t>(requested_force) * config.gain) / 100);

    const int16_t limited_force = constrain(requested_force, -config.output_limit, config.output_limit);

    command.target_torque = requested_force;
    command.output_percent = static_cast<int8_t>(limited_force);
    command.saturated = (limited_force != requested_force);
    return command;
}
