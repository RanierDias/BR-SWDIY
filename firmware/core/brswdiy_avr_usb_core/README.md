# BRSWDIY AVR USB Core

This directory contains the custom AVR USB core files used by the firmware to
support HID Force Feedback on ATmega32U4 boards.

Why it exists:
- the stock Arduino AVR core does not expose the HID callbacks this firmware needs
- the firmware depends on USB/HID behavior that must be installed before build
- keeping the overlay in-repo makes the project self-contained and reproducible

Scope:
- `HID.cpp`
- `HID.h`
- `USBAPI.h`
- `USBCore.cpp`
- `USBCore.h`
- `USBDesc.h`

Build integration:
- `platformio.ini` installs this overlay through `scripts/install_brswdiy_core.py`

Maintenance note:
- this core should be treated as part of the firmware codebase
- future USB/FFB changes should be made here first, then validated on hardware
