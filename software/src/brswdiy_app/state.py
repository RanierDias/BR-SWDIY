from __future__ import annotations

from dataclasses import dataclass, field


@dataclass(slots=True)
class PedalState:
    current: int = 0
    minimum: int = 0
    maximum: int = 1023

    min_offset: int = 0
    max_offset: int = 0


@dataclass(slots=True)
class AppState:
    connected: bool = False
    detected_port: str | None = None
    status_text: str = "Disconnected"

    angle: int = 0
    max_angle: int = 1080
    output_limit: int = 75
    gain: int = 50
    damper: int = 6
    friction: int = 2
    inertia: int = 2
    spring: int = 0

    throttle: PedalState = field(default_factory=PedalState)
    brake: PedalState = field(default_factory=PedalState)
    clutch: PedalState = field(default_factory=PedalState)

    invert_pedals: bool = False
    ffb_enabled: bool = True

    last_error: str | None = None
