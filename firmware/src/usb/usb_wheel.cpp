#include "usb/usb_wheel.h"

#include <Arduino.h>

#if defined(USBCON)
#include <USBAPI.h>
#include <USBCore.h>

#include "app/app.h"
#include "ffb/ffb_effects.h"

namespace
{
    constexpr uint8_t HID_REPORT_TYPE_OUTPUT = 0x02;
    constexpr uint8_t HID_REPORT_TYPE_FEATURE = 0x03;

    constexpr uint8_t JOYSTICK_REPORT_ID = 0x04;
    constexpr uint8_t PID_STATE_REPORT_ID = 0x02;

    constexpr uint8_t PID_SET_EFFECT_REPORT_ID = 0x01;
    constexpr uint8_t PID_SET_ENVELOPE_REPORT_ID = 0x02;
    constexpr uint8_t PID_SET_CONDITION_REPORT_ID = 0x03;
    constexpr uint8_t PID_SET_PERIODIC_REPORT_ID = 0x04;
    constexpr uint8_t PID_SET_CONSTANT_FORCE_REPORT_ID = 0x05;
    constexpr uint8_t PID_SET_RAMP_FORCE_REPORT_ID = 0x06;
    constexpr uint8_t PID_EFFECT_OPERATION_REPORT_ID = 0x0A;
    constexpr uint8_t PID_BLOCK_FREE_REPORT_ID = 0x0B;
    constexpr uint8_t PID_DEVICE_CONTROL_REPORT_ID = 0x0C;
    constexpr uint8_t PID_DEVICE_GAIN_REPORT_ID = 0x0D;

    constexpr uint8_t PID_CREATE_NEW_EFFECT_REPORT_ID = 0x05;
    constexpr uint8_t PID_BLOCK_LOAD_REPORT_ID = 0x06;
    constexpr uint8_t PID_POOL_REPORT_ID = 0x07;

    constexpr uint8_t PID_STATUS_DEVICE_PAUSED = 1 << 0;
    constexpr uint8_t PID_STATUS_ACTUATORS_ENABLED = 1 << 1;
    constexpr uint8_t PID_STATUS_SAFETY_SWITCH = 1 << 2;
    constexpr uint8_t PID_STATUS_ACTUATOR_POWER = 1 << 4;
    constexpr uint8_t PID_STATUS_EFFECT_PLAYING = 1 << 7;

    constexpr uint8_t EFFECT_OP_START = 1;
    constexpr uint8_t EFFECT_OP_START_SOLO = 2;
    constexpr uint8_t EFFECT_OP_STOP = 3;

    constexpr uint8_t DEVICE_CONTROL_DISABLE_ACTUATORS = 1;
    constexpr uint8_t DEVICE_CONTROL_ENABLE_ACTUATORS = 2;
    constexpr uint8_t DEVICE_CONTROL_STOP_ALL_EFFECTS = 3;
    constexpr uint8_t DEVICE_CONTROL_DEVICE_RESET = 4;
    constexpr uint8_t DEVICE_CONTROL_DEVICE_PAUSE = 5;
    constexpr uint8_t DEVICE_CONTROL_DEVICE_CONTINUE = 6;

    constexpr uint8_t BLOCK_LOAD_SUCCESS = 1;
    constexpr uint8_t BLOCK_LOAD_FULL = 2;
    constexpr uint8_t BLOCK_LOAD_ERROR = 3;

    constexpr uint8_t EFFECT_TYPE_CONSTANT_ORDINAL = 1;
    constexpr uint8_t EFFECT_TYPE_RAMP_ORDINAL = 2;
    constexpr uint8_t EFFECT_TYPE_SQUARE_ORDINAL = 3;
    constexpr uint8_t EFFECT_TYPE_SINE_ORDINAL = 4;
    constexpr uint8_t EFFECT_TYPE_TRIANGLE_ORDINAL = 5;
    constexpr uint8_t EFFECT_TYPE_SAW_UP_ORDINAL = 6;
    constexpr uint8_t EFFECT_TYPE_SAW_DOWN_ORDINAL = 7;
    constexpr uint8_t EFFECT_TYPE_SPRING_ORDINAL = 8;
    constexpr uint8_t EFFECT_TYPE_DAMPER_ORDINAL = 9;
    constexpr uint8_t EFFECT_TYPE_INERTIA_ORDINAL = 10;
    constexpr uint8_t EFFECT_TYPE_FRICTION_ORDINAL = 11;

