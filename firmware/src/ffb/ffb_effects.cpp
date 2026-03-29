#include "ffb/ffb_effects.h"

#include <Arduino.h>

namespace
{
    constexpr uint8_t MAX_EFFECT_SLOTS = 8;
    static int16_t SINE_WAVE_TABLE[256] = {
        0, 3, 6, 9, 12, 15, 18, 21, 24, 28, 31, 34, 37, 40, 43, 46,
        48, 51, 54, 57, 60, 63, 65, 68, 71, 73, 76, 78, 81, 83, 85, 88,
        90, 92, 94, 96, 98, 100, 102, 104, 106, 107, 109, 111, 112, 113, 115, 116,
        117, 118, 120, 121, 122, 122, 123, 124, 125, 125, 126, 126, 126, 127, 127, 127,
        127, 127, 127, 127, 126, 126, 126, 125, 125, 124, 123, 122, 122, 121, 120, 118,
        117, 116, 115, 113, 112, 111, 109, 107, 106, 104, 102, 100, 98, 96, 94, 92,
        90, 88, 85, 83, 81, 78, 76, 73, 71, 68, 65, 63, 60, 57, 54, 51,
        48, 46, 43, 40, 37, 34, 31, 28, 24, 21, 18, 15, 12, 9, 6, 3,
        0, -3, -6, -9, -12, -15, -18, -21, -24, -28, -31, -34, -37, -40, -43, -46,
        -48, -51, -54, -57, -60, -63, -65, -68, -71, -73, -76, -78, -81, -83, -85, -88,
        -90, -92, -94, -96, -98, -100, -102, -104, -106, -107, -109, -111, -112, -113, -115, -116,
        -117, -118, -120, -121, -122, -122, -123, -124, -125, -125, -126, -126, -126, -127, -127, -127,
        -127, -127, -127, -127, -126, -126, -126, -125, -125, -124, -123, -122, -122, -121, -120, -118,
        -117, -116, -115, -113, -112, -111, -109, -107, -106, -104, -102, -100, -98, -96, -94, -92,
        -90, -88, -85, -83, -81, -78, -76, -73, -71, -68, -65, -63, -60, -57, -54, -51,
        -48, -46, -43, -40, -37, -34, -31, -28, -24, -21, -18, -15, -12, -9, -6, -3};

    FfbEffectSlot g_effect_slots[MAX_EFFECT_SLOTS];
    FfbDeviceState g_ffb_state;

    int16_t clamp_force(int32_t value)
    {
        return constrain(value, -100, 100);
    }

    int16_t apply_signed_condition(int16_t value,
                                   int16_t deadband,
                                   int16_t coefficient_positive,
                                   int16_t coefficient_negative)
    {
        if (value > deadband)
        {
            return clamp_force((static_cast<int32_t>(value - deadband) * coefficient_positive) / 100);
        }

        if (value < -deadband)
        {
            return clamp_force((static_cast<int32_t>(value + deadband) * coefficient_negative) / 100);
        }

        return 0;
    }

    int16_t apply_gain(int16_t value, uint8_t gain)
    {
        return static_cast<int16_t>((static_cast<int32_t>(value) * gain) / 100);
    }

    int16_t apply_signed_friction(int16_t value,
                                  int16_t deadband,
                                  int16_t coefficient_positive,
                                  int16_t coefficient_negative)
    {
        if (value > deadband)
        {
            return constrain(coefficient_positive, 0, 100);
        }

        if (value < -deadband)
        {
            return -constrain(coefficient_negative, 0, 100);
        }

        return 0;
    }

    int16_t sample_periodic_wave(const FfbPeriodic &periodic, uint32_t elapsed_ms)
    {
        const uint16_t period_ms = (periodic.period_ms == 0) ? 1 : periodic.period_ms;

        const uint32_t elapsed_phase = (elapsed_ms % period_ms) * 8192UL / period_ms;
        const uint32_t phase_offset = (static_cast<uint32_t>(periodic.phase % 36000U) * 8192UL) / 36000UL;
        const uint16_t phase = static_cast<uint16_t>((elapsed_phase + phase_offset) & 0x1FFFU);

        const uint8_t index = static_cast<uint8_t>(phase >> 5);
        const uint8_t next_index = static_cast<uint8_t>((index + 1U) & 0xFFU);
        const uint8_t fraction = static_cast<uint8_t>(phase & 0x1FU); // Resto de 5 bits (0-31)

        const int16_t a = SINE_WAVE_TABLE[index];
        const int16_t b = SINE_WAVE_TABLE[next_index];

        const int16_t interpolated = static_cast<int16_t>(
            a + ((static_cast<int32_t>(b - a) * fraction) >> 5));

        return static_cast<int16_t>(
            periodic.offset + ((static_cast<int32_t>(interpolated) * periodic.magnitude) / 127L));
    }

