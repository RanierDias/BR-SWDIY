def build_start_command() -> str:
    return "S"


def build_stop_command() -> str:
    return "X"


def output_ffb(value: int) -> str:
    return f"O {value}"
