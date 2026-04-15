from __future__ import annotations

import customtkinter as ctk


BG_MAIN = "#0b0f1a"
BG_PANEL = "#121829"
BG_ELEMENT = "#1a2238"

ACCENT = "#3b82f6"
ACCENT_SOFT = "#0CC0AB"

TEXT = "#e5e7eb"
TEXT_DIM = "#9ca3af"

BORDER = "#1f2a44"


def setup_theme() -> None:
    ctk.set_appearance_mode("dark")
    ctk.set_default_color_theme("blue")
