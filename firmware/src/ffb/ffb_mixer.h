#pragma once

#include <stdint.h>

#include "config/config_model.h"
#include "types/control_types.h"

MotorCommand ffb_mix_motor_command(const WheelInputState &input,
                                   const DeviceConfig &config,
                                   int8_t manual_output_percent);