    int16_t apply_direction(int16_t value, int16_t direction)
    {
        constexpr int16_t DIRECTION_AXIS_DEADBAND_PERCENT = 6;

        uint16_t normalized = static_cast<uint16_t>(direction);
        if (normalized >= 36000U)
        {
            normalized %= 36000U;
        }

        int16_t axis_percent = 0;
        if (normalized < 9000U)
        {
            axis_percent = static_cast<int16_t>(100 - ((static_cast<int32_t>(normalized) * 100L) / 9000L));
        }
        else if (normalized < 18000U)
        {
            axis_percent = static_cast<int16_t>(-((static_cast<int32_t>(normalized - 9000U) * 100L) / 9000L));
        }
        else if (normalized < 27000U)
        {
            axis_percent = static_cast<int16_t>(-100 + ((static_cast<int32_t>(normalized - 18000U) * 100L) / 9000L));
        }
        else
        {
            axis_percent = static_cast<int16_t>((static_cast<int32_t>(normalized - 27000U) * 100L) / 9000L);
        }

        if (abs(axis_percent) < DIRECTION_AXIS_DEADBAND_PERCENT)
        {
            return 0;
        }

        return static_cast<int16_t>((static_cast<int32_t>(value) * axis_percent) / 100L);
    }

    int16_t apply_envelope(const FfbEffectSlot &slot, int16_t value, uint32_t elapsed_ms)
    {
        if (!slot.envelope.enabled)
        {
            return value;
        }

        int16_t envelope_percent = 100;

        if (slot.envelope.attack_time_ms > 0 && elapsed_ms < slot.envelope.attack_time_ms)
        {
            const int16_t attack_delta = static_cast<int16_t>(100 - slot.envelope.attack_level);
            envelope_percent = static_cast<int16_t>(
                slot.envelope.attack_level +
                ((static_cast<int32_t>(attack_delta) * elapsed_ms) / slot.envelope.attack_time_ms));
        }

        if (slot.duration_ms > 0 && slot.envelope.fade_time_ms > 0 && elapsed_ms < slot.duration_ms)
        {
            const uint32_t fade_start_ms = (slot.duration_ms > slot.envelope.fade_time_ms)
                                               ? (slot.duration_ms - slot.envelope.fade_time_ms)
                                               : 0;
            if (elapsed_ms >= fade_start_ms)
            {
                const uint32_t fade_elapsed_ms = elapsed_ms - fade_start_ms;
                const int16_t fade_delta = static_cast<int16_t>(slot.envelope.fade_level - 100);
                const int16_t fade_percent = static_cast<int16_t>(
                    100 + ((static_cast<int32_t>(fade_delta) * fade_elapsed_ms) / slot.envelope.fade_time_ms));
                if (fade_percent < envelope_percent)
                {
                    envelope_percent = fade_percent;
                }
            }
        }

        envelope_percent = constrain(envelope_percent, 0, 100);
        return static_cast<int16_t>((static_cast<int32_t>(value) * envelope_percent) / 100L);
    }

    bool is_effect_active(const FfbEffectSlot &slot, uint32_t now_ms)
    {
        if (!slot.allocated || !slot.enabled)
        {
            return false;
        }

        const uint32_t ready_time_ms = slot.start_time_ms + slot.start_delay_ms;

        if (now_ms < ready_time_ms)
        {
            return false;
        }

        if (slot.duration_ms == 0)
        {
            return true;
        }

        const uint32_t elapsed_since_ready = now_ms - ready_time_ms;

        if (slot.loop_count == 0)
        {
            return true;
        }

        if (slot.loop_count == 1)
        {
            return elapsed_since_ready <= slot.duration_ms;
        }

        const uint32_t total_window_ms = static_cast<uint32_t>(slot.duration_ms) * slot.loop_count;
        return elapsed_since_ready <= total_window_ms;
    }

    uint32_t effect_elapsed_ms(const FfbEffectSlot &slot, uint32_t now_ms)
    {
        const uint32_t ready_time_ms = slot.start_time_ms + slot.start_delay_ms;

        if (now_ms <= ready_time_ms)
        {
            return 0;
        }

        const uint32_t elapsed = now_ms - ready_time_ms;

        if (slot.duration_ms == 0)
        {
            return elapsed;
        }

        return elapsed % slot.duration_ms;
    }

