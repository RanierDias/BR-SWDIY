from serial.tools import list_ports


def list_serial_ports() -> list[str]:
    return [port.device for port in list_ports.comports()]
