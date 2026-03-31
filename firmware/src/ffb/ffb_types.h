#pragma once

#include <stdint.h>

enum class FfbEffectType : uint8_t
{
    None = 0,
    ConstantForce,
    Spring,
    Damper,
    Friction,
    Inertia,
    Sine,
    Ramp,
};

struct FfbCondition
{
    int16_t center = 0;
    int16_t deadband = 0;
    int16_t coefficient_positive = 0;
    int16_t coefficient_negative = 0;
};

struct FfbPeriodic
{
    int16_t magnitude = 0;
    uint16_t period_ms = 0;
    uint16_t phase = 0;
    int16_t offset = 0;
};

struct FfbEnvelope
{
    bool enabled = false;
    uint8_t attack_level = 100;
    uint8_t fade_level = 100;
    uint16_t attack_time_ms = 0;
    uint16_t fade_time_ms = 0;
};

struct FfbEffectSlot
{
    uint8_t effect_id = 0;
    FfbEffectType type = FfbEffectType::None;
    bool allocated = false;
    bool enabled = false;
    uint8_t gain = 100;
    uint16_t duration_ms = 0;
    uint16_t start_delay_ms = 0;
    uint8_t loop_count = 1;
    int16_t magnitude = 0;
    int16_t ramp_start = 0;
    int16_t ramp_end = 0;
    uint16_t direction = 0;
    uint32_t start_time_ms = 0;
    FfbCondition condition;
    FfbPeriodic periodic;
    FfbEnvelope envelope;
};