    constexpr uint8_t EFFECT_TYPE_CONSTANT_USAGE = 0x26;
    constexpr uint8_t EFFECT_TYPE_RAMP_USAGE = 0x27;
    constexpr uint8_t EFFECT_TYPE_SQUARE_USAGE = 0x30;
    constexpr uint8_t EFFECT_TYPE_SINE_USAGE = 0x31;
    constexpr uint8_t EFFECT_TYPE_TRIANGLE_USAGE = 0x32;
    constexpr uint8_t EFFECT_TYPE_SAW_UP_USAGE = 0x33;
    constexpr uint8_t EFFECT_TYPE_SAW_DOWN_USAGE = 0x34;
    constexpr uint8_t EFFECT_TYPE_SPRING_USAGE = 0x40;
    constexpr uint8_t EFFECT_TYPE_DAMPER_USAGE = 0x41;
    constexpr uint8_t EFFECT_TYPE_INERTIA_USAGE = 0x42;
    constexpr uint8_t EFFECT_TYPE_FRICTION_USAGE = 0x43;

    constexpr uint32_t INPUT_REPORT_INTERVAL_MS = 2;
    constexpr uint32_t STATUS_REPORT_INTERVAL_MS = 20;

    struct __attribute__((packed)) SetEffectOutputReport
    {
        uint8_t report_id;
        uint8_t effect_block_index;
        uint8_t effect_type;
        uint16_t duration;
        uint16_t trigger_repeat_interval;
        int16_t gain;
        uint8_t trigger_button;
        uint8_t enable_axis;
        uint16_t direction;
        uint16_t start_delay;
    };

    struct __attribute__((packed)) SetEnvelopeOutputReport
    {
        uint8_t report_id;
        uint8_t effect_block_index;
        uint8_t attack_level;
        uint8_t fade_level;
        uint16_t attack_time;
        uint16_t fade_time;
    };

    struct __attribute__((packed)) SetConditionOutputReport
    {
        uint8_t report_id;
        uint8_t effect_block_index;
        uint8_t parameter_block_offset;
        int16_t cp_offset;
        int16_t positive_coefficient;
        uint8_t dead_band;
    };

    struct __attribute__((packed)) SetPeriodicOutputReport
    {
        uint8_t report_id;
        uint8_t effect_block_index;
        int16_t magnitude;
        int16_t offset;
        uint8_t phase;
        uint16_t period;
    };

    struct __attribute__((packed)) SetConstantForceOutputReport
    {
        uint8_t report_id;
        uint8_t effect_block_index;
        int16_t magnitude;
    };

    struct __attribute__((packed)) SetRampForceOutputReport
    {
        uint8_t report_id;
        uint8_t effect_block_index;
        int8_t ramp_start;
        int8_t ramp_end;
    };

    struct __attribute__((packed)) EffectOperationOutputReport
    {
        uint8_t report_id;
        uint8_t effect_block_index;
        uint8_t operation;
        uint8_t loop_count;
    };

    struct __attribute__((packed)) BlockFreeOutputReport
    {
        uint8_t report_id;
        uint8_t effect_block_index;
    };

    struct __attribute__((packed)) DeviceControlOutputReport
    {
        uint8_t report_id;
        uint8_t control;
    };

    struct __attribute__((packed)) DeviceGainOutputReport
    {
        uint8_t report_id;
        uint8_t device_gain;
    };

    struct __attribute__((packed)) CreateNewEffectFeatureReport
    {
        uint8_t report_id;
        uint8_t effect_type;
        uint16_t byte_count;
    };

    struct __attribute__((packed)) BlockLoadFeatureReport
    {
        uint8_t report_id;
        uint8_t effect_block_index;
        uint8_t load_status;
        uint16_t ram_pool_available;
    };

