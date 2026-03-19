#include "app/app.h"

#include <Arduino.h>

#include "constants/protocol_constants.h"
#include "proto/serial_protocol.h"

#include "types/device_state.h"
#include "types/telemetry_types.h"
#include "config/config_model.h"

#include "hw/pedals.h"
#include "hw/encoder.h"

static DeviceConfig g_config;
static DeviceStatus g_status;
static int16_t g_encoder_zero_offset;

static void update_pedals()
{
  uint16_t throttle_raw = read_throttle_raw();
  uint16_t brake_raw = read_brake_raw();
  uint16_t clutch_raw = read_clutch_raw();

  g_status.throttle = static_cast<uint8_t>(map(throttle_raw, 0, 1023, 0, 100));
  g_status.brake = static_cast<uint8_t>(map(brake_raw, 0, 1023, 0, 100));
  g_status.clutch = static_cast<uint8_t>(map(clutch_raw, 0, 1023, 0, 100));
}

static void update_encoder()
{
  int16_t raw_position = get_encoder_position();
  g_status.angle = raw_position - g_encoder_zero_offset;
}

void setup_app()
{
  Serial.begin(brswdiy::protocol::SERIAL_BAUDRATE);
  setup_pedals();
  setup_encoder();

  g_status.state = DeviceState::BOOT;

  g_config.gain = brswdiy::protocol::GAIN_DEFAULT;
  g_config.output_limit = brswdiy::protocol::OUTPUT_LIMIT_DEFAULT;
  g_config.safe_start = true;
  g_config.watchdog_ms = brswdiy::protocol::WATCHDOG_DEFAULT_MS;
  g_config.dir_invert = false;
  g_config.encoder_invert = false;

  g_encoder_zero_offset = get_encoder_position();
  g_status.angle = 0;
  g_status.center = 0;
  g_status.output = 0;
  g_status.limit = g_config.output_limit;
  g_status.throttle = 0;
  g_status.brake = 0;
  g_status.clutch = 0;
  g_status.buttons = 0;
  g_status.fault = false;
  g_status.motor_enabled = false;
  g_status.calibrated = false;
  g_status.config_saved = false;

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

void handle_motor(bool enable)
{
  g_status.motor_enabled = enable;
}

void recenter_encoder()
{
  g_encoder_zero_offset = get_encoder_position();
  g_status.angle = 0;
}
