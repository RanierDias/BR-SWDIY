from dataclasses import dataclass


@dataclass(slots=True)
class Telemetry:
    angle: int
    throttle: int
    brake: int
    clutch: int


def parse_stream(line: str) -> Telemetry | None:
    parts = line.strip().split()

    if len(parts) != 5:
        return None

    try:
        return Telemetry(
            angle=int(parts[1]),
            throttle=int(parts[2]),
            brake=int(parts[3]),
            clutch=int(parts[4])
        )
    except ValueError:
        return None
