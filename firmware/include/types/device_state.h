#pragma once

enum class DeviceState
{
  BOOT,
  IDLE,
  READY,
  CALIBRATION,
  ACTIVE,
  FAULT,
};

inline const char *state_name(DeviceState state)
{
  switch (state)
  {
  case DeviceState::BOOT:
    return "BOOT";
  case DeviceState::IDLE:
    return "IDLE";
  case DeviceState::READY:
    return "READY";
  case DeviceState::CALIBRATION:
    return "CALIBRATION";
  case DeviceState::ACTIVE:
    return "ACTIVE";
  case DeviceState::FAULT:
    return "FAULT";
  default:
    return "UNKNOWN";
  }
}
