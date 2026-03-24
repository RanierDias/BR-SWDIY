from brswdiy_app.core.controller import BaseDevice


def test_device_can_be_instantiated() -> None:
    device = BaseDevice()
    assert isinstance(device.list_ports(), list)
