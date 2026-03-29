#include "safety/safety_manager.h"

#include <Arduino.h>

void setup_safety_manager()
{
}

MotorCommand safety_apply_motor_rules(const MotorCommand &requested,
                                      const DeviceConfig &config,
                                      const FfbDeviceState &ffb_state,
                                      bool motor_enabled,
                                      uint32_t now_ms)
{
    MotorCommand safe_command = requested;

    if (!motor_enabled)
    {
        safe_command.output_percent = 0;
        safe_command.target_torque = 0;
        safe_command.safety_limited = true;
        return safe_command;
    }

    if (ffb_state.ffb_enabled && ffb_state.host_connected && config.watchdog_ms > 0)
    {
        if ((now_ms - ffb_state.last_ffb_packet_ms) > config.watchdog_ms)
        {
            safe_command.output_percent = 0;
            safe_command.target_torque = 0;
            safe_command.safety_limited = true;
            return safe_command;
        }
    }

    safe_command.output_percent = constrain(safe_command.output_percent, -config.output_limit, config.output_limit);
    return safe_command;
}