    struct __attribute__((packed)) PoolFeatureReport
    {
        uint8_t report_id;
        uint16_t ram_pool_size;
        uint8_t max_simultaneous_effects;
        uint8_t memory_management;
    };

    struct SentInputSnapshot
    {
        uint16_t steering = 0;
        uint16_t throttle = 0;
        uint16_t brake = 0;
        uint16_t clutch = 0;
        uint16_t aux_axis = 0;
        uint32_t buttons = 0;
        bool valid = false;
    };

    struct PidStatusPayload
    {
        uint8_t status = 0;
        uint8_t effect_block_index = 0;
    };

    WheelInputState g_input_state;
    SentInputSnapshot g_last_input_snapshot;
    PidStatusPayload g_last_status_payload;
    BlockLoadFeatureReport g_last_block_load_report = {PID_BLOCK_LOAD_REPORT_ID, 0, BLOCK_LOAD_FULL, 0};
    UsbFfbRuntimeStatus g_runtime_status;
    uint32_t g_last_input_report_ms = 0;
    uint32_t g_last_status_report_ms = 0;
    bool g_usb_session_active = false;

    template <typename T>
    bool report_has_length(uint16_t len)
    {
        return len >= sizeof(T);
    }

    template <typename T>
    const T *as_report(const uint8_t *data, uint16_t len)
    {
        if (!report_has_length<T>(len))
        {
            return nullptr;
        }

        return reinterpret_cast<const T *>(data);
    }

    uint8_t clamp_percent_from_u8(uint8_t value)
    {
        return (value > 100) ? 100 : value;
    }

    uint8_t level8_to_percent(uint8_t value)
    {
        return static_cast<uint8_t>(constrain((static_cast<uint16_t>(value) * 100U) / 255U, 0U, 100U));
    }

    uint8_t gain16_to_percent(int16_t gain)
    {
        if (gain <= 0)
        {
            return 100;
        }

        const int32_t scaled = (static_cast<int32_t>(gain) * 100L) / 32767L;
        return static_cast<uint8_t>(constrain(scaled, 0L, 100L));
    }

    int16_t signed16_to_percent_force(int16_t value)
    {
        return static_cast<int16_t>(constrain(static_cast<int32_t>(value) / 256L, -100L, 100L));
    }

    int16_t condition16_to_percent(int16_t value)
    {
        return static_cast<int16_t>(constrain(static_cast<int32_t>(value) / 256L, -100L, 100L));
    }

    uint16_t scale_axis_to_bits(uint16_t value, uint8_t bits)
    {
        const uint32_t max_output = (1UL << bits) - 1UL;
        const uint32_t scaled = static_cast<uint32_t>(value) * max_output / 1023UL;
        return static_cast<uint16_t>(constrain(scaled, 0UL, max_output));
    }

    uint16_t steering_to_hid(int16_t angle)
    {
        int32_t max_counts = static_cast<int32_t>(get_half_angle_counts());
        if (max_counts <= 0)
        {
            max_counts = 1200;
        }

        const int32_t clamped_angle = constrain(static_cast<int32_t>(angle), -max_counts, max_counts);
        const int32_t shifted = clamped_angle + max_counts;
        const uint32_t full_scale = static_cast<uint32_t>(max_counts) * 2UL;

        if (full_scale == 0)
        {
            return 32767;
        }

        const uint32_t scaled = static_cast<uint32_t>(shifted) * 65535UL / full_scale;
        return static_cast<uint16_t>(constrain(static_cast<int32_t>(scaled), 0L, 65535L));
    }

    bool is_valid_effect_id(uint8_t effect_id)
    {
        return effect_id > 0 && effect_id <= ffb_get_max_effect_slots();
    }

    uint8_t find_free_effect_id()
    {
        for (uint8_t effect_id = 1; effect_id <= ffb_get_max_effect_slots(); ++effect_id)
        {
            if (!ffb_is_effect_allocated(effect_id))
            {
                return effect_id;
            }
        }

        return 0;
    }

