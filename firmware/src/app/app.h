#pragma once

#include "config/config_model.h"
#include "types/device_state.h"
#include "types/telemetry_types.h"
#include "config/calibration_model.h"

void setup_app();
void update_app();

const DeviceConfig &get_config();
const DeviceStatus &get_status();
const InputCalibration &get_pedal_calibration();

bool save_config();
bool load_config();
void reset_config();
bool recenter_encoder();
bool handle_motor(bool enable);

uint16_t get_max_angle();

bool set_max_angle(int value);
bool set_gain(int gain);
bool set_output_limit(int limit);
bool set_safe_start(bool safe_start);
bool set_watchdog(int watchdog_ms);
bool set_throttle_min(int value);
bool set_throttle_max(int value);
bool set_brake_min(int value);
bool set_brake_max(int value);
bool set_clutch_min(int value);
bool set_clutch_max(int value);
bool set_pedal_invert(bool enable);
bool set_output(int value);