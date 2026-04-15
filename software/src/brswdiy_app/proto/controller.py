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
    build_set_encoder_ppr,
    build_set_gain,
    build_set_damper,
    build_set_friction,
    build_set_inertia,
    build_set_spring,
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
        self.response_queue = queue.Queue()
        self.comm_thread = None

    def connect(self, port: str, baudrate: int = 115200) -> None:
        if self.connection.is_open:
            self.disconnect()

        self.connection.open(port, baudrate)
        self.connected = True
        self.running = True
        self.port = port
        self.last_error = None
        self.command_queue = queue.Queue()
        self.response_queue = queue.Queue()
        self.comm_thread = threading.Thread(
            target=self._communication_worker, daemon=True)
        self.comm_thread.start()

    def disconnect(self) -> None:
        self.running = False
        self.connected = False

        if self.connection.is_open:
            self.connection.close()

        self.port = None
        if self.comm_thread is not None and self.comm_thread.is_alive():
            self.comm_thread.join(timeout=0.3)
        self.comm_thread = None

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

                    if cmd == "CAL":
                        for _ in range(20):
                            line = self.connection.read_line()

                            if not line:
                                break

                            frame = parse_line(line)

                            if isinstance(frame, CalibrationFrame):
                                self.response_queue.put(frame)
                                break

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
                self.running = False

    def send_command(self, cmd_str: str):
        self.command_queue.put(cmd_str)

    def read_calibration(self) -> CalibrationFrame | None:
        while not self.response_queue.empty():
            self.response_queue.get()

        self.send_command(build_calibration())

        try:
            frame = self.response_queue.get(timeout=1.0)

            if isinstance(frame, CalibrationFrame):
                return frame
            if isinstance(frame, ErrorFrame):
                self.last_error = f"{frame.code}:{frame.name}"
                return None
        except queue.Empty:
            self.last_error = "Timeout: No response"
            return None

    def apply_steering(self, max_angle: int, output_limit: int) -> None:
        self.send_command(build_set_max_angle(max_angle))
        self.send_command(build_set_output_limit(output_limit))

    def set_max_angle(self, value: int) -> None:
        self.send_command(build_set_max_angle(value))

    def set_output_limit(self, value: int) -> None:
        self.send_command(build_set_output_limit(value))

    def set_encoder_ppr(self, value: int) -> None:
        self.send_command(build_set_encoder_ppr(value))

    def apply_ffb_filters(self,
                          gain: int,
                          damper: int,
                          friction: int,
                          inertia: int,
                          spring: int) -> None:
        self.send_command(build_set_gain(gain))
        self.send_command(build_set_damper(damper))
        self.send_command(build_set_friction(friction))
        self.send_command(build_set_inertia(inertia))
        self.send_command(build_set_spring(spring))

    def recenter(self) -> None:
        self.send_command(build_recenter())

    def set_throttle_min(self, value: int) -> None:
        self.send_command(build_set_throttle_min(value))

    def set_throttle_max(self, value: int) -> None:
        self.send_command(build_set_throttle_max(value))

    def set_brake_min(self, value: int) -> None:
        self.send_command(build_set_brake_min(value))

    def set_brake_max(self, value: int) -> None:
        self.send_command(build_set_brake_max(value))

    def set_clutch_min(self, value: int) -> None:
        self.send_command(build_set_clutch_min(value))

    def set_clutch_max(self, value: int) -> None:
        self.send_command(build_set_clutch_max(value))

    def set_invert_pedals(self, enabled: bool) -> None:
        self.send_command(build_set_invert_pedals(enabled))

    def set_ffb_enable(self, enabled: bool) -> None:
        self.send_command(build_enable_ffb(enabled))

    def set_gain(self, value: int) -> None:
        self.send_command(build_set_gain(value))

    def set_damper(self, value: int) -> None:
        self.send_command(build_set_damper(value))

    def set_friction(self, value: int) -> None:
        self.send_command(build_set_friction(value))

    def set_inertia(self, value: int) -> None:
        self.send_command(build_set_inertia(value))

    def set_spring(self, value: int) -> None:
        self.send_command(build_set_spring(value))

    def save(self) -> None:
        self.send_command(build_save())

    def load(self) -> None:
        self.send_command(build_load())

    def reset_default(self) -> None:
        self.send_command(build_reset())

    def get_telemetry(self) -> DeviceTelemetry:
        return self.telemetry

    def is_connected(self) -> bool:
        return self.connected and self.connection.is_open