    uint16_t compute_ram_pool_available()
    {
        const uint8_t total = ffb_get_max_effect_slots();
        const uint8_t used_slots = ffb_get_allocated_effect_count();
        const uint8_t free_slots = (used_slots >= total) ? 0 : static_cast<uint8_t>(total - used_slots);
        const uint16_t bytes_per_effect = static_cast<uint16_t>(sizeof(FfbEffectSlot));

        return static_cast<uint16_t>(free_slots * bytes_per_effect);
    }

    uint16_t compute_ram_pool_size()
    {
        return static_cast<uint16_t>(ffb_get_max_effect_slots() * sizeof(FfbEffectSlot));
    }

    FfbEffectType decode_effect_type(uint8_t raw_type)
    {
        switch (raw_type)
        {
        case EFFECT_TYPE_CONSTANT_ORDINAL:
        case EFFECT_TYPE_CONSTANT_USAGE:
            return FfbEffectType::ConstantForce;
        case EFFECT_TYPE_RAMP_ORDINAL:
        case EFFECT_TYPE_RAMP_USAGE:
            return FfbEffectType::Ramp;
        case EFFECT_TYPE_SQUARE_ORDINAL:
        case EFFECT_TYPE_SINE_ORDINAL:
        case EFFECT_TYPE_TRIANGLE_ORDINAL:
        case EFFECT_TYPE_SAW_UP_ORDINAL:
        case EFFECT_TYPE_SAW_DOWN_ORDINAL:
        case EFFECT_TYPE_SQUARE_USAGE:
        case EFFECT_TYPE_SINE_USAGE:
        case EFFECT_TYPE_TRIANGLE_USAGE:
        case EFFECT_TYPE_SAW_UP_USAGE:
        case EFFECT_TYPE_SAW_DOWN_USAGE:
            return FfbEffectType::Sine;
        case EFFECT_TYPE_SPRING_ORDINAL:
        case EFFECT_TYPE_SPRING_USAGE:
            return FfbEffectType::Spring;
        case EFFECT_TYPE_DAMPER_ORDINAL:
        case EFFECT_TYPE_DAMPER_USAGE:
            return FfbEffectType::Damper;
        case EFFECT_TYPE_INERTIA_ORDINAL:
        case EFFECT_TYPE_INERTIA_USAGE:
            return FfbEffectType::Inertia;
        case EFFECT_TYPE_FRICTION_ORDINAL:
        case EFFECT_TYPE_FRICTION_USAGE:
            return FfbEffectType::Friction;
        default:
            return FfbEffectType::None;
        }
    }

    uint8_t current_effect_id_with_activity()
    {
        for (uint8_t effect_id = 1; effect_id <= ffb_get_max_effect_slots(); ++effect_id)
        {
            FfbEffectSlot slot;
            if (ffb_get_effect_slot(effect_id, slot) && slot.allocated && slot.enabled)
            {
                return effect_id;
            }
        }

        return 0;
    }

    PidStatusPayload build_status_payload()
    {
        PidStatusPayload payload;
        const FfbDeviceState &state = ffb_get_device_state();

        if (!state.ffb_enabled)
        {
            payload.status |= PID_STATUS_DEVICE_PAUSED;
        }
        else
        {
            payload.status |= PID_STATUS_ACTUATORS_ENABLED;
        }

        if (state.host_connected)
        {
            payload.status |= PID_STATUS_ACTUATOR_POWER;
            payload.status |= PID_STATUS_SAFETY_SWITCH;
        }

        const uint8_t effect_id = current_effect_id_with_activity();
        if (effect_id != 0)
        {
            payload.effect_block_index = static_cast<uint8_t>(PID_STATUS_EFFECT_PLAYING | effect_id);
        }

        return payload;
    }

    bool same_snapshot(const SentInputSnapshot &lhs, const SentInputSnapshot &rhs)
    {
        return lhs.valid == rhs.valid &&
               lhs.steering == rhs.steering &&
               lhs.throttle == rhs.throttle &&
               lhs.brake == rhs.brake &&
               lhs.clutch == rhs.clutch &&
               lhs.aux_axis == rhs.aux_axis &&
               lhs.buttons == rhs.buttons;
    }

