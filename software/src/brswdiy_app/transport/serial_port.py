from __future__ import annotations

from dataclasses import dataclass
from typing import Optional

import serial


@dataclass
class SerialConnection:
    port: str
    baudrate: int = 115200
    timeout: float = 0.2
    is_open: bool = False
    _serial: Optional[serial.Serial] = None

    def open(self, port: str, baudrate: int) -> None:
        self._serial = serial.Serial(
            port,
            baudrate,
            timeout=self.timeout
        )

        self.is_open = True

    def close(self) -> None:
        if self._serial and self._serial.is_open:
            self._serial.close()

        self.is_open = False

    def send_line(self, line: str) -> None:
        if not self._serial or not self._serial.is_open:
            raise RuntimeError("Serial port is not open")

        self._serial.write((line + "\n").encode("ascii"))

    def read_line(self) -> str:
        if not self._serial or not self._serial.is_open:
            raise RuntimeError("Serial port is not open")

        return self._serial.readline().decode("ascii", errors="replace").strip()
