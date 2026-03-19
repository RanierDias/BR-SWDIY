#include "proto/serial_protocol.h"

#include <Arduino.h>
#include <string.h>
#include <stdlib.h>

#include "config/config_model.h"
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
      Serial.print("INFO DEVICE=BRSWDIY FW=");
      Serial.print(BRSWDIY_FW_VERSION);
      Serial.print(" PROTO=");
      Serial.print(BRSWDIY_PROTO_VERSION);
      Serial.print(" HW=");
      Serial.println(BRSWDIY_HW_VERSION);
      return;
    }

    if (strcmp(raw, "GET_STATE") == 0)
    {
      const DeviceStatus &status = get_status();

      Serial.print("STATE NAME=");
      Serial.print(state_name(status.state));
      Serial.print(" MOTOR=");
      Serial.print(status.motor_enabled ? 1 : 0);
      Serial.print(" FAULT=");
      Serial.print(status.fault ? 1 : 0);
      Serial.print(" CALIBRATED=");
      Serial.print(status.calibrated ? 1 : 0);
      Serial.print(" CONFIG_SAVED=");
      Serial.println(status.config_saved ? 1 : 0);
      return;
    }

    if (strcmp(raw, "GET_STATUS") == 0)
    {
      const DeviceStatus &status = get_status();

      Serial.print("STATUS STATE=");
      Serial.print(state_name(status.state));
      Serial.print(" ANGLE=");
      Serial.print(status.angle);
      Serial.print(" OUTPUT=");
      Serial.print(status.output);
      Serial.print(" LIMIT=");
      Serial.print(status.limit);
      Serial.print(" THROTTLE=");
      Serial.print(status.throttle);
      Serial.print(" BRAKE=");
      Serial.print(status.brake);
      Serial.print(" CLUTCH=");
      Serial.print(status.clutch);
      Serial.print(" FAULT=");
      Serial.print(status.fault ? 1 : 0);
      Serial.print(" CALIBRATED=");
      Serial.println(status.calibrated ? 1 : 0);
      return;
    }

    if (strcmp(raw, "GET_CONFIG") == 0)
    {
      const DeviceConfig &config = get_config();

      Serial.print("CONFIG GAIN=");
      Serial.print(config.gain);
      Serial.print(" OUTPUT_LIMIT=");
      Serial.print(config.output_limit);
      Serial.print(" SAFE_START=");
      Serial.print(config.safe_start ? 1 : 0);
      Serial.print(" WATCHDOG_MS=");
      Serial.println(config.watchdog_ms);
      return;
    }

    if (strncmp(raw, "SET_GAIN ", 9) == 0)
    {
      int value = atoi(raw + 9);

      if (set_gain(value))
      {
        Serial.print("OK GAIN=");
        Serial.println(get_config().gain);
      }
      else
      {
        write_error(3, "INVALID_RANGE", "GAIN");
      }

      return;
    }

    if (strncmp(raw, "SET_OUTPUT_LIMIT ", 17) == 0)
    {
      int value = atoi(raw + 17);

      if (set_output_limit(value))
      {
        Serial.print("OK OUTPUT_LIMIT=");
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
        Serial.print("OK SAFE_START=");
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
        Serial.print("OK WATCHDOG_MS=");
        Serial.println(get_config().watchdog_ms);
      }
      else
      {
        write_error(3, "INVALID_RANGE", "WATCHDOG_MS");
      }

      return;
    }

    if (strcmp(raw, "SAVE_CONFIG") == 0)
    {
      Serial.println("SAVED=1");
      return;
    }

    if (strcmp(raw, "LOAD_CONFIG") == 0)
    {
      Serial.println("LOADED=1");
      return;
    }

    if (strcmp(raw, "RESET_ANGLE") == 0)
    {
      reset_angle();

      Serial.println("OK RESET_ANGLE");
      return;
    }

    if (strcmp(raw, "MOTOR_START") == 0)
    {
      handle_motor(true);

      Serial.println("OK MOTOR=1");
      return;
    }

    if (strcmp(raw, "MOTOR_STOP") == 0)
    {
      handle_motor(false);

      Serial.println("OK MOTOR=0");
      return;
    }

    if (strcmp(raw, "GET_ERRORS") == 0)
    {
      Serial.println("ERRORS COUNT=0");
      return;
    }

    if (strcmp(raw, "CLEAR_ERRORS") == 0)
    {
      Serial.println("CLEARED=1");
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