    SentInputSnapshot build_snapshot(const WheelInputState &state)
    {
        SentInputSnapshot snapshot;
        snapshot.steering = steering_to_hid(state.angle);
        snapshot.throttle = scale_axis_to_bits(state.throttle, 16);
        snapshot.brake = scale_axis_to_bits(state.brake, 12);
        snapshot.clutch = scale_axis_to_bits(state.clutch, 12);
        snapshot.aux_axis = 0;
        snapshot.buttons = state.buttons;
        snapshot.valid = true;
        return snapshot;
    }

    void reset_usb_session_state()
    {
        g_usb_session_active = false;
        g_last_input_snapshot = SentInputSnapshot{};
        g_last_status_payload = PidStatusPayload{};
        g_last_input_report_ms = 0;
        g_last_status_report_ms = 0;
        g_last_block_load_report = {PID_BLOCK_LOAD_REPORT_ID, 0, BLOCK_LOAD_FULL, 0};
    }

    void send_pid_status_if_needed(uint32_t now_ms, bool force)
    {
        const PidStatusPayload payload = build_status_payload();
        if (!force &&
            payload.status == g_last_status_payload.status &&
            payload.effect_block_index == g_last_status_payload.effect_block_index &&
            (now_ms - g_last_status_report_ms) < STATUS_REPORT_INTERVAL_MS)
        {
            return;
        }

        HID_SendReport(PID_STATE_REPORT_ID, &payload, sizeof(payload));
        g_last_status_payload = payload;
        g_last_status_report_ms = now_ms;
    }

    void send_input_report_if_needed(uint32_t now_ms)
    {
        const SentInputSnapshot snapshot = build_snapshot(g_input_state);
        if (same_snapshot(snapshot, g_last_input_snapshot) &&
            (now_ms - g_last_input_report_ms) < INPUT_REPORT_INTERVAL_MS)
        {
            return;
        }

        Joystick.send_16_16_12_12_12_28(snapshot.steering,
                                        snapshot.throttle,
                                        snapshot.brake,
                                        snapshot.clutch,
                                        snapshot.aux_axis,
                                        snapshot.buttons);
        g_last_input_snapshot = snapshot;
        g_last_input_report_ms = now_ms;
    }

    void prepare_block_load_report(uint8_t effect_id, uint8_t load_status)
    {
        g_last_block_load_report.report_id = PID_BLOCK_LOAD_REPORT_ID;
        g_last_block_load_report.effect_block_index = effect_id;
        g_last_block_load_report.load_status = load_status;
        g_last_block_load_report.ram_pool_available = compute_ram_pool_available();
    }

    void stop_other_effects(uint8_t keep_effect_id)
    {
        for (uint8_t effect_id = 1; effect_id <= ffb_get_max_effect_slots(); ++effect_id)
        {
            if (effect_id != keep_effect_id)
            {
                ffb_stop_effect(effect_id);
            }
        }
    }

    void handle_device_control(uint8_t command, uint32_t now_ms)
    {
        if (command == DEVICE_CONTROL_ENABLE_ACTUATORS)
        {
            ffb_set_enabled(true, now_ms);
            return;
        }

        if (command == DEVICE_CONTROL_DISABLE_ACTUATORS)
        {
            ffb_set_enabled(false, now_ms);
            return;
        }

        if (command == DEVICE_CONTROL_STOP_ALL_EFFECTS)
        {
            ffb_clear_all_effects();
            return;
        }

        if (command == DEVICE_CONTROL_DEVICE_RESET || command == 0x04)
        {
            ffb_clear_all_effects();
            ffb_set_enabled(false, now_ms);
            return;
        }

        if (command == DEVICE_CONTROL_DEVICE_PAUSE || command == 0x05)
        {
            ffb_set_enabled(false, now_ms);
            return;
        }

        if (command == DEVICE_CONTROL_DEVICE_CONTINUE || command == 0x06)
        {
            ffb_set_enabled(true, now_ms);
            return;
        }
    }

