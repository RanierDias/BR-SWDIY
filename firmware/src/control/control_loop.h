#pragma once

#include <stdint.h>

#include "types/control_types.h"

void setup_control_loop();

void control_set_manual_output(int8_t output_percent, uint32_t now_ms);
int8_t control_get_manual_output(uint32_t now_ms);

WheelInputState control_build_input_state(int16_t angle,
                                          uint16_t throttle,
                                          uint16_t brake,
                                          uint16_t clutch,
                                          uint16_t buttons,
                                          uint32_t now_us);
