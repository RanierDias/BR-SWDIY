from brbase_app.core.device import BRBaseDevice


def test_device_can_be_instantiated() -> None:
    device = BRBaseDevice()
    assert isinstance(device.list_ports(), list)
