from __future__ import annotations

from dataclasses import dataclass
from typing import Optional

import serial


@dataclass
class SerialConnection:
    port: str
    baudrate: int = 115200
    timeout: float = 0.5

    _serial: Optional[serial.Serial] = None

    @property
    def is_open(self) -> bool:
        return self._serial is not None and self._serial.is_open

    def open(self, port: str, baudrate: int) -> None:
        self._serial = serial.Serial(
            port=port,
            baudrate=baudrate,
            timeout=self.timeout,
        )
        self.port = port
        self.baudrate = baudrate

    def close(self) -> None:
        if self._serial and self._serial.is_open:
            self._serial.close()

        self._serial = None

    def send_line(self, line: str) -> None:
        if not self.is_open:
            raise RuntimeError("Serial port is not open")

        if self._serial is not None:
            self._serial.write((line + "\n").encode("ascii"))

    def read_line(self) -> str:
        if not self.is_open:
            raise RuntimeError("Serial port is not open")

        if self._serial is not None:
            return self._serial.readline().decode("ascii", errors="replace").strip()

        return "ERR CODE=0 NAME=NONE FIELD=N/A"
