from pathlib import Path
import shutil

Import("env")


CORE_FILES = [
    "HID.cpp",
    "HID.h",
    "USBAPI.h",
    "USBCore.cpp",
    "USBCore.h",
    "USBDesc.h",
]


def install_brswdiy_core(*args):
    project_dir = Path(env["PROJECT_DIR"])
    overlay_dir = project_dir / "core" / "brswdiy_avr_usb_core"
    framework_dir = Path(env.PioPlatform().get_package_dir("framework-arduino-avr"))
    core_dir = framework_dir / "cores" / "arduino"

    if not overlay_dir.exists():
        print("BRSWDIY USB core overlay not found, skipping custom USB core installation.")
        return

    core_dir.mkdir(parents=True, exist_ok=True)

    for name in CORE_FILES:
        shutil.copy2(overlay_dir / name, core_dir / name)

    print(f"Installed BRSWDIY USB core overlay from {overlay_dir} to {core_dir}")


install_brswdiy_core()
