#pragma once

#include <stdint.h>

#include "config/config_model.h"
#include "types/control_types.h"

void setup_safety_manager();
MotorCommand safety_apply_motor_rules(const MotorCommand &requested,
                                      const DeviceConfig &config,
                                      const FfbDeviceState &ffb_state,
                                      bool motor_enabled,
                                      uint32_t now_ms);
