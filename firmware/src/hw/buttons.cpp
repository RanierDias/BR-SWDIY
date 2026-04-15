#include "hw/buttons.h"

#include <Arduino.h>

namespace
{
constexpr uint8_t LEFT_PADDLE_PIN = 12;
constexpr uint8_t RIGHT_PADDLE_PIN = 13;

constexpr uint8_t LEFT_PADDLE_MASK = 1 << 0;
constexpr uint8_t RIGHT_PADDLE_MASK = 1 << 1;
constexpr uint32_t DEBOUNCE_MS = 5;

struct DebouncedButton
{
  uint8_t pin;
  uint8_t mask;
  bool stable_state;
  bool raw_state;
  uint32_t last_change_ms;

  DebouncedButton(uint8_t button_pin, uint8_t button_mask)
      : pin(button_pin), mask(button_mask), stable_state(false), raw_state(false), last_change_ms(0)
  {
  }
};

DebouncedButton g_buttons[] = {
    DebouncedButton(LEFT_PADDLE_PIN, LEFT_PADDLE_MASK),
    DebouncedButton(RIGHT_PADDLE_PIN, RIGHT_PADDLE_MASK),
};

bool sample_button(uint8_t pin)
{
  return digitalRead(pin) == LOW;
}
} // namespace

void setup_buttons()
{
  pinMode(LEFT_PADDLE_PIN, INPUT_PULLUP);
  pinMode(RIGHT_PADDLE_PIN, INPUT_PULLUP);

  const uint32_t now_ms = millis();
  for (auto &button : g_buttons)
  {
    const bool pressed = sample_button(button.pin);
    button.stable_state = pressed;
    button.raw_state = pressed;
    button.last_change_ms = now_ms;
  }
}

uint8_t read_buttons()
{
  const uint32_t now_ms = millis();
  uint8_t buttons = 0;

  for (auto &button : g_buttons)
  {
    const bool raw_pressed = sample_button(button.pin);
    if (raw_pressed != button.raw_state)
    {
      button.raw_state = raw_pressed;
      button.last_change_ms = now_ms;
    }

    if ((now_ms - button.last_change_ms) >= DEBOUNCE_MS)
    {
      button.stable_state = button.raw_state;
    }

    if (button.stable_state)
    {
      buttons |= button.mask;
    }
  }

  return buttons;
}