    FfbEffectSlot *find_slot(uint8_t effect_id)
    {
        for (uint8_t i = 0; i < MAX_EFFECT_SLOTS; ++i)
        {
            if (g_effect_slots[i].allocated && g_effect_slots[i].effect_id == effect_id)
            {
                return &g_effect_slots[i];
            }
        }

        return nullptr;
    }

    FfbEffectSlot *find_or_allocate_slot(uint8_t effect_id)
    {
        if (FfbEffectSlot *slot = find_slot(effect_id))
        {
            return slot;
        }

        for (uint8_t i = 0; i < MAX_EFFECT_SLOTS; ++i)
        {
            if (!g_effect_slots[i].allocated)
            {
                g_effect_slots[i].allocated = true;
                g_effect_slots[i].effect_id = effect_id;
                g_effect_slots[i].gain = 100;
                return &g_effect_slots[i];
            }
        }

        return nullptr;
    }
}

void setup_ffb_effects()
{
    reset_ffb_effects();
}

void reset_ffb_effects()
{
    for (uint8_t i = 0; i < MAX_EFFECT_SLOTS; ++i)
    {
        g_effect_slots[i] = FfbEffectSlot{};
    }

    g_ffb_state = FfbDeviceState{};
}

void ffb_set_enabled(bool enabled, uint32_t now_ms)
{
    g_ffb_state.ffb_enabled = enabled;
    g_ffb_state.last_ffb_packet_ms = now_ms;

    if (!enabled)
    {
        ffb_clear_all_effects();
    }
}

void ffb_set_device_gain(uint8_t gain_percent, uint32_t now_ms)
{
    g_ffb_state.device_gain = constrain(gain_percent, 0, 100);
    g_ffb_state.last_ffb_packet_ms = now_ms;
}

void ffb_note_host_activity(uint32_t now_ms)
{
    g_ffb_state.host_connected = true;
    g_ffb_state.last_ffb_packet_ms = now_ms;
}

void ffb_set_host_connection(bool connected, uint32_t now_ms)
{
    const bool was_connected = g_ffb_state.host_connected;
    g_ffb_state.host_connected = connected;

    if (connected)
    {
        if (!was_connected)
        {
            g_ffb_state.last_ffb_packet_ms = now_ms;
        }
        return;
    }

    g_ffb_state.last_ffb_packet_ms = 0;
    g_ffb_state.ffb_enabled = false;
    ffb_clear_all_effects();
}

bool ffb_has_active_effects()
{
    for (uint8_t i = 0; i < MAX_EFFECT_SLOTS; ++i)
    {
        if (g_effect_slots[i].allocated && g_effect_slots[i].enabled)
        {
            return true;
        }
    }

    return false;
}

const FfbDeviceState &ffb_get_device_state()
{
    return g_ffb_state;
}

bool ffb_prepare_effect(uint8_t effect_id, FfbEffectType type, uint8_t gain_percent)
{
    FfbEffectSlot *slot = find_or_allocate_slot(effect_id);

    if (slot == nullptr)
    {
        return false;
    }

    slot->type = type;
    slot->gain = constrain(gain_percent, 0, 100);
    return true;
}

bool ffb_update_effect_parameters(uint8_t effect_id,
                                  uint8_t gain_percent,
                                  uint16_t duration_ms,
                                  int16_t direction,
                                  uint16_t start_delay_ms)
{
    FfbEffectSlot *slot = find_slot(effect_id);

    if (slot == nullptr)
    {
        return false;
    }

    slot->gain = constrain(gain_percent, 0, 100);
    slot->duration_ms = duration_ms;
    slot->direction = direction;
    slot->start_delay_ms = start_delay_ms;
    return true;
}

bool ffb_set_constant_force(uint8_t effect_id, int16_t magnitude, uint8_t gain_percent)
{
    FfbEffectSlot *slot = find_or_allocate_slot(effect_id);

    if (slot == nullptr)
    {
        return false;
    }

    slot->type = FfbEffectType::ConstantForce;
    slot->magnitude = constrain(magnitude, -100, 100);
    slot->gain = constrain(gain_percent, 0, 100);
    return true;
}

bool ffb_set_envelope(uint8_t effect_id,
                      uint8_t attack_level,
                      uint8_t fade_level,
                      uint16_t attack_time_ms,
                      uint16_t fade_time_ms)
{
    FfbEffectSlot *slot = find_slot(effect_id);

    if (slot == nullptr)
    {
        return false;
    }

    slot->envelope.enabled = (attack_time_ms > 0 || fade_time_ms > 0 ||
                              attack_level < 100 || fade_level < 100);
    slot->envelope.attack_level = constrain(attack_level, 0, 100);
    slot->envelope.fade_level = constrain(fade_level, 0, 100);
    slot->envelope.attack_time_ms = attack_time_ms;
    slot->envelope.fade_time_ms = fade_time_ms;
    return true;
}

