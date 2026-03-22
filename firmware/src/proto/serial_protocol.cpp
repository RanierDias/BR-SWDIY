#include "proto/serial_protocol.h"

#include <Arduino.h>
#include <string.h>
#include <stdlib.h>

#include "config/config_model.h"
#include "config/calibration_model.h"
#include "types/device_state.h"
#include "types/telemetry_types.h"
#include "app/app.h"

namespace
{
  constexpr uint8_t LINE_BUFFER_SIZE = 64;
  char g_line[LINE_BUFFER_SIZE];
  uint8_t g_line_len = 0;

  void write_error(int code, const char *name, const char *field = nullptr)
  {
    Serial.print("ERR CODE=");
    Serial.print(code);
    Serial.print(" NAME=");
    Serial.print(name);
    Serial.print(" FIELD=");
    Serial.println(field ? field : "N/A");
  }

  void handle_command(const char *raw)
  {
    if (strcmp(raw, "GET_INFO") == 0)
    {
      Serial.print(F("INFO DEVICE=BRSWDIY FW="));
      Serial.print(BRSWDIY_FW_VERSION);
      Serial.print(F(" PROTO="));
      Serial.print(BRSWDIY_PROTO_VERSION);
      Serial.print(F(" HW="));
      Serial.println(BRSWDIY_HW_VERSION);
      return;
    }

    if (strcmp(raw, "GET_STATE") == 0)
    {
      const DeviceStatus &status = get_status();

      Serial.print(F("STATE NAME="));
      Serial.print(state_name(status.state));
      Serial.print(F(" MOTOR="));
      Serial.print(status.motor_enabled ? 1 : 0);
      Serial.print(F(" FAULT="));
      Serial.print(status.fault ? 1 : 0);
      Serial.print(F(" CALIBRATED="));
      Serial.print(status.calibrated ? 1 : 0);
      Serial.print(F(" CONFIG_SAVED="));
      Serial.println(status.config_saved ? 1 : 0);
      return;
    }

    if (strcmp(raw, "GET_STATUS") == 0)
    {
      const DeviceStatus &status = get_status();

      Serial.print(F("STATUS STATE="));
      Serial.print(state_name(status.state));
      Serial.print(F(" ANGLE="));
      Serial.print(status.angle);
      Serial.print(F(" OUTPUT="));
      Serial.print(status.output);
      Serial.print(F(" LIMIT="));
      Serial.print(status.limit);
      Serial.print(F(" THROTTLE="));
      Serial.print(status.throttle);
      Serial.print(F(" BRAKE="));
      Serial.print(status.brake);
      Serial.print(F(" CLUTCH="));
      Serial.print(status.clutch);
      Serial.print(F(" FAULT="));
      Serial.print(status.fault ? 1 : 0);
      Serial.print(F(" CALIBRATED="));
      Serial.println(status.calibrated ? 1 : 0);
      return;
    }

    if (strcmp(raw, "GET_CONFIG") == 0)
    {
      const DeviceConfig &config = get_config();

      Serial.print(F("CONFIG GAIN="));
      Serial.print(config.gain);
      Serial.print(F(" OUTPUT_LIMIT="));
      Serial.print(config.output_limit);
      Serial.print(F(" SAFE_START="));
      Serial.print(config.safe_start ? 1 : 0);
      Serial.print(F(" WATCHDOG_MS="));
      Serial.println(config.watchdog_ms);
      return;
    }

    if (strcmp(raw, "GET_CALIBRATION") == 0)
    {
      const InputCalibration &input = get_pedal_calibration();

      Serial.print(F("CALIBRATION THROTTLE_MIN="));
      Serial.print(input.throttle.min_raw);
      Serial.print(F(" THROTTLE_MAX="));
      Serial.print(input.throttle.max_raw);
      Serial.print(F(" BRAKE_MIN="));
      Serial.print(input.brake.min_raw);
      Serial.print(F(" BRAKE_MAX="));
      Serial.print(input.brake.max_raw);
      Serial.print(F(" CLUTCH_MIN="));
      Serial.print(input.clutch.min_raw);
      Serial.print(F(" CLUTCH_MAX="));
      Serial.print(input.clutch.max_raw);
      Serial.print(F(" INVERT="));
      Serial.println(input.invert_pedals ? 1 : 0);
      return;
    }

    if (strncmp(raw, "SET_GAIN ", 9) == 0)
    {
      int value = atoi(raw + 9);

      if (set_gain(value))
      {
        Serial.print(F("OK GAIN="));
        Serial.println(get_config().gain);
      }
      else
      {
        write_error(3, "INVALID_RANGE", "GAIN");
      }

      return;
    }

    if (strncmp(raw, "SET_OUTPUT ", 11) == 0)
    {
      int value = atoi(raw + 11);

      if (set_motor_output_command(value))
      {
        Serial.print(F("OK OUTPUT="));
        Serial.println(value);
      }
      else
      {
        write_error(3, "INVALID_RANGE", "OUTPUT");
      }

      return;
    }

    if (strncmp(raw, "SET_OUTPUT_LIMIT ", 17) == 0)
    {
      int value = atoi(raw + 17);

      if (set_output_limit(value))
      {
        Serial.print(F("OK OUTPUT_LIMIT="));
        Serial.println(get_config().output_limit);
      }
      else
      {
        write_error(3, "INVALID_RANGE", "OUTPUT_LIMIT");
      }

      return;
    }

    if (strncmp(raw, "SET_SAFE_START ", 15) == 0)
    {
      int value = atoi(raw + 15);

      if (value != 0 && value != 1)
      {
        write_error(4, "INVALID_STATE", "SAFE_START");
        return;
      }

      if (set_safe_start(value != 0))
      {
        Serial.print(F("OK SAFE_START="));
        Serial.println(get_config().safe_start ? 1 : 0);
      }
      else
      {
        write_error(4, "INVALID_STATE", "SAFE_START");
      }

      return;
    }

    if (strncmp(raw, "SET_WATCHDOG ", 13) == 0)
    {
      int value = atoi(raw + 13);

      if (set_watchdog(value))
      {
        Serial.print(F("OK WATCHDOG_MS="));
        Serial.println(get_config().watchdog_ms);
      }
      else
      {
        write_error(3, "INVALID_RANGE", "WATCHDOG_MS");
      }

      return;
    }

    if (strncmp(raw, "SET_THROTTLE_MAX ", 17) == 0)
    {
      int value = atoi(raw + 17);

      if (set_throttle_max(value))
      {
        Serial.print(F("OK THROTTLE_MAX="));
        Serial.println(get_pedal_calibration().throttle.max_raw);
      }
      else
      {
        write_error(3, "INVALID_RANGE", "THROTTLE_MAX");
      }

      return;
    }

    if (strncmp(raw, "SET_THROTTLE_MIN ", 17) == 0)
    {
      int value = atoi(raw + 17);

      if (set_throttle_min(value))
      {
        Serial.print(F("OK THROTTLE_MIN="));
        Serial.println(get_pedal_calibration().throttle.min_raw);
      }
      else
      {
        write_error(3, "INVALID_RANGE", "THROTTLE_MIN");
      }

      return;
    }

    if (strncmp(raw, "SET_BRAKE_MAX ", 14) == 0)
    {
      int value = atoi(raw + 14);

      if (set_brake_max(value))
      {
        Serial.print(F("OK BRAKE_MAX="));
        Serial.println(get_pedal_calibration().brake.max_raw);
      }
      else
      {
        write_error(3, "INVALID_RANGE", "BRAKE_MAX");
      }

      return;
    }

    if (strncmp(raw, "SET_BRAKE_MIN ", 14) == 0)
    {
      int value = atoi(raw + 14);

      if (set_brake_min(value))
      {
        Serial.print(F("OK BRAKE_MIN="));
        Serial.println(get_pedal_calibration().brake.min_raw);
      }
      else
      {
        write_error(3, "INVALID_RANGE", "BRAKE_MIN");
      }

      return;
    }

    if (strncmp(raw, "SET_CLUTCH_MAX ", 15) == 0)
    {
      int value = atoi(raw + 15);

      if (set_clutch_max(value))
      {
        Serial.print(F("OK CLUTCH_MAX="));
        Serial.println(get_pedal_calibration().clutch.max_raw);
      }
      else
      {
        write_error(3, "INVALID_RANGE", "CLUTCH_MAX");
      }

      return;
    }

    if (strncmp(raw, "SET_CLUTCH_MIN ", 15) == 0)
    {
      int value = atoi(raw + 15);

      if (set_clutch_min(value))
      {
        Serial.print(F("OK CLUTCH_MIN="));
        Serial.println(get_pedal_calibration().clutch.min_raw);
      }
      else
      {
        write_error(3, "INVALID_RANGE", "CLUTCH_MIN");
      }

      return;
    }

    if (strncmp(raw, "SET_PEDAL_INVERT ", 17) == 0)
    {
      int value = atoi(raw + 17);

      if (value != 0 && value != 1)
      {
        write_error(3, "INVALID_RANGE", "PEDAL_INVERT");
        return;
      }

      if (set_pedal_invert(value != 0))
      {
        Serial.print(F("OK PEDAL_INVERT="));
        Serial.println(get_pedal_calibration().invert_pedals ? 1 : 0);
      }
      else
      {
        write_error(4, "INVALID_STATE", "PEDAL_INVERT");
      }

      return;
    }

    if (strcmp(raw, "SAVE_CONFIG") == 0)
    {
      if (save_config())
      {
        Serial.println(F("OK SAVED=1"));
      }
      else
      {
        write_error(7, "EEPROM_ERROR", "SAVE_CONFIG");
      }

      return;
    }

    if (strcmp(raw, "LOAD_CONFIG") == 0)
    {
      if (load_config())
      {
        Serial.println(F("OK LOADED=1"));
      }
      else
      {
        write_error(7, "EEPROM_ERROR", "LOAD_CONFIG");
      }

      return;
    }

    if (strcmp(raw, "RESET_CONFIG") == 0)
    {
      reset_config();

      Serial.println(F("OK RESET=1"));
      return;
    }

    if (strcmp(raw, "RECENTER") == 0)
    {
      recenter_encoder();

      Serial.println(F("OK RECENTERED"));
      return;
    }

    if (strcmp(raw, "MOTOR_START") == 0)
    {
      handle_motor(true);

      Serial.println(F("OK MOTOR=1"));
      return;
    }

    if (strcmp(raw, "MOTOR_STOP") == 0)
    {
      handle_motor(false);
      set_motor_output_command(0);

      Serial.println(F("OK MOTOR=0"));
      return;
    }

    if (strcmp(raw, "GET_ERRORS") == 0)
    {
      Serial.println(F("ERRORS COUNT=0"));
      return;
    }

    if (strcmp(raw, "CLEAR_ERRORS") == 0)
    {
      Serial.println(F("CLEARED=1"));
      return;
    }

    write_error(1, "UNKNOWN_COMMAND");
  }
}

void process_serial_protocol()
{
  while (Serial.available() > 0)
  {
    char c = static_cast<char>(Serial.read());

    if (c == '\r')
    {
      continue;
    }

    if (c == '\n')
    {
      if (g_line_len > 0)
      {
        handle_command(g_line);

        g_line_len = 0;
        g_line[0] = '\0';
      }
    }
    else
    {
      if (g_line_len < LINE_BUFFER_SIZE - 1)
      {
        g_line[g_line_len++] = c;
        g_line[g_line_len] = '\0';
      }
      else
      {
        g_line_len = 0;
        g_line[0] = '\0';

        write_error(2, "INVALID_ARGUMENT", "LINE_TOO_LONG");
      }
    }
  }
}
