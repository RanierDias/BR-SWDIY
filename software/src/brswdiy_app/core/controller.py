from __future__ import annotations

from dataclasses import dataclass

from brswdiy_app.transport.serial_port import SerialConnection
from brswdiy_app.protocol.stream_commands import (
    output_ffb,
    build_start_command,
    build_stop_command
)
from brswdiy_app.protocol.stream_parser import Telemetry, parse_stream


@dataclass(slots=True)
class DeviceTelemetry:
    angle: int = 0
    throttle: int = 0
    brake: int = 0
    clutch: int = 0


class Controller:
    def __init__(self) -> None:
        self.connection = SerialConnection("")
        self.telemetry = DeviceTelemetry()
        self.connected = False
        self.last_error: str | None = None

    def connect(self, port: str, baudrate: int = 115200) -> None:
        self.connection.open(port, baudrate)
        self.connected = True
        self.last_error = None

    def disconnect(self) -> None:
        if self.connection.is_open:
            self.connection.close()

        self.connected = False

    def start_motor(self) -> None:
        self.connection.send_line(build_start_command())

    def stop_motor(self) -> None:
        self.connection.send_line(build_stop_command())

    def set_output(self, value: int) -> None:
        if value < -100 or value > 100:
            raise ValueError("output must be between 100 and -100")

        self.connection.send_line(output_ffb(value))

    def poll(self) -> Telemetry | None:
        line = self.connection.read_line()

        if not line:
            return None

        frame = parse_stream(line)

        if frame is None:
            return None

        self.telemetry.angle = frame.angle
        self.telemetry.throttle = frame.throttle
        self.telemetry.brake = frame.brake
        self.telemetry.clutch = frame.clutch

        return frame

    def get_telemetry(self) -> DeviceTelemetry:
        return self.telemetry
