from __future__ import annotations


def build_hi() -> str:
    return "HI"


def build_telemetry() -> str:
    return "TEL"


def build_calibration() -> str:
    return "CAL"


def build_enable_ffb(enabled: bool) -> str:
    return f"FFB {1 if enabled else 0}"


def build_set_max_angle(value: int) -> str:
    return f"ANG {value}"


def build_set_output_limit(value: int) -> str:
    return f"AMX {value}"


def build_recenter() -> str:
    return "RCE"


def build_set_invert_pedals(enabled: bool) -> str:
    return f"INV {1 if enabled else 0}"


def build_set_throttle_min(value: int) -> str:
    return f"TMN {value}"


def build_set_throttle_max(value: int) -> str:
    return f"TMX {value}"


def build_set_brake_min(value: int) -> str:
    return f"BMN {value}"


def build_set_brake_max(value: int) -> str:
    return f"BMX {value}"


def build_set_clutch_min(value: int) -> str:
    return f"CMN {value}"


def build_set_clutch_max(value: int) -> str:
    return f"CMX {value}"


def build_save() -> str:
    return "SAVE"


def build_load() -> str:
    return "LOAD"


def build_reset() -> str:
    return "RESET"
