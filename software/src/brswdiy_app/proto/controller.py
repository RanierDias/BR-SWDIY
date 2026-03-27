from __future__ import annotations

from dataclasses import dataclass

from brswdiy_app.serial_port import SerialConnection
from brswdiy_app.port_scanner import list_available_ports
from brswdiy_app.proto.protocol import (
    build_hi,
    build_telemetry,
    build_calibration,
    build_enable_ffb,
    build_set_max_angle,
    build_set_output_limit,
    build_recenter,
    build_set_invert_pedals,
    build_set_throttle_min,
    build_set_throttle_max,
    build_set_brake_min,
    build_set_brake_max,
    build_set_clutch_min,
    build_set_clutch_max,
    build_save,
    build_load,
    build_reset,
)
from brswdiy_app.proto.parser import (
    TelemetryFrame,
    CalibrationFrame,
    ErrorFrame,
    HandshakeFrame,
    parse_line,
)

import threading
import queue
import time


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
        self.running = False
        self.port: str | None = None
        self.last_error: str | None = None

        self.command_queue = queue.Queue()
        self.comm_thread = None

    def connect(self, port: str, baudrate: int = 115200) -> None:
        self.connection.open(port, baudrate)
        self.connected = True
        self.running = True
        self.port = port
        self.last_error = None
        self.comm_thread = threading.Thread(
            target=self._communication_worker, daemon=True)
        self.comm_thread.start()

    def disconnect(self) -> None:
        if self.connection.is_open:
            self.connection.close()

        self.connected = False
        self.port = None

    def auto_detect(self, baudrate: int = 115200) -> str | None:
        for port in list_available_ports():
            temp = SerialConnection(port)

            try:
                temp.open(port, baudrate)
                temp.send_line(build_hi())
                line = temp.read_line()

                frame = parse_line(line)
                if isinstance(frame, HandshakeFrame):
                    temp.close()
                    return port

            except Exception:
                pass

            finally:
                if temp.is_open:
                    temp.close()

        return None

    def _communication_worker(self):
        while self.running:
            if not self.connected:
                time.sleep(0.1)
                continue

            try:
                while not self.command_queue.empty():
                    cmd = self.command_queue.get_nowait()
                    self.connection.send_line(cmd)

                self.connection.send_line(build_telemetry())
                line = self.connection.read_line()

                frame = parse_line(line)
                if isinstance(frame, TelemetryFrame):
                    self.telemetry.angle = frame.angle
                    self.telemetry.throttle = frame.throttle
                    self.telemetry.brake = frame.brake
                    self.telemetry.clutch = frame.clutch

                time.sleep(0.001)
            except Exception:
                self.connected = False

    def send_command(self, cmd_str: str):
        self.command_queue.put(cmd_str)

    def read_telemetry(self) -> TelemetryFrame | None:
        self.connection.send_line(build_telemetry())
        line = self.connection.read_line()

        frame = parse_line(line)

        if isinstance(frame, TelemetryFrame):
            self.telemetry.angle = frame.angle
            self.telemetry.throttle = frame.throttle
            self.telemetry.brake = frame.brake
            self.telemetry.clutch = frame.clutch
            return frame

        if isinstance(frame, ErrorFrame):
            self.last_error = f"{frame.code}:{frame.name}:{frame.field}"
            return None

        return None

    def read_calibration(self) -> CalibrationFrame | None:
        self.connection.send_line(build_calibration())
        line = self.connection.read_line()

        frame = parse_line(line)

        if isinstance(frame, CalibrationFrame):
            return frame

        if isinstance(frame, ErrorFrame):
            self.last_error = f"{frame.code}:{frame.name}:{frame.field}"
            return None

        return None

    def apply_steering(self, max_angle: int, output_limit: int) -> None:
        self.connection.send_line(build_set_max_angle(max_angle))
        self.connection.send_line(build_set_output_limit(output_limit))

    def recenter(self) -> None:
        self.connection.send_line(build_recenter())

    def set_throttle_min(self, value: int) -> None:
        self.connection.send_line(build_set_throttle_min(value))

    def set_throttle_max(self, value: int) -> None:
        self.connection.send_line(build_set_throttle_max(value))

    def set_brake_min(self, value: int) -> None:
        self.connection.send_line(build_set_brake_min(value))

    def set_brake_max(self, value: int) -> None:
        self.connection.send_line(build_set_brake_max(value))

    def set_clutch_min(self, value: int) -> None:
        self.connection.send_line(build_set_clutch_min(value))

    def set_clutch_max(self, value: int) -> None:
        self.connection.send_line(build_set_clutch_max(value))

    def set_invert_pedals(self, enabled: bool) -> None:
        self.connection.send_line(build_set_invert_pedals(enabled))

    def set_ffb_enable(self, enabled: bool) -> None:
        self.connection.send_line(build_enable_ffb(enabled))

    def save(self) -> None:
        self.connection.send_line(build_save())

    def load(self) -> None:
        self.connection.send_line(build_load())

    def reset_default(self) -> None:
        self.connection.send_line(build_reset())

    def get_telemetry(self) -> DeviceTelemetry:
        return self.telemetry

    def is_connected(self) -> bool:
        return self.connected and self.connection.is_open
