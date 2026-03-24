#include "proto/response_stream.h"

#include "app/app.h"
#include <Arduino.h>

void send_telemetry_frame()
{
    const DeviceStatus &status = get_status();

    Serial.print('T');
    Serial.print(' ');
    Serial.print(status.angle);
    Serial.print(' ');
    Serial.print(status.throttle);
    Serial.print(' ');
    Serial.print(status.brake);
    Serial.print(' ');
    Serial.println(status.clutch);
}
