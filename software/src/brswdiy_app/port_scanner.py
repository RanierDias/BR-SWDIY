from __future__ import annotations

from typing import List

import serial.tools.list_ports


def list_available_ports() -> List[str]:
    ports = serial.tools.list_ports.comports()
    return [port.device for port in ports]
