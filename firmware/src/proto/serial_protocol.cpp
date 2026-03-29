#include "proto/serial_protocol.h"

#include <Arduino.h>

#include "app/app.h"
#include "ffb/ffb_effects.h"
#include "usb/usb_wheel.h"

namespace
{
  constexpr uint8_t LINE_BUFFER_SIZE = 64;
  char g_line[LINE_BUFFER_SIZE];
  uint8_t g_line_len = 0;

  const char *effect_type_name(FfbEffectType type)
  {
    switch (type)
    {
    case FfbEffectType::ConstantForce:
      return "CONST";
    case FfbEffectType::Spring:
      return "SPRING";
    case FfbEffectType::Damper:
      return "DAMPER";
    case FfbEffectType::Friction:
      return "FRICTION";
    case FfbEffectType::Inertia:
      return "INERTIA";
    case FfbEffectType::Sine:
      return "SINE";
    case FfbEffectType::Ramp:
      return "RAMP";
    case FfbEffectType::None:
    default:
      return "NONE";
    }
  }

  void write_ok_pair(const __FlashStringHelper *name, int value)
  {
    Serial.print(F("OK "));
    Serial.print(name);
    Serial.print("=");
    Serial.println(value);
  }

  void write_effect_summary(uint8_t effect_id, const FfbEffectSlot &slot)
  {
    Serial.print(F("FX ID="));
    Serial.print(effect_id);
    Serial.print(F(" TYPE="));
    Serial.print(effect_type_name(slot.type));
    Serial.print(F(" EN="));
    Serial.print(slot.enabled ? 1 : 0);
    Serial.print(F(" G="));
    Serial.print(slot.gain);
    Serial.print(F(" MAG="));
    Serial.print(slot.magnitude);
    Serial.print(F(" CTR="));
    Serial.print(slot.condition.center);
    Serial.print(F(" DB="));
    Serial.print(slot.condition.deadband);
    Serial.print(F(" CP="));
    Serial.print(slot.condition.coefficient_positive);
    Serial.print(F(" CN="));
    Serial.println(slot.condition.coefficient_negative);
  }

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

      Serial.print(F("C A="));
      Serial.print(get_max_angle());

      Serial.print(F(" I="));
      Serial.print(calibration.invert_pedals ? 1 : 0);

      Serial.print(F(" O="));
      Serial.print(get_config().output_limit);

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

    if (strcmp(raw, "FXSTAT") == 0)
    {
      const FfbDeviceState &ffb_state = ffb_get_device_state();

      Serial.print(F("FFB EN="));
      Serial.print(ffb_state.ffb_enabled ? 1 : 0);
      Serial.print(F(" HOST="));
      Serial.print(ffb_state.host_connected ? 1 : 0);
      Serial.print(F(" GAIN="));
      Serial.print(ffb_state.device_gain);
      Serial.print(F(" ACTIVE="));
      Serial.println(ffb_get_active_effect_count());
      return;
    }

    if (strcmp(raw, "USBSTAT") == 0)
    {
      const UsbPidDebugState &debug = usb_wheel_get_debug_state();

      Serial.print(F("USB READY="));
      Serial.print(usb_wheel_ready() ? 1 : 0);
      Serial.print(F(" IN="));
      Serial.print(debug.input_reports_sent);
      Serial.print(F(" GET="));
      Serial.print(debug.get_report_count);
      Serial.print(F(" SET="));
      Serial.print(debug.set_report_count);
      Serial.print(F(" FEAT="));
      Serial.print(debug.feature_report_count);
      Serial.print(F(" OUT="));
      Serial.print(debug.output_report_count);
      Serial.print(F(" ERR="));
      Serial.print(debug.invalid_report_count);
      Serial.print(F(" LRID="));
      Serial.print(debug.last_report_id);
      Serial.print(F(" LRT="));
      Serial.print(debug.last_report_type);
      Serial.print(F(" LEN="));
      Serial.print(debug.last_report_length);
      Serial.print(F(" FX="));
      Serial.print(debug.last_effect_id);
      Serial.print(F(" OP="));
      Serial.println(debug.last_operation);
      return;
    }

    if (strcmp(raw, "USBAXIS") == 0)
    {
      const DeviceStatus &status = get_status();
      const uint16_t max_angle = get_max_angle();

      Serial.print(F("USBAXIS ANG="));
      Serial.print(status.angle);
      Serial.print(F(" MAXANG="));
      Serial.print(max_angle);
      Serial.print(F(" HID="));
      Serial.println(usb_wheel_get_steering_hid_value(status.angle));
      return;
    }

    if (strcmp(raw, "USBRST") == 0)
    {
      usb_wheel_reset_debug_state();
      Serial.println(F("OK USBRST=1"));
      return;
    }

