#include "proto/serial_protocol.h"

#include <Arduino.h>

#include "app/app.h"
#include "ffb/ffb_effects.h"
#include "usb/usb_wheel.h"

namespace
{
  constexpr uint8_t LINE_BUFFER_SIZE = 80;
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
    if (strcmp(raw, "TEL") == 0)
    {
      const DeviceStatus &status = get_status();

      Serial.print(F("T "));
      Serial.print(status.angle);
      Serial.print(" ");
      Serial.print(status.throttle);
      Serial.print(" ");
      Serial.print(status.brake);
      Serial.print(" ");
      Serial.println(status.clutch);
      return;
    }

    if (strcmp(raw, "RCE") == 0)
    {
      if (recenter_encoder())
      {
        Serial.println(F("OK RCE=1"));
      }
      else
      {
        write_error(6, "NOT_CALIBRATED", "RCE");
      }

      return;
    }

    if (strcmp(raw, "CAL") == 0)
    {
      const InputCalibration &calibration = get_pedal_calibration();
      const DeviceConfig &config = get_config();

      Serial.print(F("C A="));
      Serial.print(get_max_angle());

      Serial.print(F(" I="));
      Serial.print(calibration.invert_pedals ? 1 : 0);

      Serial.print(F(" E="));
      Serial.print(config.encoder_ppr);

      Serial.print(F(" O="));
      Serial.print(config.output_limit);

      Serial.print(F(" G="));
      Serial.print(config.gain);

      Serial.print(F(" D="));
      Serial.print(config.damper);

      Serial.print(F(" F="));
      Serial.print(config.friction);

      Serial.print(F(" N="));
      Serial.print(config.inertia);

      Serial.print(F(" S="));
      Serial.print(config.spring);

      Serial.print(F(" M="));
      Serial.print(get_status().motor_enabled);

      Serial.print(F(" TMN="));
      Serial.print(calibration.throttle.min_raw);
      Serial.print(F(" TMX="));
      Serial.print(calibration.throttle.max_raw);

      Serial.print(F(" BMN="));
      Serial.print(calibration.brake.min_raw);
      Serial.print(F(" BMX="));
      Serial.print(calibration.brake.max_raw);

      Serial.print(F(" CMN="));
      Serial.print(calibration.clutch.min_raw);
      Serial.print(F(" CMX="));
      Serial.println(calibration.clutch.max_raw);
      return;
    }

    if (strcmp(raw, "FSTAT") == 0)
    {
      const FfbDeviceState &ffb_state = ffb_get_device_state();
      const UsbFfbRuntimeStatus &usb_status = usb_wheel_get_runtime_status();
      const DeviceStatus &status = get_status();

      Serial.print(F("FFB USB="));
      Serial.print(usb_wheel_ready() ? 1 : 0);
      Serial.print(F(" EN="));
      Serial.print(ffb_state.ffb_enabled ? 1 : 0);
      Serial.print(F(" HOST="));
      Serial.print(ffb_state.host_connected ? 1 : 0);
      Serial.print(F(" GAIN="));
      Serial.print(ffb_state.device_gain);
      Serial.print(F(" ALLOC="));
      Serial.print(ffb_get_allocated_effect_count());
      Serial.print(F(" ACT="));
      Serial.print(ffb_get_active_effect_count());
      Serial.print(F(" OUT="));
      Serial.print(status.output);
      Serial.print(F(" AGE="));
      Serial.print(ffb_state.last_ffb_packet_ms == 0 ? 0 : millis() - ffb_state.last_ffb_packet_ms);
      Serial.print(F(" UOUT="));
      Serial.print(usb_status.output_report_count);
      Serial.print(F(" UFEAT="));
      Serial.print(usb_status.feature_report_count);
      Serial.print(F(" LRID="));
      Serial.println(usb_status.last_report_id);
      return;
    }

    if (strcmp(raw, "HI") == 0)
    {
      Serial.println(F("OK BRSWDIY"));
      return;
    }

    if (strncmp(raw, "GAIN ", 5) == 0)
    {
      const int value = atoi(raw + 4);

      if (set_gain(value))
      {
        Serial.print(F("OK GAIN="));
        Serial.println(value);
      }
      else
      {
        write_error(3, "INVALID_RANGE", "ANG");
      }

      return;
    }

    if (strncmp(raw, "DAM ", 4) == 0)
    {
      const int value = atoi(raw + 4);

      if (set_damper(value))
      {
        Serial.print(F("OK DAM="));
        Serial.println(value);
      }
      else
      {
        write_error(3, "INVALID_RANGE", "DAM");
      }

      return;
    }

    if (strncmp(raw, "FRI ", 4) == 0)
    {
      const int value = atoi(raw + 4);

      if (set_friction(value))
      {
        Serial.print(F("OK FRI="));
        Serial.println(value);
      }
      else
      {
        write_error(3, "INVALID_RANGE", "FRI");
      }

      return;
    }

