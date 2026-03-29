#pragma once

#include <stdint.h>

#include "ffb/ffb_types.h"
#include "types/control_types.h"

void setup_ffb_effects();
void reset_ffb_effects();

void ffb_set_enabled(bool enabled, uint32_t now_ms);
void ffb_set_device_gain(uint8_t gain_percent, uint32_t now_ms);
void ffb_note_host_activity(uint32_t now_ms);
void ffb_set_host_connection(bool connected, uint32_t now_ms);

bool ffb_has_active_effects();
const FfbDeviceState &ffb_get_device_state();

bool ffb_prepare_effect(uint8_t effect_id, FfbEffectType type, uint8_t gain_percent);
bool ffb_update_effect_parameters(uint8_t effect_id,
                                  uint8_t gain_percent,
                                  uint16_t duration_ms,
                                  int16_t direction,
                                  uint16_t start_delay_ms = 0);
bool ffb_set_envelope(uint8_t effect_id,
                      uint8_t attack_level,
                      uint8_t fade_level,
                      uint16_t attack_time_ms,
                      uint16_t fade_time_ms);
bool ffb_set_constant_force(uint8_t effect_id, int16_t magnitude, uint8_t gain_percent);
bool ffb_set_periodic(uint8_t effect_id,
                      int16_t magnitude,
                      uint16_t period_ms,
                      uint16_t phase,
                      int16_t offset,
                      uint8_t gain_percent);
bool ffb_set_ramp(uint8_t effect_id,
                  int16_t start_level,
                  int16_t end_level,
                  uint8_t gain_percent);
bool ffb_set_condition(uint8_t effect_id,
                       int16_t center,
                       int16_t deadband,
                       int16_t coefficient_positive,
                       int16_t coefficient_negative,
                       uint8_t gain_percent);
bool ffb_set_spring(uint8_t effect_id,
                    int16_t center,
                    int16_t deadband,
                    int16_t coefficient_positive,
                    int16_t coefficient_negative,
                    uint8_t gain_percent);
bool ffb_set_damper(uint8_t effect_id,
                    int16_t coefficient_positive,
                    int16_t coefficient_negative,
                    uint8_t gain_percent);
void ffb_start_effect(uint8_t effect_id, uint32_t now_ms, uint8_t loop_count = 1);
void ffb_stop_effect(uint8_t effect_id);
void ffb_clear_effect(uint8_t effect_id);
void ffb_clear_all_effects();

bool ffb_get_effect_slot(uint8_t effect_id, FfbEffectSlot &slot);
uint8_t ffb_get_active_effect_count();
uint8_t ffb_get_allocated_effect_count();
uint8_t ffb_get_max_effect_slots();
bool ffb_is_effect_allocated(uint8_t effect_id);
bool ffb_is_enabled();

int16_t ffb_compute_base_force(const WheelInputState &input);
