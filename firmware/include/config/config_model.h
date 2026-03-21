#pragma once

#include <stdint.h>

struct DeviceConfig
{
  uint8_t gain = 25;
  uint8_t damper = 0;
  uint8_t friction = 0;
  uint8_t inertia = 0;
  uint8_t spring = 0;
  uint8_t output_limit = 25;
  bool safe_start = true;
  uint16_t watchdog_ms = 250;
};
