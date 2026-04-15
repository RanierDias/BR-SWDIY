#pragma once

#include <stdint.h>

void setup_pedals();

uint16_t read_throttle_raw();
uint16_t read_brake_raw();
uint16_t read_clutch_raw();
