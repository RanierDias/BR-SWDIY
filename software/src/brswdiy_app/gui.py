from __future__ import annotations

import customtkinter as ctk

from brswdiy_app.state import AppState
from src.brswdiy_app.ui.theme import (
    setup_theme,
    BG_MAIN,
    BG_PANEL,
    BG_ELEMENT,
    ACCENT,
    ACCENT_SOFT,
    TEXT,
    TEXT_DIM,
    BORDER,
)

from brswdiy_app.proto.controller import Controller


class App(ctk.CTk):
    def __init__(self) -> None:
        super().__init__()

        setup_theme()

        self.app_state = AppState()
        self.controller = Controller()

        self.title("BRSWDIY - Apus")
        self.geometry("1100x700")
        self.minsize(1000, 650)

        self.configure(fg_color=BG_MAIN)

        self._configure_grid()
        self._build_header()
        self._build_main()
        self._build_footer()

        self._bind_callbacks()
        self.after(50, self._update_loop)

    def _configure_grid(self) -> None:
        self.grid_rowconfigure(0, weight=0)
        self.grid_rowconfigure(1, weight=1)
        self.grid_rowconfigure(2, weight=0)

        self.grid_columnconfigure(0, weight=1)

    def _build_header(self) -> None:
        header = ctk.CTkFrame(
            self,
            fg_color=BG_PANEL,
            corner_radius=10,
            border_width=1,
            border_color=BORDER,
        )
        header.grid(row=0, column=0, sticky="ew", padx=12, pady=(12, 6))

        header.grid_columnconfigure(0, weight=1)
        header.grid_columnconfigure(1, weight=0)
        header.grid_columnconfigure(2, weight=0)

        title = ctk.CTkLabel(
            header,
            text="Apus Utility",
            font=ctk.CTkFont(size=22, weight="bold"),
            text_color=TEXT,
        )
        title.grid(row=0, column=0, sticky="w", padx=14, pady=12)

        right_box = ctk.CTkFrame(header, fg_color="transparent")
        right_box.grid(row=0, column=2, sticky="e", padx=14, pady=12)

        self.res_label = ctk.CTkLabel(
            header,
            text="Se beber não dirija",
            font=ctk.CTkFont(size=14, weight="bold"),
            text_color=TEXT_DIM,
        )
        self.res_label.grid(row=0, column=1, padx=(0, 25), pady=12)

        self.status_label = ctk.CTkLabel(
            right_box,
            text="Disconnected",
            font=ctk.CTkFont(size=13, weight="bold"),
            text_color=TEXT_DIM,
        )
        self.status_label.pack(side="left", padx=(0, 10))

        self.switch_button = ctk.CTkButton(
            right_box,
            text="Connect",
            width=90,
            fg_color=ACCENT,
            hover_color=ACCENT_SOFT,
        )
        self.switch_button.pack(side="left")

    def _build_main(self) -> None:
        main = ctk.CTkFrame(self, fg_color="transparent")
        main.grid(row=1, column=0, sticky="nsew", padx=12, pady=6)

        main.grid_rowconfigure(0, weight=1)
        main.grid_columnconfigure(0, weight=1)
        main.grid_columnconfigure(1, weight=2)
        main.grid_columnconfigure(2, weight=1)

        self.steering_frame = self._make_panel(main, "Steering")
        self.steering_frame.grid(row=0, column=0, sticky="nsew", padx=(0, 6))

        self.monitor_frame = self._make_panel(main, "Live Monitor")
        self.monitor_frame.grid(row=0, column=1, sticky="nsew", padx=6)

        self.pedals_frame = self._make_panel(main, "Pedals")
        self.pedals_frame.grid(row=0, column=2, sticky="nsew", padx=(6, 0))

        self._build_steering_panel()
        self._build_monitor_panel()
        self._build_pedals_panel()

    def _make_panel(self, parent, title: str) -> ctk.CTkFrame:
        panel = ctk.CTkFrame(
            parent,
            fg_color=BG_PANEL,
            corner_radius=10,
            border_width=1,
            border_color=BORDER,
        )

        label = ctk.CTkLabel(
            panel,
            text=title,
            font=ctk.CTkFont(size=16, weight="bold"),
            text_color=TEXT,
        )
        label.pack(anchor="w", padx=14, pady=(12, 8))

        return panel

    def _build_steering_panel(self) -> None:
        body = ctk.CTkFrame(self.steering_frame, fg_color="transparent")
        body.pack(fill="both", expand=True, padx=12, pady=(0, 12))

        angle_box = ctk.CTkFrame(body, fg_color="transparent")
        angle_box.pack(fill="both", expand=False, pady=(10, 0))

        angle_box.grid_columnconfigure(0, weight=1)
        angle_box.grid_columnconfigure(1, weight=1)

        max_angle_label = ctk.CTkLabel(
            angle_box,
            text="Lock Angle",
            text_color=TEXT,
        )
        max_angle_label.grid(row=0, column=0, sticky="w")

        self.max_angle_value = ctk.CTkLabel(
            angle_box,
            text="1080",
            text_color=TEXT
        )
        self.max_angle_value.grid(row=0, column=1, sticky="e")

        self.max_angle_slider = ctk.CTkSlider(
            body,
            from_=180,
            to=1080,
            number_of_steps=900,
            fg_color=BG_ELEMENT,
            progress_color=ACCENT,
            button_color=ACCENT,
            button_hover_color=ACCENT,
        )
        self.max_angle_slider.set(940)
        self.max_angle_slider.pack(fill="x", pady=(0, 14))

        output_box = ctk.CTkFrame(body, fg_color="transparent")
        output_box.pack(fill="both", expand=False, pady=(10, 0))

        output_box.grid_columnconfigure(0, weight=1)
        output_box.grid_columnconfigure(1, weight=1)

        output_limit_label = ctk.CTkLabel(
            output_box,
            text="Strength",
            text_color=TEXT,
        )
        output_limit_label.grid(row=0, column=0, sticky="w")

        self.output_limit_value = ctk.CTkLabel(
            output_box,
            text="75",
            text_color=TEXT,
        )
        self.output_limit_value.grid(row=0, column=1, sticky="e")

        self.output_limit_slider = ctk.CTkSlider(
            body,
            from_=0,
            to=100,
            number_of_steps=100,
            fg_color=BG_ELEMENT,
            progress_color=ACCENT,
            button_color=ACCENT,
            button_hover_color=ACCENT,
        )
        self.output_limit_slider.set(75)
        self.output_limit_slider.pack(fill="x", pady=(0, 18))

        buttons = ctk.CTkFrame(body, fg_color="transparent")
        buttons.pack(fill="x", pady=(8, 0))

        self.recenter_button = ctk.CTkButton(
            buttons,
            text="Recenter",
            fg_color=BG_ELEMENT,
            hover_color=ACCENT,
        )
        self.recenter_button.pack(fill="x", pady=(0, 8))

        self.apply_steering_button = ctk.CTkButton(
            buttons,
            text="Apply Steering",
            fg_color=ACCENT,
            hover_color=ACCENT_SOFT,
        )
        self.apply_steering_button.pack(fill="x")

    def _build_monitor_panel(self) -> None:
        body = ctk.CTkFrame(self.monitor_frame, fg_color="transparent")
        body.pack(fill="both", expand=True, padx=12, pady=(0, 12))

        self.angle_value_label = ctk.CTkLabel(
            body,
            text="0°",
            font=ctk.CTkFont(size=38, weight="bold"),
            text_color=TEXT,
        )
        self.angle_value_label.pack(
            anchor="center", pady=(12, 16), padx=(15, 0))

        pedals_box = ctk.CTkFrame(body, fg_color="transparent")
        pedals_box.pack(fill="both", expand=True, pady=(10, 0))

        pedals_box.grid_columnconfigure(0, weight=1)
        pedals_box.grid_columnconfigure(1, weight=1)
        pedals_box.grid_columnconfigure(2, weight=1)

        self._build_monitor_pedal_column(pedals_box, 0, "Throttle")
        self._build_monitor_pedal_column(pedals_box, 1, "Brake")
        self._build_monitor_pedal_column(pedals_box, 2, "Clutch")

    def _build_monitor_pedal_column(self, parent, column: int, name: str) -> None:
        box = ctk.CTkFrame(parent, fg_color="transparent")
        box.grid(row=0, column=column, sticky="n", padx=10)

        label = ctk.CTkLabel(
            box,
            text=name,
            text_color=TEXT,
        )
        label.pack(pady=(0, 8))

        bar = ctk.CTkProgressBar(
            box,
            orientation="vertical",
            height=220,
            width=20,
            fg_color=BG_ELEMENT,
            progress_color=ACCENT
        )
        bar.set(0)
        bar.pack()

        value = ctk.CTkLabel(
            box,
            text="0",
            text_color=TEXT_DIM,
        )
        value.pack(pady=(8, 0))

        if name == "Throttle":
            self.throttle_slider = bar
            self.throttle_value_label = value
        elif name == "Brake":
            self.brake_slider = bar
            self.brake_value_label = value
        elif name == "Clutch":
            self.clutch_slider = bar
            self.clutch_value_label = value

    def _build_pedals_panel(self) -> None:
        body = ctk.CTkFrame(self.pedals_frame, fg_color="transparent")
        body.pack(fill="both", expand=True, padx=12, pady=(0, 12))

        self._build_calibration_pedal_section(body, "Throttle")
        self._build_calibration_pedal_section(body, "Brake")
        self._build_calibration_pedal_section(body, "Clutch")

    def _build_calibration_pedal_section(self, parent, name: str) -> None:
        section = ctk.CTkFrame(
            parent,
            fg_color=BG_ELEMENT,
            corner_radius=8,
            border_width=1,
            border_color=BORDER,
        )
        section.pack(fill="x", pady=(0, 10))

        title = ctk.CTkLabel(
            section,
            text=name,
            font=ctk.CTkFont(size=14, weight="bold"),
            text_color=TEXT,
        )
        title.pack(anchor="w", padx=10, pady=(8, 4))

        buttons_box = ctk.CTkFrame(section, fg_color="transparent")
        buttons_box.pack(fill="both", expand=True, pady=(10, 10))

        buttons_box.grid_columnconfigure(0, weight=1)
        buttons_box.grid_columnconfigure(1, weight=1)
        buttons_box.grid_columnconfigure(2, weight=1)

        set_min_button = ctk.CTkButton(
            buttons_box,
            text="Set Min",
            width=80,
            fg_color=BG_MAIN,
            hover_color=ACCENT,
        )
        set_min_button.grid(row=0, column=0, sticky="n", padx=5)

        offset_label = ctk.CTkLabel(
            buttons_box,
            text="5",
            text_color=TEXT_DIM
        )
        offset_label.grid(row=0, column=1, padx=5)

        set_max_button = ctk.CTkButton(
            buttons_box,
            text="Set Max",
            width=80,
            fg_color=BG_MAIN,
            hover_color=ACCENT,
        )
        set_max_button.grid(row=0, column=2, sticky="n", padx=5)

        offset_slider = ctk.CTkSlider(
            section,
            from_=0,
            to=15,
            number_of_steps=15,
            fg_color=BG_MAIN,
            progress_color=ACCENT,
            button_color=ACCENT,
            button_hover_color=ACCENT,
        )
        offset_slider.set(5)
        offset_slider.pack(fill="x", padx=10, pady=(10, 12))

        if name == "Throttle":
            self.throttle_offset_slider = offset_slider
            self.throttle_offset_label = offset_label
            self.throttle_set_min_button = set_min_button
            self.throttle_set_max_button = set_max_button

        elif name == "Brake":
            self.brake_offset_slider = offset_slider
            self.brake_offset_label = offset_label
            self.brake_set_min_button = set_min_button
            self.brake_set_max_button = set_max_button

        elif name == "Clutch":
            self.clutch_offset_slider = offset_slider
            self.clutch_offset_label = offset_label
            self.clutch_set_min_button = set_min_button
            self.clutch_set_max_button = set_max_button

    def _build_footer(self) -> None:
        footer = ctk.CTkFrame(
            self,
            fg_color=BG_PANEL,
            corner_radius=10,
            border_width=1,
            border_color=BORDER,
        )
        footer.grid(row=2, column=0, sticky="ew", padx=12, pady=(6, 12))

        footer.grid_columnconfigure(0, weight=1)
        footer.grid_columnconfigure(1, weight=0)

        left = ctk.CTkFrame(footer, fg_color="transparent")
        left.grid(row=0, column=0, sticky="w", padx=12, pady=10)

        self.invert_checkbox = ctk.CTkCheckBox(
            left,
            text="Invert Pedals",
            text_color=TEXT,
        )
        self.invert_checkbox.pack(side="left", padx=(0, 12))

        self.ffb_checkbox = ctk.CTkCheckBox(
            left,
            text="FFB",
            text_color=TEXT,
        )
        self.ffb_checkbox.pack(side="left")

        right = ctk.CTkFrame(footer, fg_color="transparent")
        right.grid(row=0, column=1, sticky="e", padx=12, pady=10)

        self.save_button = ctk.CTkButton(
            right, text="Save", fg_color=ACCENT, hover_color=ACCENT_SOFT)
        self.save_button.pack(side="left", padx=4)

        self.load_button = ctk.CTkButton(
            right, text="Load", fg_color=BG_ELEMENT, hover_color=ACCENT)
        self.load_button.pack(side="left", padx=4)

        self.reset_button = ctk.CTkButton(
            right, text="Reset Default", fg_color=BG_ELEMENT, hover_color=ACCENT)
        self.reset_button.pack(side="left", padx=4)

    def _bind_callbacks(self) -> None:
        self.switch_button.configure(command=self._on_switch)

        self.max_angle_slider.configure(command=self._on_max_angle_slider)
        self.output_limit_slider.configure(
            command=self._on_output_limit_slider)

        self.recenter_button.configure(command=self._on_recenter)
        self.apply_steering_button.configure(command=self._on_apply_steering)

        self.throttle_offset_slider.configure(
            command=lambda value: self._on_offset_slider_change(
                self.throttle_offset_label, value
            )
        )
        self.brake_offset_slider.configure(
            command=lambda value: self._on_offset_slider_change(
                self.brake_offset_label, value
            )
        )
        self.clutch_offset_slider.configure(
            command=lambda value: self._on_offset_slider_change(
                self.clutch_offset_label, value
            )
        )

        self.throttle_set_min_button.configure(
            command=lambda: self._on_set_pedal("throttle", "min")
        )
        self.throttle_set_max_button.configure(
            command=lambda: self._on_set_pedal("throttle", "max")
        )

        self.brake_set_min_button.configure(
            command=lambda: self._on_set_pedal("brake", "min")
        )
        self.brake_set_max_button.configure(
            command=lambda: self._on_set_pedal("brake", "max")
        )

        self.clutch_set_min_button.configure(
            command=lambda: self._on_set_pedal("clutch", "min")
        )
        self.clutch_set_max_button.configure(
            command=lambda: self._on_set_pedal("clutch", "max")
        )

        self.invert_checkbox.configure(command=self._on_invert_changed)
        self.ffb_checkbox.configure(command=self._on_ffb_changed)

        self.save_button.configure(command=self._on_save)
        self.load_button.configure(command=self._on_load)
        self.reset_button.configure(command=self._on_reset)

    def _on_max_angle_slider(self, value: float) -> None:
        self.max_angle_value.configure(text=str(int(value)))

    def _on_output_limit_slider(self, value: float) -> None:
        self.output_limit_value.configure(text=str(int(value)))

    def _on_offset_slider_change(self, label: ctk.CTkLabel, value: float) -> None:
        label.configure(text=str(int(value)))

    def _on_switch(self) -> None:
        self.status_label.configure(text="Searching...")

        if self.app_state.connected:
            self.controller.disconnect()
            self.app_state.connected = False
            self.app_state.detected_port = None
            self.app_state.status_text = "Disconnect"
            self.switch_button.configure(text="Connect")
            self.status_label.configure(text=self.app_state.status_text)
            return

        try:
            port = self.controller.auto_detect()

            if not port:
                self.app_state.connected = False
                self.app_state.detected_port = None
                self.app_state.status_text = "Device not found"
                self.status_label.configure(text=self.app_state.status_text)
                return

            self.controller.connect(port)
            self.app_state.connected = True
            self.app_state.detected_port = port
            self.app_state.status_text = f"Connected: {port}"
            self.status_label.configure(text=self.app_state.status_text)
            self.switch_button.configure(
                text="Disconnect", hover_color=BG_MAIN)

            self._read_calibration_into_ui()

        except Exception as exc:
            self.app_state.connected = False
            self.app_state.last_error = str(exc)
            self.app_state.status_text = f"Error: {exc}"
            self.status_label.configure(text=self.app_state.status_text)

    def _update_loop(self) -> None:
        if self.controller.is_connected():
            try:
                self.app_state.angle = int(
                    self.controller.telemetry.angle * 3 / 10)
                self.app_state.throttle.current = self.controller.telemetry.throttle
                self.app_state.brake.current = self.controller.telemetry.brake
                self.app_state.clutch.current = self.controller.telemetry.clutch

                self._refresh_live_monitor()

            except Exception as exc:
                self.app_state.last_error = str(exc)
                self.res_label.configure(text=f"Error: {exc}")

        self.after(30, self._update_loop)

    def _refresh_live_monitor(self) -> None:
        throttle = self.app_state.throttle.current / self.app_state.throttle.maximum
        brake = self.app_state.brake.current / self.app_state.brake.maximum
        clutch = self.app_state.clutch.current / self.app_state.clutch.maximum

        self.angle_value_label.configure(text=f"{self.app_state.angle}°")

        self.throttle_slider.set(throttle)
        self.brake_slider.set(brake)
        self.clutch_slider.set(clutch)

        self.throttle_value_label.configure(
            text=str(int(throttle * 100)))
        self.brake_value_label.configure(
            text=str(int(brake * 100)))
        self.clutch_value_label.configure(
            text=str(int(clutch * 100)))

    def _read_calibration_into_ui(self) -> None:
        frame = self.controller.read_calibration()

        if not frame:
            self.res_label.configure(text="Don't loaded config")
            return

        self.app_state.max_angle = frame.max_angle
        self.app_state.output_limit = frame.output_limit
        self.app_state.invert_pedals = frame.invert_pedals
        self.app_state.ffb_enabled = frame.motor_enable

        self.app_state.ffb_enabled = frame.motor_enable

        self.app_state.throttle.minimum = 0
        self.app_state.throttle.maximum = 1023 if frame.throttle_max < 1024 else 4095 if frame.throttle_max < 4096 else 65535

        self.app_state.brake.minimum = 0
        self.app_state.brake.maximum = 1023 if frame.brake_max < 1024 else 4095 if frame.brake_max < 4096 else 65535

        self.app_state.clutch.minimum = 0
        self.app_state.clutch.maximum = 1023 if frame.clutch_max < 1024 else 4095 if frame.clutch_max < 4096 else 65535

        self.max_angle_slider.set(self.app_state.max_angle)
        self.max_angle_value.configure(text=str(self.app_state.max_angle))
        self.output_limit_slider.set(self.app_state.output_limit)
        self.output_limit_value.configure(
            text=str(self.app_state.output_limit))

        self.invert_checkbox.select() if frame.invert_pedals else self.invert_checkbox.deselect()
        self.ffb_checkbox.select() if frame.motor_enable else self.ffb_checkbox.deselect()

    def _on_apply_steering(self) -> None:
        if not self.controller.is_connected():
            return

        try:
            max_angle = int(self.max_angle_slider.get())
            output_limit = int(self.output_limit_slider.get())

            self.controller.apply_steering(max_angle, output_limit)

            self.app_state.max_angle = max_angle
            self.app_state.output_limit = output_limit
            self.res_label.configure(text="Steering applied")

        except Exception as exc:
            self.app_state.last_error = str(exc)
            self.res_label.configure(text=f"Error: {exc}")

    def _on_recenter(self) -> None:
        if not self.controller.is_connected():
            return

        try:
            self.controller.recenter()
            self.res_label.configure(text="Recentered")

        except Exception as exc:
            self.app_state.last_error = str(exc)
            self.res_label.configure(text=f"Error: {exc}")

    def _on_invert_changed(self) -> None:
        if not self.controller.is_connected():
            return

        try:
            enabled = self.invert_checkbox.get() == 1
            self.controller.set_invert_pedals(enabled)
            self.app_state.invert_pedals = enabled
            self.res_label.configure(text="Invert updated")

        except Exception as exc:
            self.app_state.last_error = str(exc)
            self.res_label.configure(text=f"Error: {exc}")

    def _on_ffb_changed(self) -> None:
        if not self.controller.is_connected():
            return

        try:
            enabled = self.ffb_checkbox.get() == 1
            self.controller.set_ffb_enable(enabled)
            self.app_state.ffb_enabled = enabled
            self.res_label.configure(text="FFB updated")

        except Exception as exc:
            self.app_state.last_error = str(exc)
            self.res_label.configure(text=f"Error: {exc}")

    def _get_pedal_current_and_offset(self, pedal_name: str) -> tuple[int, int]:
        pedal_state = getattr(self.app_state, pedal_name)
        slider = getattr(self, f"{pedal_name}_offset_slider")

        current_raw = pedal_state.current
        offset = int(slider.get() * 1023 / 100)

        return current_raw, offset

    def _on_set_pedal(self, pedal_name: str, target: str) -> None:
        if not self.controller.is_connected():
            return

        try:
            current, offset = self._get_pedal_current_and_offset(pedal_name)
            inverted = self.invert_checkbox.get() == 1
            pedal_state = getattr(self.app_state, pedal_name)

            if target == "min":
                value = (current + offset) if not inverted else (current - offset)
            else:
                value = (current - offset) if not inverted else (current + offset)

            value = max(0, min(pedal_state.maximum, value))

            self._apply_pedal_value(pedal_name, target, value)

            self.res_label.configure(
                text=f"{pedal_name.capitalize()} {target} set: {value} (Raw: {current})")

        except Exception as exc:
            self.app_state.last_error = str(exc)
            self.res_label.configure(text=f"Error: {exc}")

    def _apply_pedal_value(self, pedal_name: str, target: str, value: int) -> None:
        methods = {
            "throttle": {"min": self.controller.set_throttle_min, "max": self.controller.set_throttle_max},
            "brake":    {"min": self.controller.set_brake_min,    "max": self.controller.set_brake_max},
            "clutch":   {"min": self.controller.set_clutch_min,   "max": self.controller.set_clutch_max},
        }

        if pedal_name in methods:
            methods[pedal_name][target](value)
        else:
            raise ValueError(f"Unknown pedal: {pedal_name}")

    def _on_save(self) -> None:
        if not self.controller.is_connected():
            return

        try:
            self.controller.save()
            self.res_label.configure(text="Saved to device")

        except Exception as exc:
            self.app_state.last_error = str(exc)
            self.res_label.configure(text=f"Error: {exc}")

    def _on_load(self) -> None:
        if not self.controller.is_connected():
            return

        try:
            self.res_label.configure(text="Loading...")
            self.controller.load()
            self.after(50, self._read_calibration_into_ui)
            self.res_label.configure(text="Loaded from device")

        except Exception as exc:
            self.app_state.last_error = str(exc)
            self.res_label.configure(text=f"Error: {exc}")

    def _on_reset(self) -> None:
        if not self.controller.is_connected():
            return

        try:
            self.controller.reset_default()
            self.after(50, self._read_calibration_into_ui)
            self.res_label.configure(text="Defaults restored")

        except Exception as exc:
            self.app_state.last_error = str(exc)
            self.res_label.configure(text=f"Error: {exc}")