bool ffb_set_periodic(uint8_t effect_id,
                      int16_t magnitude,
                      uint16_t period_ms,
                      uint16_t phase,
                      int16_t offset,
                      uint8_t gain_percent)
{
    FfbEffectSlot *slot = find_or_allocate_slot(effect_id);

    if (slot == nullptr)
    {
        return false;
    }

    slot->type = FfbEffectType::Sine;
    slot->periodic.magnitude = constrain(magnitude, -100, 100);
    slot->periodic.period_ms = constrain(period_ms, 1, 2000);
    slot->periodic.phase = phase;
    slot->periodic.offset = constrain(offset, -100, 100);
    slot->gain = constrain(gain_percent, 0, 100);
    return true;
}

bool ffb_set_ramp(uint8_t effect_id,
                  int16_t start_level,
                  int16_t end_level,
                  uint8_t gain_percent)
{
    FfbEffectSlot *slot = find_or_allocate_slot(effect_id);

    if (slot == nullptr)
    {
        return false;
    }

    slot->type = FfbEffectType::Ramp;
    slot->ramp_start = constrain(start_level, -100, 100);
    slot->ramp_end = constrain(end_level, -100, 100);
    slot->gain = constrain(gain_percent, 0, 100);
    return true;
}

bool ffb_set_condition(uint8_t effect_id,
                       int16_t center,
                       int16_t deadband,
                       int16_t coefficient_positive,
                       int16_t coefficient_negative,
                       uint8_t gain_percent)
{
    FfbEffectSlot *slot = find_slot(effect_id);

    if (slot == nullptr)
    {
        return false;
    }

    if (slot->type != FfbEffectType::Spring &&
        slot->type != FfbEffectType::Damper &&
        slot->type != FfbEffectType::Friction &&
        slot->type != FfbEffectType::Inertia)
    {
        return false;
    }

    slot->condition.center = center;
    slot->condition.deadband = constrain(deadband, 0, 2048);
    slot->condition.coefficient_positive = constrain(coefficient_positive, -100, 100);
    slot->condition.coefficient_negative = constrain(coefficient_negative, -100, 100);
    slot->gain = constrain(gain_percent, 0, 100);
    return true;
}

bool ffb_set_spring(uint8_t effect_id,
                    int16_t center,
                    int16_t deadband,
                    int16_t coefficient_positive,
                    int16_t coefficient_negative,
                    uint8_t gain_percent)
{
    FfbEffectSlot *slot = find_or_allocate_slot(effect_id);

    if (slot == nullptr)
    {
        return false;
    }

    slot->type = FfbEffectType::Spring;
    slot->condition.center = center;
    slot->condition.deadband = constrain(deadband, 0, 2048);
    slot->condition.coefficient_positive = constrain(coefficient_positive, -100, 100);
    slot->condition.coefficient_negative = constrain(coefficient_negative, -100, 100);
    slot->gain = constrain(gain_percent, 0, 100);
    return true;
}

bool ffb_set_damper(uint8_t effect_id,
                    int16_t coefficient_positive,
                    int16_t coefficient_negative,
                    uint8_t gain_percent)
{
    FfbEffectSlot *slot = find_or_allocate_slot(effect_id);

    if (slot == nullptr)
    {
        return false;
    }

    slot->type = FfbEffectType::Damper;
    slot->condition.coefficient_positive = constrain(coefficient_positive, -100, 100);
    slot->condition.coefficient_negative = constrain(coefficient_negative, -100, 100);
    slot->condition.deadband = 0;
    slot->condition.center = 0;
    slot->gain = constrain(gain_percent, 0, 100);
    return true;
}

void ffb_start_effect(uint8_t effect_id, uint32_t now_ms, uint8_t loop_count)
{
    if (FfbEffectSlot *slot = find_slot(effect_id))
    {
        slot->enabled = true;
        slot->start_time_ms = now_ms;
        slot->loop_count = loop_count;
    }
}

void ffb_stop_effect(uint8_t effect_id)
{
    if (FfbEffectSlot *slot = find_slot(effect_id))
    {
        slot->enabled = false;
    }
}

void ffb_clear_effect(uint8_t effect_id)
{
    if (FfbEffectSlot *slot = find_slot(effect_id))
    {
        *slot = FfbEffectSlot{};
    }
}

