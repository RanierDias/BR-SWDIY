from __future__ import annotations

from dataclasses import dataclass
from typing import Optional

from brswdiy_app.protocol.commands import get_config, get_status, hello
from brswdiy_app.protocol.parser import parse_kv_line
from brswdiy_app.protocol.validators import is_supported_proto
from brswdiy_app.transport.port_scanner import list_serial_ports
from brswdiy_app.transport.serial_port import SerialConnection


@dataclass
class BaseDevice:
    connection: Optional[SerialConnection] = None

    def list_ports(self) -> list[str]:
        return list_serial_ports()

    def connect(self, port: str) -> dict[str, str]:
        self.connection = SerialConnection(port=port)
        self.connection.open()
        self.connection.send_line(hello())
        response = self.connection.read_line()
        kind, fields = parse_kv_line(response)
        if kind != "HELLO" or not is_supported_proto(fields):
            raise RuntimeError(f"Unsupported device response: {response}")
        return fields

    def get_status(self) -> tuple[str, dict[str, str]]:
        if not self.connection:
            raise RuntimeError("Not connected")
        self.connection.send_line(get_status())
        return parse_kv_line(self.connection.read_line())

    def get_config(self) -> tuple[str, dict[str, str]]:
        if not self.connection:
            raise RuntimeError("Not connected")
        self.connection.send_line(get_config())
        return parse_kv_line(self.connection.read_line())
