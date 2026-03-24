from brswdiy_app.core.controller import Controller


def main() -> None:
    dev = Controller()
    dev.connect("COM18")

    dev.start_motor()
    dev.set_output(20)

    try:
        while True:
            frame = dev.poll()

            if frame is not None:
                print(frame)
    finally:
        dev.stop_motor()
        dev.disconnect()


if __name__ == "__main__":
    main()
