#include "hw/encoder.h"

#include <Arduino.h>

namespace
{
    constexpr uint8_t ENCODER_PIN_A = 2;
    constexpr uint8_t ENCODER_PIN_B = 3;

    volatile int16_t g_encoder_position = 0;

    volatile const uint8_t *g_encoder_pin_a_reg = nullptr;
    volatile const uint8_t *g_encoder_pin_b_reg = nullptr;

    uint8_t g_encoder_pin_a_mask = 0;
    uint8_t g_encoder_pin_b_mask = 0;

    inline bool read_encoder_a()
    {
        return (*g_encoder_pin_a_reg & g_encoder_pin_a_mask) != 0;
    }

    inline bool read_encoder_b()
    {
        return (*g_encoder_pin_b_reg & g_encoder_pin_b_mask) != 0;
    }

    void on_encoder_a_change()
    {
        const bool a = read_encoder_a();
        const bool b = read_encoder_b();

        if (a == b)
        {
            ++g_encoder_position;
        }
        else
        {
            --g_encoder_position;
        }
    }
}

void setup_encoder()
{
    pinMode(ENCODER_PIN_A, INPUT_PULLUP);
    pinMode(ENCODER_PIN_B, INPUT_PULLUP);

    const uint8_t port_a = digitalPinToPort(ENCODER_PIN_A);
    const uint8_t port_b = digitalPinToPort(ENCODER_PIN_B);

    g_encoder_pin_a_reg = portInputRegister(port_a);
    g_encoder_pin_b_reg = portInputRegister(port_b);

    g_encoder_pin_a_mask = digitalPinToBitMask(ENCODER_PIN_A);
    g_encoder_pin_b_mask = digitalPinToBitMask(ENCODER_PIN_B);

    attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), on_encoder_a_change, CHANGE);
}

int16_t get_encoder_position()
{
    noInterrupts();
    const int16_t position = g_encoder_position;

    interrupts();
    return position;
}

void reset_encoder_position()
{
    noInterrupts();
    g_encoder_position = 0;
    interrupts();
}
