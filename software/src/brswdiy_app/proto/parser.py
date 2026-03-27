from __future__ import annotations

from dataclasses import dataclass


@dataclass(slots=True)
class TelemetryFrame:
    angle: int
    throttle: int
    brake: int
    clutch: int


@dataclass(slots=True)
class CalibrationFrame:
    max_angle: int
    invert_pedals: bool
    motor_enable: bool
    output_limit: int
    throttle_min: int
    throttle_max: int
    brake_min: int
    brake_max: int
    clutch_min: int
    clutch_max: int


@dataclass(slots=True)
class ErrorFrame:
    code: int
    name: str | None = None
    field: str | None = None


@dataclass(slots=True)
class HandshakeFrame:
    device_name: str


def parse_handshake(line: str) -> HandshakeFrame | None:
    line = line.strip()

    if line != "OK BRSWDIY":
        return None

    return HandshakeFrame(device_name="BRSWDIY")


def parse_telemetry(line: str) -> TelemetryFrame | None:
    parts = line.strip().split()

    if parts[0] != "T":
        return None

    try:
        return TelemetryFrame(
            angle=int(parts[1]),
            throttle=int(parts[2]),
            brake=int(parts[3]),
            clutch=int(parts[4]),
        )
    except ValueError:
        return None


def parse_calibration(line: str) -> CalibrationFrame | None:
    parts = line.strip().split()

    if parts[0] != "C":
        return None

    values = _parse_kv_tokens(parts[1:])

    required_keys = ("A", "I", "O", "M", "TMN", "TMX",
                     "BMN", "BMX", "CMN", "CMX")

    if any(key not in values for key in required_keys):
        return None

    try:
        return CalibrationFrame(
            max_angle=int(values["A"]),
            invert_pedals=values["I"] == "1",
            output_limit=int(values["O"]),
            motor_enable=values["M"] == "1",
            throttle_min=int(values["TMN"]),
            throttle_max=int(values["TMX"]),
            brake_min=int(values["BMN"]),
            brake_max=int(values["BMX"]),
            clutch_min=int(values["CMN"]),
            clutch_max=int(values["CMX"]),
        )
    except ValueError:
        return None


def parse_error(line: str) -> ErrorFrame | None:
    parts = line.strip().split()

    if not parts:
        return None

    if parts[0] != "ERR":
        return None

    values = _parse_kv_tokens(parts[1:])

    if "CODE" not in values:
        return None

    try:
        return ErrorFrame(
            code=int(values["CODE"]),
            name=values.get("NAME"),
            field=values.get("FIELD"),
        )
    except ValueError:
        return None


def parse_line(
    line: str,
) -> HandshakeFrame | TelemetryFrame | CalibrationFrame | ErrorFrame | None:
    line = line.strip()

    if not line:
        return None

    frame = parse_handshake(line)
    if frame is not None:
        return frame

    frame = parse_telemetry(line)
    if frame is not None:
        return frame

    frame = parse_calibration(line)
    if frame is not None:
        return frame

    frame = parse_error(line)
    if frame is not None:
        return frame

    return None


def _parse_kv_tokens(parts: list[str]) -> dict[str, str]:
    values: dict[str, str] = {}

    for part in parts:
        if "=" not in part:
            continue

        key, value = part.split("=", 1)
        values[key] = value

    return values
