from brswdiy_app.protocol.commands import get_config, get_status, hello


def test_commands() -> None:
    assert hello() == "HELLO"
    assert get_status() == "GET_STATUS"
    assert get_config() == "GET_CONFIG"
