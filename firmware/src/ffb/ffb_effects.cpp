#include "ffb/ffb_effects.h"

#include <Arduino.h>
#include <math.h>

namespace
{
    constexpr uint8_t MAX_EFFECT_SLOTS = 8;
    constexpr float PI_F = 3.14159265f;

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

        if (slot.loop_count <= 1)
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

    if (slot->type != FfbEffectType::Spring && slot->type != FfbEffectType::Damper)
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
        slot->loop_count = (loop_count == 0) ? 1 : loop_count;
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
    int16_t total_force = 0;

    for (uint8_t i = 0; i < MAX_EFFECT_SLOTS; ++i)
    {
        const FfbEffectSlot &slot = g_effect_slots[i];

        if (!is_effect_active(slot, now_ms))
        {
            continue;
        }

        if (slot.type == FfbEffectType::ConstantForce)
        {
            total_force += apply_gain(slot.magnitude, slot.gain);
        }
        else if (slot.type == FfbEffectType::Spring)
        {
            const int16_t displacement = input.angle - slot.condition.center;
            const int16_t spring_force = -apply_signed_condition(displacement,
                                                                 slot.condition.deadband,
                                                                 slot.condition.coefficient_positive,
                                                                 slot.condition.coefficient_negative);
            total_force += apply_gain(spring_force, slot.gain);
        }
        else if (slot.type == FfbEffectType::Damper)
        {
            const int16_t damper_force = -apply_signed_condition(input.angular_velocity,
                                                                 slot.condition.deadband,
                                                                 slot.condition.coefficient_positive,
                                                                 slot.condition.coefficient_negative);
            total_force += apply_gain(damper_force, slot.gain);
        }
        else if (slot.type == FfbEffectType::Sine)
        {
            const uint32_t elapsed_ms = effect_elapsed_ms(slot, now_ms);
            const float phase = ((2.0f * PI_F * elapsed_ms) / slot.periodic.period_ms) +
                                ((2.0f * PI_F * slot.periodic.phase) / 360.0f);
            const int16_t periodic_force = static_cast<int16_t>(
                slot.periodic.offset + (sinf(phase) * slot.periodic.magnitude));
            total_force += apply_gain(constrain(periodic_force, -100, 100), slot.gain);
        }
        else if (slot.type == FfbEffectType::Ramp)
        {
            if (slot.duration_ms == 0)
            {
                total_force += apply_gain(slot.ramp_end, slot.gain);
                continue;
            }

            const uint32_t elapsed_ms = effect_elapsed_ms(slot, now_ms);
            const int32_t delta = static_cast<int32_t>(slot.ramp_end) - slot.ramp_start;
            const int16_t ramp_force = static_cast<int16_t>(
                slot.ramp_start + ((delta * static_cast<int32_t>(elapsed_ms)) / slot.duration_ms));
            total_force += apply_gain(constrain(ramp_force, -100, 100), slot.gain);
        }
    }

    total_force = static_cast<int16_t>(
        (static_cast<int32_t>(total_force) * g_ffb_state.device_gain) / 100);

    return constrain(total_force, -100, 100);
}