    if (strncmp(raw, "INE ", 4) == 0)
    {
      const int value = atoi(raw + 4);

      if (set_inertia(value))
      {
        Serial.print(F("OK INE="));
        Serial.println(value);
      }
      else
      {
        write_error(3, "INVALID_RANGE", "INE");
      }

      return;
    }

    if (strncmp(raw, "SPR ", 4) == 0)
    {
      const int value = atoi(raw + 4);

      if (set_spring(value))
      {
        Serial.print(F("OK SPR="));
        Serial.println(value);
      }
      else
      {
        write_error(3, "INVALID_RANGE", "SPR");
      }

      return;
    }

    if (strncmp(raw, "ANG ", 4) == 0)
    {
      const int value = atoi(raw + 4);

      if (set_max_angle(value))
      {
        Serial.print(F("OK ANG="));
        Serial.println(value);
      }
      else
      {
        write_error(3, "INVALID_RANGE", "ANG");
      }

      return;
    }

    if (strncmp(raw, "ENC ", 4) == 0)
    {
      const int value = atoi(raw + 4);

      if (set_encoder_ppr(value))
      {
        Serial.print(F("OK ENC="));
        Serial.println(value);
      }
      else
      {
        write_error(3, "INVALID_RANGE", "ENC");
      }

      return;
    }

    if (strncmp(raw, "INV ", 4) == 0)
    {
      const int value = atoi(raw + 4);

      if (value != 0 && value != 1)
      {
        write_error(4, "INVALID_STATE", "INV");
        return;
      }

      if (set_pedal_invert(value != 0))
      {
        Serial.print(F("OK INV="));
        Serial.println(value);
      }
      else
      {
        write_error(6, "NOT_CALIBRATED", "INV");
      }

      return;
    }

    if (strncmp(raw, "OUT ", 4) == 0)
    {
      const int value = atoi(raw + 4);

      if (set_output_limit(value))
      {
        Serial.print(F("OK OUT="));
        Serial.println(value);
      }
      else
      {
        write_error(3, "INVALID_RANGE", "OUT");
      }

      return;
    }

    if (strncmp(raw, "TMN ", 4) == 0)
    {
      const int value = atoi(raw + 4);

      if (set_throttle_min(value))
      {
        Serial.print(F("OK TMN="));
        Serial.println(value);
      }
      else
      {
        write_error(3, "INVALID_RANGE", "TMN");
      }

      return;
    }

    if (strncmp(raw, "TMX ", 4) == 0)
    {
      const int value = atoi(raw + 4);

      if (set_throttle_max(value))
      {
        Serial.print(F("OK TMX="));
        Serial.println(value);
      }
      else
      {
        write_error(3, "INVALID_RANGE", "TMX");
      }

      return;
    }

    if (strncmp(raw, "BMN ", 4) == 0)
    {
      const int value = atoi(raw + 4);

      if (set_brake_min(value))
      {
        Serial.print(F("OK BMN="));
        Serial.println(value);
      }
      else
      {
        write_error(3, "INVALID_RANGE", "BMN");
      }

      return;
    }

    if (strncmp(raw, "BMX ", 4) == 0)
    {
      const int value = atoi(raw + 4);

      if (set_brake_max(value))
      {
        Serial.print(F("OK BMX="));
        Serial.println(value);
      }
      else
      {
        write_error(3, "INVALID_RANGE", "BMX");
      }

      return;
    }

    if (strncmp(raw, "CMN ", 4) == 0)
    {
      const int value = atoi(raw + 4);

      if (set_clutch_min(value))
      {
        Serial.print(F("OK CMN="));
        Serial.println(value);
      }
      else
      {
        write_error(3, "INVALID_RANGE", "CMN");
      }

      return;
    }

    if (strncmp(raw, "CMX ", 4) == 0)
    {
      const int value = atoi(raw + 4);

      if (set_clutch_max(value))
      {
        Serial.print(F("OK CMX="));
        Serial.println(value);
      }
      else
      {
        write_error(3, "INVALID_RANGE", "CMX");
      }

      return;
    }

    if (strncmp(raw, "FFB ", 4) == 0)
    {
      const int value = atoi(raw + 4);

      if (value != 0 && value != 1)
      {
        write_error(4, "INVALID_STATE", "FFB");
        return;
      }

      if (handle_motor(value != 0))
      {
        Serial.print(F("OK FFB="));
        Serial.println(value);
      }
      else
      {
        write_error(6, "NOT_CALIBRATED", "FFB");
      }

      return;
    }

    if (strcmp(raw, "SAVE") == 0)
    {
      if (save_config())
      {
        Serial.println(F("OK SAVE=1"));
      }
      else
      {
        write_error(7, "EEPROM_ERROR", "SAVE");
      }

      return;
    }

    if (strcmp(raw, "LOAD") == 0)
    {
      if (load_config())
      {
        Serial.println(F("OK LOAD=1"));
      }
      else
      {
        write_error(7, "EEPROM_ERROR", "LOAD");
      }

      return;
    }

    if (strcmp(raw, "RESET") == 0)
    {
      reset_config();
      Serial.println(F("OK RESET=1"));
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
