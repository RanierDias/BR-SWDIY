#include "app/app.h"
#include <Arduino.h>

#include "constants/protocol_constants.h"
#include "control/control_loop.h"
#include "ffb/ffb_effects.h"
#include "ffb/ffb_mixer.h"
#include "proto/serial_protocol.h"
#include "safety/safety_manager.h"
#include "types/device_state.h"
#include "types/telemetry_types.h"
#include "types/control_types.h"
#include "config/config_model.h"
#include "config/calibration_model.h"
#include "hw/pedals.h"
#include "hw/encoder.h"
#include "hw/motor.h"
#include "hw/eeprom_store.h"
#include "usb/usb_wheel.h"

static DeviceConfig g_config;
static DeviceStatus g_status;
static InputCalibration g_input_calibration;
static int16_t g_encoder_zero_offset = 0;
static uint32_t g_last_slow_update = 0;
static uint16_t g_max_angle_counts = 0;
static WheelInputState g_input_state;

static constexpr int8_t LOCK_ZONE = 20;
static constexpr int8_t LOCK_MIN_FORCE = 12;
static constexpr int8_t LOCK_MAX_FORCE = 100;

static void apply_default_config()
{
  g_config.max_angle = 720;
  g_max_angle_counts = 2400;
  g_config.gain = brswdiy::protocol::GAIN_DEFAULT;
  g_config.damper = 6;
  g_config.friction = 2;
  g_config.inertia = 2;
  g_config.spring = 0;
  g_config.output_limit = brswdiy::protocol::OUTPUT_LIMIT_DEFAULT;
  g_config.safe_start = true;
  g_config.watchdog_ms = brswdiy::protocol::WATCHDOG_DEFAULT_MS;

  g_input_calibration.throttle.min_raw = 0;
  g_input_calibration.throttle.max_raw = 1023;
  g_input_calibration.brake.min_raw = 0;
  g_input_calibration.brake.max_raw = 1023;
  g_input_calibration.clutch.min_raw = 0;
  g_input_calibration.clutch.max_raw = 1023;
  g_input_calibration.invert_pedals = false;
}

static PersistedConfig build_persisted_config()
{
  PersistedConfig settings;

  settings.max_angle = g_config.max_angle;
  settings.gain = g_config.gain;
  settings.damper = g_config.damper;
  settings.friction = g_config.friction;
  settings.inertia = g_config.inertia;
  settings.spring = g_config.spring;
  settings.output_limit = g_config.output_limit;
  settings.safe_start = g_config.safe_start;
  settings.watchdog_ms = g_config.watchdog_ms;

  settings.throttle_min = g_input_calibration.throttle.min_raw;
  settings.throttle_max = g_input_calibration.throttle.max_raw;
  settings.brake_min = g_input_calibration.brake.min_raw;
  settings.brake_max = g_input_calibration.brake.max_raw;
  settings.clutch_min = g_input_calibration.clutch.min_raw;
  settings.clutch_max = g_input_calibration.clutch.max_raw;
  settings.invert_pedals = g_input_calibration.invert_pedals;

  return settings;
}

static void apply_persisted_config(const PersistedConfig &settings)
{
  g_config.max_angle = settings.max_angle;
  g_max_angle_counts = settings.max_angle * 10 / 3;
  g_config.gain = settings.gain;
  g_config.damper = settings.damper;
  g_config.friction = settings.friction;
  g_config.inertia = settings.inertia;
  g_config.spring = settings.spring;
  g_config.output_limit = settings.output_limit;
  g_config.safe_start = settings.safe_start;
  g_config.watchdog_ms = settings.watchdog_ms;

  g_input_calibration.throttle.min_raw = settings.throttle_min;
  g_input_calibration.throttle.max_raw = settings.throttle_max;
  g_input_calibration.brake.min_raw = settings.brake_min;
  g_input_calibration.brake.max_raw = settings.brake_max;
  g_input_calibration.clutch.min_raw = settings.clutch_min;
  g_input_calibration.clutch.max_raw = settings.clutch_max;
  g_input_calibration.invert_pedals = settings.invert_pedals;
}

