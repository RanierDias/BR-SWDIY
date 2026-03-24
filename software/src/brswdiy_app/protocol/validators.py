def is_supported_proto(fields: dict[str, str]) -> bool:
    return fields.get("PROTO") == "1"
