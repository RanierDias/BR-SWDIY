#pragma once

#include <stdint.h>

enum class EncoderType : uint8_t
{
    IncrementalAB = 0,
    AS5600 = 1,
};

void setup_encoder();
int16_t get_encoder_position();
EncoderType get_encoder_type();
