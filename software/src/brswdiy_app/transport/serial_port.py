from __future__ import annotations

from dataclasses import dataclass
from typing import Optional

import serial


@dataclass
class SerialConnection:
    port: str
    baudrate: int = 115200
    timeout: float = 0.2
    _serial: Optional[serial.Serial] = None

    def open(self) -> None:
        self._serial = serial.Serial(self.port, self.baudrate, timeout=self.timeout)

    def close(self) -> None:
        if self._serial and self._serial.is_open:
            self._serial.close()

    def send_line(self, line: str) -> None:
        if not self._serial or not self._serial.is_open:
            raise RuntimeError("Serial port is not open")
        self._serial.write((line + "\n").encode("ascii"))

    def read_line(self) -> str:
        if not self._serial or not self._serial.is_open:
            raise RuntimeError("Serial port is not open")
        return self._serial.readline().decode("ascii", errors="replace").strip()
