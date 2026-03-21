#include "app/app.h"

#include <Arduino.h>

#include "constants/protocol_constants.h"
#include "proto/serial_protocol.h"

#include "types/device_state.h"
#include "types/telemetry_types.h"
#include "config/config_model.h"
#include "config/calibration_model.h"

#include "hw/pedals.h"
#include "hw/encoder.h"
#include "hw/eeprom_store.h"

static DeviceConfig g_config;
static DeviceStatus g_status;
static int16_t g_encoder_zero_offset;
static InputCalibration g_input_calibration;

static void apply_default_config()
{
  g_config.gain = brswdiy::protocol::GAIN_DEFAULT;
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

  settings.gain = g_config.gain;
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
  g_config.gain = settings.gain;
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

static uint8_t normalize_pedal(uint16_t raw, const PedalCalibration &input)
{
  if (input.max_raw <= input.min_raw)
  {
    return 0;
  }

  uint16_t value = raw;

  if (value < input.min_raw)
    value = input.min_raw;
  if (value > input.max_raw)
    value = input.max_raw;

  uint16_t range = input.max_raw - input.min_raw;
  uint16_t difference = value - input.min_raw;

  uint8_t result = (uint32_t(difference) * 100) / range;

  if (g_input_calibration.invert_pedals)
  {
    result = 100 - result;
  }

  return result;
}

static void update_pedals()
{
  uint16_t throttle_raw = read_throttle_raw();
  uint16_t brake_raw = read_brake_raw();
  uint16_t clutch_raw = read_clutch_raw();

  g_status.throttle = normalize_pedal(throttle_raw, g_input_calibration.throttle);
  g_status.brake = normalize_pedal(brake_raw, g_input_calibration.brake);
  g_status.clutch = normalize_pedal(clutch_raw, g_input_calibration.clutch);
}

static void update_encoder()
{
  const int16_t raw_position = get_encoder_position();
  g_status.angle = raw_position - g_encoder_zero_offset;
}

void setup_app()
{
  Serial.begin(brswdiy::protocol::SERIAL_BAUDRATE);
  setup_pedals();
  setup_encoder();

  g_status.state = DeviceState::BOOT;
  apply_default_config();
  g_status.config_saved = load_config();

  g_encoder_zero_offset = get_encoder_position();
  g_status.state = DeviceState::READY;
}

void update_app()
{
  update_pedals();
  update_encoder();
  process_serial_protocol();
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

bool set_gain(int gain)
{
  if (gain < 0 || gain > 100)
  {
    return false;
  }

  g_config.gain = gain;
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
  return true;
}

bool recenter_encoder()
{
  g_encoder_zero_offset = get_encoder_position();
  g_status.angle = 0;
  return true;
}
