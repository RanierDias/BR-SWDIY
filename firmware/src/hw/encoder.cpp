#include "hw/encoder.h"

#include <Arduino.h>

#ifndef ENCODER_AS5600_ENABLED
#define ENCODER_AS5600_ENABLED 0
#endif

#if ENCODER_AS5600_ENABLED
#include <Wire.h>
#endif

namespace
{
    constexpr uint8_t ENCODER_PIN_A = 2;
    constexpr uint8_t ENCODER_PIN_B = 3;
#if ENCODER_AS5600_ENABLED
    // The AS5600 magnetic build assumes the sensor is wired on this bus
    constexpr uint8_t AS5600_I2C_ADDRESS = 0x36;
    constexpr uint16_t AS5600_COUNTS_PER_REV = 4096;
    constexpr uint8_t AS5600_RAW_ANGLE_REG = 0x0C;
#endif

    volatile int16_t g_encoder_position = 0;
    EncoderType g_encoder_type = EncoderType::IncrementalAB;
#if ENCODER_AS5600_ENABLED
    int32_t g_as5600_cumulative_position = 0;
    uint16_t g_as5600_last_raw = 0;
    bool g_as5600_available = false;
#endif

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

#if !ENCODER_AS5600_ENABLED
    void setup_incremental_encoder()
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
        g_encoder_type = EncoderType::IncrementalAB;
    }
#endif

#if ENCODER_AS5600_ENABLED
    bool read_as5600_raw(uint16_t &raw)
    {
        Wire.beginTransmission(AS5600_I2C_ADDRESS);
        Wire.write(AS5600_RAW_ANGLE_REG);
        if (Wire.endTransmission(false) != 0)
        {
            return false;
        }

        const uint8_t requested = 2;
        if (Wire.requestFrom(static_cast<int>(AS5600_I2C_ADDRESS), static_cast<int>(requested)) != requested)
        {
            return false;
        }

        const uint8_t high = Wire.read();
        const uint8_t low = Wire.read();
        raw = static_cast<uint16_t>(((static_cast<uint16_t>(high) << 8) | low) & 0x0FFFU);
        return true;
    }

    void setup_as5600_encoder()
    {
        uint16_t raw = 0;
        if (!read_as5600_raw(raw))
        {
            g_encoder_type = EncoderType::AS5600;
            g_as5600_available = false;
            g_as5600_last_raw = 0;
            g_as5600_cumulative_position = 0;
            return;
        }

        g_encoder_type = EncoderType::AS5600;
        g_as5600_available = true;
        g_as5600_last_raw = raw;
        g_as5600_cumulative_position = raw;
    }
#endif
}

void setup_encoder()
{
#if ENCODER_AS5600_ENABLED
    setup_as5600_encoder();
#else
    setup_incremental_encoder();
#endif
}

int16_t get_encoder_position()
{
#if ENCODER_AS5600_ENABLED
    if (g_encoder_type == EncoderType::AS5600)
    {
        if (!g_as5600_available)
        {
            return static_cast<int16_t>(constrain(g_as5600_cumulative_position, -32768L, 32767L));
        }

        uint16_t raw = 0;
        if (!read_as5600_raw(raw))
        {
            g_as5600_available = false;
            return static_cast<int16_t>(constrain(g_as5600_cumulative_position, -32768L, 32767L));
        }

        int16_t delta = static_cast<int16_t>(raw) - static_cast<int16_t>(g_as5600_last_raw);
        if (delta > static_cast<int16_t>(AS5600_COUNTS_PER_REV / 2))
        {
            delta -= AS5600_COUNTS_PER_REV;
        }
        else if (delta < -static_cast<int16_t>(AS5600_COUNTS_PER_REV / 2))
        {
            delta += AS5600_COUNTS_PER_REV;
        }

        g_as5600_cumulative_position += delta;
        g_as5600_last_raw = raw;
        g_as5600_available = true;
        return static_cast<int16_t>(constrain(g_as5600_cumulative_position, -32768L, 32767L));
    }
#endif

    noInterrupts();
    const int16_t position = g_encoder_position;

    interrupts();
    return position;
}

EncoderType get_encoder_type()
{
    return g_encoder_type;
}
