from dataclasses import dataclass


@dataclass
class ProtocolResponse:
    kind: str
    fields: dict[str, str]
