#pragma once

#include "types/control_types.h"

struct UsbPidDebugState
{
    uint32_t input_reports_sent = 0;
    uint32_t get_report_count = 0;
    uint32_t set_report_count = 0;
    uint32_t feature_report_count = 0;
    uint32_t output_report_count = 0;
    uint32_t invalid_report_count = 0;
    uint8_t last_report_id = 0;
    uint8_t last_report_type = 0;
    uint8_t last_effect_id = 0;
    uint8_t last_operation = 0;
    uint16_t last_report_length = 0;
};

void setup_usb_wheel();
void update_usb_wheel();
bool usb_wheel_ready();
void usb_wheel_set_input_state(const WheelInputState &state);
uint16_t usb_wheel_get_steering_hid_value(int16_t angle_counts);
const UsbPidDebugState &usb_wheel_get_debug_state();
void usb_wheel_reset_debug_state();
