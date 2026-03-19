#pragma once

#include "config/config_model.h"
#include "types/device_state.h"
#include "types/telemetry_types.h"

void setup_app();
void update_app();

const DeviceConfig &get_config();
const DeviceStatus &get_status();

bool set_gain(int gain);
bool set_output_limit(int limit);
bool set_safe_start(bool safe_start);
bool set_watchdog(int watchdog_ms);

void handle_motor(bool enable);
void reset_angle();
