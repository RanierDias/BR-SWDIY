#include "hw/motor.h"

#include <Arduino.h>

namespace
{
    constexpr uint8_t MOTOR_LEFT_PIN = 9;
    constexpr uint8_t MOTOR_RIGHT_PIN = 10;
    constexpr uint8_t MOTOR_ENABLE_PIN = 11;
}

static uint8_t percent_to_pwm(uint16_t percent)
{
    return static_cast<uint8_t>((percent * 255) / 100);
}

void setup_motor()
{
    pinMode(MOTOR_LEFT_PIN, OUTPUT);
    pinMode(MOTOR_RIGHT_PIN, OUTPUT);
    pinMode(MOTOR_ENABLE_PIN, OUTPUT);

    digitalWrite(MOTOR_ENABLE_PIN, LOW);

    analogWrite(MOTOR_LEFT_PIN, 0);
    analogWrite(MOTOR_RIGHT_PIN, 0);
}

void set_motor_output(int8_t output_percent)
{
    if (output_percent == 0)
    {
        analogWrite(MOTOR_LEFT_PIN, 0);
        analogWrite(MOTOR_RIGHT_PIN, 0);
        digitalWrite(MOTOR_ENABLE_PIN, LOW);
        return;
    }

    digitalWrite(MOTOR_ENABLE_PIN, HIGH);

    const uint8_t magnitude = output_percent > 0 ? output_percent : -output_percent;
    const uint8_t pwm = percent_to_pwm(magnitude);

    // The installed motor/H-bridge polarity is opposite to the FFB sign convention.
    if (output_percent > 0)
    {
        analogWrite(MOTOR_LEFT_PIN, pwm);
        analogWrite(MOTOR_RIGHT_PIN, 0);
    }
    else
    {
        analogWrite(MOTOR_LEFT_PIN, 0);
        analogWrite(MOTOR_RIGHT_PIN, pwm);
    }
}