    void handle_set_effect(const SetEffectOutputReport &report)
    {
        const FfbEffectType effect_type = decode_effect_type(report.effect_type);
        if (!is_valid_effect_id(report.effect_block_index) || effect_type == FfbEffectType::None)
        {
            return;
        }

        uint8_t gain_percent = gain16_to_percent(report.gain);
        FfbEffectSlot existing_slot;
        if (report.gain <= 0 && ffb_get_effect_slot(report.effect_block_index, existing_slot))
        {
            gain_percent = existing_slot.gain;
        }

        ffb_prepare_effect(report.effect_block_index, effect_type, gain_percent);
        ffb_update_effect_parameters(report.effect_block_index,
                                     gain_percent,
                                     report.duration,
                                     static_cast<int16_t>(report.direction),
                                     report.start_delay);
    }

    void handle_set_condition(const SetConditionOutputReport &report)
    {
        if (!is_valid_effect_id(report.effect_block_index))
        {
            return;
        }

        FfbEffectSlot slot;
        if (!ffb_get_effect_slot(report.effect_block_index, slot))
        {
            return;
        }

        const int16_t center = condition16_to_percent(report.cp_offset);
        const int16_t coefficient = condition16_to_percent(report.positive_coefficient);
        const int16_t deadband = static_cast<int16_t>(constrain(static_cast<int32_t>(report.dead_band) * 100L / 255L, 0L, 100L));

        switch (slot.type)
        {
        case FfbEffectType::Spring:
            ffb_set_spring(report.effect_block_index, center, deadband, coefficient, coefficient, slot.gain);
            break;
        case FfbEffectType::Damper:
            ffb_set_damper(report.effect_block_index, coefficient, coefficient, slot.gain);
            break;
        default:
            ffb_set_condition(report.effect_block_index, center, deadband, coefficient, coefficient, slot.gain);
            break;
        }
    }

    void handle_set_envelope(const SetEnvelopeOutputReport &report)
    {
        if (!is_valid_effect_id(report.effect_block_index))
        {
            return;
        }

        ffb_set_envelope(report.effect_block_index,
                         level8_to_percent(report.attack_level),
                         level8_to_percent(report.fade_level),
                         report.attack_time,
                         report.fade_time);
    }

    void handle_set_periodic(const SetPeriodicOutputReport &report)
    {
        if (!is_valid_effect_id(report.effect_block_index))
        {
            return;
        }

        FfbEffectSlot slot;
        uint8_t gain_percent = 100;
        if (ffb_get_effect_slot(report.effect_block_index, slot))
        {
            gain_percent = slot.gain;
        }

        ffb_set_periodic(report.effect_block_index,
                         signed16_to_percent_force(report.magnitude),
                         report.period,
                         static_cast<uint16_t>(report.phase) * 141U,
                         signed16_to_percent_force(report.offset),
                         gain_percent);
    }

    void handle_set_constant_force(const SetConstantForceOutputReport &report)
    {
        if (!is_valid_effect_id(report.effect_block_index))
        {
            return;
        }

        FfbEffectSlot slot;
        uint8_t gain_percent = 100;
        if (ffb_get_effect_slot(report.effect_block_index, slot))
        {
            gain_percent = slot.gain;
        }

        ffb_set_constant_force(report.effect_block_index,
                               signed16_to_percent_force(report.magnitude),
                               gain_percent);
    }

    void handle_set_ramp_force(const SetRampForceOutputReport &report)
    {
        if (!is_valid_effect_id(report.effect_block_index))
        {
            return;
        }

        FfbEffectSlot slot;
        uint8_t gain_percent = 100;
        if (ffb_get_effect_slot(report.effect_block_index, slot))
        {
            gain_percent = slot.gain;
        }

        ffb_set_ramp(report.effect_block_index, report.ramp_start, report.ramp_end, gain_percent);
    }

    void handle_effect_operation(const EffectOperationOutputReport &report, uint32_t now_ms)
    {
        if (!is_valid_effect_id(report.effect_block_index))
        {
            return;
        }

        switch (report.operation)
        {
        case EFFECT_OP_START:
            if (!ffb_is_enabled())
            {
                ffb_set_enabled(true, now_ms);
            }
            ffb_start_effect(report.effect_block_index, now_ms, report.loop_count == 0 ? 1 : report.loop_count);
            break;
        case EFFECT_OP_START_SOLO:
            if (!ffb_is_enabled())
            {
                ffb_set_enabled(true, now_ms);
            }
            stop_other_effects(report.effect_block_index);
            ffb_start_effect(report.effect_block_index, now_ms, report.loop_count == 0 ? 1 : report.loop_count);
            break;
        case EFFECT_OP_STOP:
            ffb_stop_effect(report.effect_block_index);
            break;
        default:
            break;
        }
    }