static inline uint16_t normalize_pedal(uint16_t raw, const PedalCalibration &input)
{
  if (raw >= input.max_raw)
    return g_input_calibration.invert_pedals ? 0 : 1023;
  if (raw <= input.min_raw)
    return g_input_calibration.invert_pedals ? 1023 : 0;

  uint16_t range = input.max_raw - input.min_raw;
  uint16_t value = (uint16_t)((uint32_t)(raw - input.min_raw) * 1023 / range);

  if (g_input_calibration.invert_pedals)
  {
    return 1023 - value;
  }

  return value;
}

static void update_pedals()
{
  g_status.throttle = normalize_pedal(read_throttle_raw(), g_input_calibration.throttle);
  g_status.brake = normalize_pedal(read_brake_raw(), g_input_calibration.brake);
  g_status.clutch = normalize_pedal(read_clutch_raw(), g_input_calibration.clutch);
}

static void update_encoder()
{
  g_status.angle = get_encoder_position() - g_encoder_zero_offset;
}

static int8_t apply_angle_limit(int8_t requested_output)
{
  const int16_t current_angle = g_status.angle;
  const uint16_t threshold = (g_max_angle_counts - LOCK_ZONE) / 2;
  const uint16_t abs_angle = (current_angle < 0) ? -current_angle : current_angle;

  if (abs_angle < threshold)
  {
    return requested_output;
  }

  const int16_t distance = abs_angle - threshold;
  const int16_t lock_range = g_max_angle_counts > threshold ? (g_max_angle_counts - threshold) : 1;
  int16_t scaled_strength = LOCK_MIN_FORCE +
                            ((static_cast<int32_t>(distance) * (LOCK_MAX_FORCE - LOCK_MIN_FORCE)) / lock_range);

  if (scaled_strength > LOCK_MAX_FORCE)
  {
    scaled_strength = LOCK_MAX_FORCE;
  }

  const int8_t lock_force = (current_angle > 0)
                                ? static_cast<int8_t>(scaled_strength)
                                : static_cast<int8_t>(-scaled_strength);

  // Preserve game FFB while preventing any torque that keeps pushing outward.
  if ((requested_output > 0 && lock_force > 0) || (requested_output < 0 && lock_force < 0))
  {
    const int16_t combined = static_cast<int16_t>(requested_output) + lock_force / 2;
    return constrain(combined, -g_config.output_limit, g_config.output_limit);
  }

  const int16_t corrected = static_cast<int16_t>(requested_output) + lock_force;
  if ((corrected > 0 && lock_force < 0) || (corrected < 0 && lock_force > 0))
  {
    return 0;
  }

  return constrain(corrected, -g_config.output_limit, g_config.output_limit);
}

static void refresh_control_state(uint32_t now_us)
{
  g_input_state = control_build_input_state(
      g_status.angle,
      g_status.throttle,
      g_status.brake,
      g_status.clutch,
      g_status.buttons,
      now_us);
  usb_wheel_set_input_state(g_input_state);
}

static void update_motor_output()
{
  const uint32_t now_ms = millis();
  MotorCommand command = ffb_mix_motor_command(g_input_state, g_config);
  command = safety_apply_motor_rules(
      command,
      g_config,
      ffb_get_device_state(),
      g_status.motor_enabled,
      now_ms);
  command.output_percent = apply_angle_limit(command.output_percent);
  command.output_percent = constrain(command.output_percent, -g_config.output_limit, g_config.output_limit);

  g_status.output = command.output_percent;
  set_motor_output(command.output_percent);
}

void setup_app()
{
  Serial.begin(brswdiy::protocol::SERIAL_BAUDRATE);
  setup_pedals();
  setup_encoder();
  setup_motor();
  setup_control_loop();
  setup_ffb_effects();
  setup_safety_manager();
  setup_usb_wheel();

  g_status.state = DeviceState::BOOT;
  apply_default_config();

  g_status.motor_enabled = true;
  g_status.limit = g_config.output_limit;
  g_status.config_saved = load_config();

  g_encoder_zero_offset = get_encoder_position();
  g_status.state = DeviceState::READY;
}

void update_app()
{
  const uint32_t now = micros();

  update_encoder();
  update_pedals();
  refresh_control_state(now);
  update_motor_output();
  update_usb_wheel();

  if ((now - g_last_slow_update) >= 5000)
  {
    g_last_slow_update = now;
    process_serial_protocol();
  }
}

const DeviceConfig &get_config()
{
  return g_config;
}

const DeviceStatus &get_status()
{
  return g_status;
}

const InputCalibration &get_pedal_calibration()
{
  return g_input_calibration;
}

