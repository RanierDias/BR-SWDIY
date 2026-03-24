from brswdiy_app.protocol.parser import parse_kv_line


def test_parse_kv_line() -> None:
    kind, fields = parse_kv_line("OK GAIN=25 SAFE_START=1")
    assert kind == "OK"
    assert fields["GAIN"] == "25"
    assert fields["SAFE_START"] == "1"