    void handle_output_report(const uint8_t *data, uint16_t len)
    {
        if (len == 0)
        {
            return;
        }

        const uint8_t report_id = data[0];
        ++g_runtime_status.output_report_count;
        g_runtime_status.last_report_id = report_id;
        ffb_note_host_activity(millis());

        switch (report_id)
        {
        case PID_SET_EFFECT_REPORT_ID:
        {
            const auto *report = as_report<SetEffectOutputReport>(data, len);
            if (!report)
            {
                return;
            }
            handle_set_effect(*report);
            break;
        }
        case PID_SET_ENVELOPE_REPORT_ID:
        {
            const auto *report = as_report<SetEnvelopeOutputReport>(data, len);
            if (!report)
            {
                return;
            }
            handle_set_envelope(*report);
            break;
        }
        case PID_SET_CONDITION_REPORT_ID:
        {
            const auto *report = as_report<SetConditionOutputReport>(data, len);
            if (!report)
            {
                return;
            }
            handle_set_condition(*report);
            break;
        }
        case PID_SET_PERIODIC_REPORT_ID:
        {
            const auto *report = as_report<SetPeriodicOutputReport>(data, len);
            if (!report)
            {
                return;
            }
            handle_set_periodic(*report);
            break;
        }
        case PID_SET_CONSTANT_FORCE_REPORT_ID:
        {
            const auto *report = as_report<SetConstantForceOutputReport>(data, len);
            if (!report)
            {
                return;
            }
            handle_set_constant_force(*report);
            break;
        }
        case PID_SET_RAMP_FORCE_REPORT_ID:
        {
            const auto *report = as_report<SetRampForceOutputReport>(data, len);
            if (!report)
            {
                return;
            }
            handle_set_ramp_force(*report);
            break;
        }
        case PID_EFFECT_OPERATION_REPORT_ID:
        {
            const auto *report = as_report<EffectOperationOutputReport>(data, len);
            if (!report)
            {
                return;
            }
            handle_effect_operation(*report, millis());
            break;
        }
        case PID_BLOCK_FREE_REPORT_ID:
        {
            const auto *report = as_report<BlockFreeOutputReport>(data, len);
            if (!report || !is_valid_effect_id(report->effect_block_index))
            {
                return;
            }
            ffb_clear_effect(report->effect_block_index);
            break;
        }
        case PID_DEVICE_CONTROL_REPORT_ID:
        {
            const auto *report = as_report<DeviceControlOutputReport>(data, len);
            if (!report)
            {
                return;
            }
            handle_device_control(report->control, millis());
            break;
        }
        case PID_DEVICE_GAIN_REPORT_ID:
        {
            const auto *report = as_report<DeviceGainOutputReport>(data, len);
            if (!report)
            {
                return;
            }
            ffb_set_device_gain(clamp_percent_from_u8(static_cast<uint8_t>((static_cast<uint16_t>(report->device_gain) * 100U) / 255U)), millis());
            break;
        }
        default:
            break;
        }
    }

    void send_feature_report(const void *data, uint16_t len)
    {
        USB_SendControl(TRANSFER_RELEASE, data, len);
    }

    bool handle_feature_get_report(uint8_t report_id)
    {
        ++g_runtime_status.feature_report_count;
        g_runtime_status.last_report_id = report_id;
        ffb_note_host_activity(millis());

        if (report_id == PID_BLOCK_LOAD_REPORT_ID)
        {
            send_feature_report(&g_last_block_load_report, sizeof(g_last_block_load_report));
            return true;
        }

        if (report_id == PID_POOL_REPORT_ID)
        {
            const PoolFeatureReport report = {
                PID_POOL_REPORT_ID,
                compute_ram_pool_size(),
                ffb_get_max_effect_slots(),
                0x03};
            send_feature_report(&report, sizeof(report));
            return true;
        }

        return false;
    }

