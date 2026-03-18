from brbase_app.core.device import BRBaseDevice


def main() -> None:
    device = BRBaseDevice()
    print("BRBase app scaffold ready.")
    print(f"Detected ports: {device.list_ports()}")


if __name__ == "__main__":
    main()
