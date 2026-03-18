#include "hw/pedals.h"

#include <Arduino.h>

namespace
{
    constexpr uint8_t THROTTLE_PIN = A0;
    constexpr uint8_t BRAKE_PIN = A1;
    constexpr uint8_t CLUTCH_PIN = A2;
}

void setup_pedals()
{
    pinMode(THROTTLE_PIN, INPUT);
    pinMode(BRAKE_PIN, INPUT);
    pinMode(CLUTCH_PIN, INPUT);
}

uint16_t read_throttle_raw()
{
    return analogRead(THROTTLE_PIN);
}

uint16_t read_brake_raw()
{
    return analogRead(BRAKE_PIN);
}

uint16_t read_clutch_raw()
{
    return analogRead(CLUTCH_PIN);
}