void ffb_clear_all_effects()
{
    for (uint8_t i = 0; i < MAX_EFFECT_SLOTS; ++i)
    {
        g_effect_slots[i] = FfbEffectSlot{};
    }
}

bool ffb_get_effect_slot(uint8_t effect_id, FfbEffectSlot &slot)
{
    FfbEffectSlot *found = find_slot(effect_id);

    if (found == nullptr)
    {
        return false;
    }

    slot = *found;
    return true;
}

uint8_t ffb_get_active_effect_count()
{
    uint8_t count = 0;

    for (uint8_t i = 0; i < MAX_EFFECT_SLOTS; ++i)
    {
        if (g_effect_slots[i].allocated && g_effect_slots[i].enabled)
        {
            ++count;
        }
    }

    return count;
}

uint8_t ffb_get_allocated_effect_count()
{
    uint8_t count = 0;

    for (uint8_t i = 0; i < MAX_EFFECT_SLOTS; ++i)
    {
        if (g_effect_slots[i].allocated)
        {
            ++count;
        }
    }

    return count;
}

uint8_t ffb_get_max_effect_slots()
{
    return MAX_EFFECT_SLOTS;
}

bool ffb_is_effect_allocated(uint8_t effect_id)
{
    return find_slot(effect_id) != nullptr;
}

bool ffb_is_enabled()
{
    return g_ffb_state.ffb_enabled;
}

int16_t ffb_compute_base_force(const WheelInputState &input)
{
    if (!g_ffb_state.ffb_enabled)
    {
        return 0;
    }

    const uint32_t now_ms = input.sample_time_us / 1000UL;
    int32_t total_force_accumulator = 0;

    for (uint8_t i = 0; i < MAX_EFFECT_SLOTS; ++i)
    {
        const FfbEffectSlot &slot = g_effect_slots[i];

        if (!is_effect_active(slot, now_ms))
        {
            continue;
        }

        int16_t effect_force = 0;
        const uint32_t elapsed_ms = effect_elapsed_ms(slot, now_ms);

        if (slot.type == FfbEffectType::ConstantForce)
        {
            int16_t base = apply_envelope(slot, slot.magnitude, elapsed_ms);
            effect_force = apply_direction(base, slot.direction);
        }
        else if (slot.type == FfbEffectType::Sine)
        {
            int16_t periodic = sample_periodic_wave(slot.periodic, elapsed_ms);
            int16_t shaped = apply_envelope(slot, periodic, elapsed_ms);
            effect_force = apply_direction(shaped, slot.direction);
        }
        else if (slot.type == FfbEffectType::Ramp)
        {
            int16_t ramp_raw = slot.ramp_start;
            if (slot.duration_ms > 0)
            {
                int32_t delta = static_cast<int32_t>(slot.ramp_end) - slot.ramp_start;
                ramp_raw += static_cast<int16_t>((delta * static_cast<int32_t>(elapsed_ms)) / slot.duration_ms);
            }
            int16_t shaped = apply_envelope(slot, constrain(ramp_raw, -100, 100), elapsed_ms);
            effect_force = apply_direction(shaped, slot.direction);
        }

        else if (slot.type == FfbEffectType::Spring)
        {
            const int16_t displacement = input.angle - slot.condition.center;
            effect_force = -apply_signed_condition(displacement,
                                                   slot.condition.deadband,
                                                   slot.condition.coefficient_positive,
                                                   slot.condition.coefficient_negative);
        }
        else if (slot.type == FfbEffectType::Damper)
        {
            effect_force = -apply_signed_condition(input.angular_velocity,
                                                   slot.condition.deadband,
                                                   slot.condition.coefficient_positive,
                                                   slot.condition.coefficient_negative);
        }
        else if (slot.type == FfbEffectType::Friction)
        {
            effect_force = -apply_signed_friction(input.angular_velocity,
                                                  slot.condition.deadband,
                                                  slot.condition.coefficient_positive,
                                                  slot.condition.coefficient_negative);
        }
        else if (slot.type == FfbEffectType::Inertia)
        {
            effect_force = -apply_signed_condition(input.angular_acceleration,
                                                   slot.condition.deadband,
                                                   slot.condition.coefficient_positive,
                                                   slot.condition.coefficient_negative);
        }

        total_force_accumulator += apply_gain(effect_force, slot.gain);
    }

    int16_t final_force = static_cast<int16_t>(
        (total_force_accumulator * g_ffb_state.device_gain) / 100);

    return constrain(final_force, -100, 100);
}