    bool handle_feature_set_report(uint8_t report_id)
    {
        ++g_runtime_status.feature_report_count;
        g_runtime_status.last_report_id = report_id;
        ffb_note_host_activity(millis());

        if (report_id != PID_CREATE_NEW_EFFECT_REPORT_ID)
        {
            return false;
        }

        CreateNewEffectFeatureReport report{};
        USB_RecvControl(&report, sizeof(report));

        const FfbEffectType effect_type = decode_effect_type(report.effect_type);
        if (effect_type == FfbEffectType::None)
        {
            prepare_block_load_report(0, BLOCK_LOAD_ERROR);
            return true;
        }

        const uint8_t effect_id = find_free_effect_id();
        if (effect_id == 0)
        {
            prepare_block_load_report(0, BLOCK_LOAD_FULL);
            return true;
        }

        if (!ffb_prepare_effect(effect_id, effect_type, 100))
        {
            prepare_block_load_report(0, BLOCK_LOAD_ERROR);
            return true;
        }

        prepare_block_load_report(effect_id, BLOCK_LOAD_SUCCESS);
        return true;
    }

    bool usb_hid_setup(Setup &setup)
    {
        const uint8_t request = setup.bRequest;
        const uint8_t report_id = setup.wValueL;
        const uint8_t report_type = setup.wValueH;

        if (REQUEST_DEVICETOHOST_CLASS_INTERFACE == setup.bmRequestType)
        {
            if (request == HID_GET_REPORT && report_type == HID_REPORT_TYPE_FEATURE)
            {
                return handle_feature_get_report(report_id);
            }

            if (request == HID_GET_PROTOCOL)
            {
                return true;
            }
        }

        if (REQUEST_HOSTTODEVICE_CLASS_INTERFACE == setup.bmRequestType)
        {
            switch (request)
            {
            case HID_SET_PROTOCOL:
            case HID_SET_IDLE:
                return true;
            case HID_SET_REPORT:
                if (report_type == HID_REPORT_TYPE_FEATURE)
                {
                    return handle_feature_set_report(report_id);
                }
                return true;
            default:
                break;
            }
        }

        return HID_Setup(setup);
    }

    void usb_hid_receive_report(uint8_t *data, uint16_t len)
    {
        handle_output_report(data, len);
    }
}

void setup_usb_wheel()
{
    g_input_state = WheelInputState{};
    g_last_input_snapshot = SentInputSnapshot{};
    g_last_status_payload = PidStatusPayload{};
    g_last_block_load_report = {PID_BLOCK_LOAD_REPORT_ID, 0, BLOCK_LOAD_FULL, 0};
    g_runtime_status = UsbFfbRuntimeStatus{};

    USBDevice.HID_Setup_Callback = usb_hid_setup;
    USBDevice.HID_ReceiveReport_Callback = usb_hid_receive_report;
}

void update_usb_wheel()
{
    const bool ready = usb_wheel_ready();
    const uint32_t now_ms = millis();

    if (!ready)
    {
        if (g_usb_session_active)
        {
            reset_usb_session_state();
            ffb_set_host_connection(false, now_ms);
        }
        return;
    }

    if (!g_usb_session_active)
    {
        g_usb_session_active = true;
        ffb_set_host_connection(true, now_ms);
        send_pid_status_if_needed(now_ms, true);
    }

    send_input_report_if_needed(now_ms);
    send_pid_status_if_needed(now_ms, false);
}

bool usb_wheel_ready()
{
    return USBDevice.configured();
}

void usb_wheel_set_input_state(const WheelInputState &state)
{
    g_input_state = state;
}

const UsbFfbRuntimeStatus &usb_wheel_get_runtime_status()
{
    return g_runtime_status;
}

#else

void setup_usb_wheel()
{
}

void update_usb_wheel()
{
}

bool usb_wheel_ready()
{
    return false;
}

void usb_wheel_set_input_state(const WheelInputState &)
{
}

const UsbFfbRuntimeStatus &usb_wheel_get_runtime_status()
{
    static UsbFfbRuntimeStatus status;
    return status;
}

#endif