    if (strcmp(raw, "HI") == 0)
    {
      Serial.println(F("OK BRSWDIY"));
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

    if (strncmp(raw, "AMX ", 4) == 0)
    {
      const int value = atoi(raw + 4);

      if (set_output_limit(value))
      {
        Serial.print(F("OK AMX="));
        Serial.println(value);
      }
      else
      {
        write_error(3, "INVALID_RANGE", "AMX");
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

    if (strncmp(raw, "FXGAIN ", 7) == 0)
    {
      const int gain = atoi(raw + 7);

      if (gain < 0 || gain > 100)
      {
        write_error(3, "INVALID_RANGE", "FXGAIN");
        return;
      }

      ffb_set_device_gain(static_cast<uint8_t>(gain), millis());
      write_ok_pair(F("FXGAIN"), gain);
      return;
    }

    if (strncmp(raw, "FXC ", 4) == 0)
    {
      int effect_id = 0;
      int magnitude = 0;
      int gain = 0;

      if (sscanf(raw + 4, "%d %d %d", &effect_id, &magnitude, &gain) != 3)
      {
        write_error(2, "INVALID_ARGUMENT", "FXC");
        return;
      }

      if (effect_id < 0 || effect_id > 15 || magnitude < -100 || magnitude > 100 || gain < 0 || gain > 100)
      {
        write_error(3, "INVALID_RANGE", "FXC");
        return;
      }

      if (!ffb_set_constant_force(static_cast<uint8_t>(effect_id),
                                  static_cast<int16_t>(magnitude),
                                  static_cast<uint8_t>(gain)))
      {
        write_error(8, "RESOURCE_EXHAUSTED", "FXC");
        return;
      }

      write_ok_pair(F("FXC"), effect_id);
      return;
    }

    if (strncmp(raw, "FXS ", 4) == 0)
    {
      int effect_id = 0;
      int center = 0;
      int deadband = 0;
      int coefficient = 0;
      int gain = 0;

      if (sscanf(raw + 4, "%d %d %d %d %d", &effect_id, &center, &deadband, &coefficient, &gain) != 5)
      {
        write_error(2, "INVALID_ARGUMENT", "FXS");
        return;
      }

      if (effect_id < 0 || effect_id > 15 || deadband < 0 || deadband > 2048 || coefficient < 0 || coefficient > 100 || gain < 0 || gain > 100)
      {
        write_error(3, "INVALID_RANGE", "FXS");
        return;
      }

      if (!ffb_set_spring(static_cast<uint8_t>(effect_id),
                          static_cast<int16_t>(center),
                          static_cast<int16_t>(deadband),
                          static_cast<int16_t>(coefficient),
                          static_cast<int16_t>(coefficient),
                          static_cast<uint8_t>(gain)))
      {
        write_error(8, "RESOURCE_EXHAUSTED", "FXS");
        return;
      }

      write_ok_pair(F("FXS"), effect_id);
      return;
    }

    if (strncmp(raw, "FXD ", 4) == 0)
    {
      int effect_id = 0;
      int coefficient = 0;
      int gain = 0;

      if (sscanf(raw + 4, "%d %d %d", &effect_id, &coefficient, &gain) != 3)
      {
        write_error(2, "INVALID_ARGUMENT", "FXD");
        return;
      }

      if (effect_id < 0 || effect_id > 15 || coefficient < 0 || coefficient > 100 || gain < 0 || gain > 100)
      {
        write_error(3, "INVALID_RANGE", "FXD");
        return;
      }

      if (!ffb_set_damper(static_cast<uint8_t>(effect_id),
                          static_cast<int16_t>(coefficient),
                          static_cast<int16_t>(coefficient),
                          static_cast<uint8_t>(gain)))
      {
        write_error(8, "RESOURCE_EXHAUSTED", "FXD");
        return;
      }

      write_ok_pair(F("FXD"), effect_id);
      return;
    }

    if (strncmp(raw, "FXGO ", 5) == 0)
    {
      const int effect_id = atoi(raw + 5);

      if (effect_id < 0 || effect_id > 15)
      {
        write_error(3, "INVALID_RANGE", "FXGO");
        return;
      }

      ffb_start_effect(static_cast<uint8_t>(effect_id), millis());
      write_ok_pair(F("FXGO"), effect_id);
      return;
    }

    if (strncmp(raw, "FXSTOP ", 7) == 0)
    {
      const int effect_id = atoi(raw + 7);

      if (effect_id < 0 || effect_id > 15)
      {
        write_error(3, "INVALID_RANGE", "FXSTOP");
        return;
      }

      ffb_stop_effect(static_cast<uint8_t>(effect_id));
      write_ok_pair(F("FXSTOP"), effect_id);
      return;
    }

    if (strncmp(raw, "FXDEL ", 6) == 0)
    {
      const int effect_id = atoi(raw + 6);

      if (effect_id < 0 || effect_id > 15)
      {
        write_error(3, "INVALID_RANGE", "FXDEL");
        return;
      }

      ffb_clear_effect(static_cast<uint8_t>(effect_id));
      write_ok_pair(F("FXDEL"), effect_id);
      return;
    }

    if (strcmp(raw, "FXCLR") == 0)
    {
      ffb_clear_all_effects();
      Serial.println(F("OK FXCLR=1"));
      return;
    }

    if (strncmp(raw, "FXINFO ", 7) == 0)
    {
      const int effect_id = atoi(raw + 7);

      if (effect_id < 0 || effect_id > 15)
      {
        write_error(3, "INVALID_RANGE", "FXINFO");
        return;
      }

      FfbEffectSlot slot;

      if (!ffb_get_effect_slot(static_cast<uint8_t>(effect_id), slot))
      {
        write_error(1, "UNKNOWN_COMMAND", "FXINFO");
        return;
      }

      write_effect_summary(static_cast<uint8_t>(effect_id), slot);
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