uint16_t get_max_angle()
{
  return g_config.max_angle;
}

bool save_config()
{
  const PersistedConfig settings = build_persisted_config();
  return save_config_to_eeprom(settings);
}

bool load_config()
{
  PersistedConfig settings;

  if (!load_config_from_eeprom(settings))
  {
    return false;
  }

  apply_persisted_config(settings);
  return true;
}

void reset_config()
{
  apply_default_config();
  g_status.limit = g_config.output_limit;
}

bool set_max_angle(int value)
{
  if (value < 180 || value > 2048)
    return false;

  g_config.max_angle = value;

  g_max_angle_counts = (uint16_t)((uint32_t)value * 10 / 3);
  return true;
}

bool set_gain(int gain)
{
  if (gain < 0 || gain > 100)
  {
    return false;
  }

  g_config.gain = gain;
  return true;
}

bool set_damper(int value)
{
  if (value < 0 || value > 100)
  {
    return false;
  }

  g_config.damper = static_cast<uint8_t>(value);
  return true;
}

bool set_friction(int value)
{
  if (value < 0 || value > 100)
  {
    return false;
  }

  g_config.friction = static_cast<uint8_t>(value);
  return true;
}

bool set_inertia(int value)
{
  if (value < 0 || value > 100)
  {
    return false;
  }

  g_config.inertia = static_cast<uint8_t>(value);
  return true;
}

bool set_spring(int value)
{
  if (value < 0 || value > 100)
  {
    return false;
  }

  g_config.spring = static_cast<uint8_t>(value);
  return true;
}

bool set_output_limit(int limit)
{
  if (limit < 0 || limit > 100)
  {
    return false;
  }

  g_config.output_limit = limit;
  g_status.limit = limit;
  return true;
}

bool set_safe_start(bool enable)
{
  g_config.safe_start = enable;
  return true;
}

bool set_watchdog(int watchdog_ms)
{
  if (watchdog_ms < 50 || watchdog_ms > 5000)
  {
    return false;
  }

  g_config.watchdog_ms = watchdog_ms;
  return true;
}

bool set_throttle_max(int value)
{
  if (value < 0 || value > 1023)
  {
    return false;
  }

  const uint16_t v = value;

  if (v <= g_input_calibration.throttle.min_raw)
  {
    return false;
  }

  g_input_calibration.throttle.max_raw = v;
  return true;
}

bool set_throttle_min(int value)
{
  if (value < 0 || value > 1023)
  {
    return false;
  }

  const uint16_t v = value;

  if (v >= g_input_calibration.throttle.max_raw)
  {
    return false;
  }

  g_input_calibration.throttle.min_raw = v;
  return true;
}

bool set_brake_max(int value)
{
  if (value < 0 || value > 1023)
  {
    return false;
  }

  const uint16_t v = value;

  if (v <= g_input_calibration.brake.min_raw)
  {
    return false;
  }

  g_input_calibration.brake.max_raw = v;
  return true;
}

bool set_brake_min(int value)
{
  if (value < 0 || value > 1023)
  {
    return false;
  }

  const uint16_t v = value;

  if (v >= g_input_calibration.brake.max_raw)
  {
    return false;
  }

  g_input_calibration.brake.min_raw = v;
  return true;
}

bool set_clutch_max(int value)
{
  if (value < 0 || value > 1023)
  {
    return false;
  }

  const uint16_t v = value;

  if (v <= g_input_calibration.clutch.min_raw)
  {
    return false;
  }

  g_input_calibration.clutch.max_raw = v;
  return true;
}

bool set_clutch_min(int value)
{
  if (value < 0 || value > 1023)
  {
    return false;
  }

  const uint16_t v = value;

  if (v >= g_input_calibration.clutch.max_raw)
  {
    return false;
  }

  g_input_calibration.clutch.min_raw = v;
  return true;
}

bool set_pedal_invert(bool enable)
{
  g_input_calibration.invert_pedals = enable;
  return true;
}

bool handle_motor(bool enable)
{
  g_status.motor_enabled = enable;
  ffb_set_enabled(enable, millis());

  if (!enable)
  {
    g_status.output = 0;
    set_motor_output(0);
  }

  return true;
}

bool recenter_encoder()
{
  g_encoder_zero_offset = get_encoder_position();
  g_status.angle = 0;
  return true;
}
