#pragma once

#include "types/control_types.h"

struct UsbFfbRuntimeStatus
{
    uint16_t output_report_count = 0;
    uint16_t feature_report_count = 0;
    uint8_t last_report_id = 0;
};

void setup_usb_wheel();
void update_usb_wheel();
bool usb_wheel_ready();
void usb_wheel_set_input_state(const WheelInputState &state);
const UsbFfbRuntimeStatus &usb_wheel_get_runtime_status();
