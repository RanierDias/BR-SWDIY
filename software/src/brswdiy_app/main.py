from brswdiy_app.core.device import BaseDevice


def main() -> None:
    device = BaseDevice()
    print("BRBase app scaffold ready.")
    print(f"Detected ports: {device.list_ports()}")


if __name__ == "__main__":
    main()
