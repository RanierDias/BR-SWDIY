from __future__ import annotations

import re
import shutil
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
PLATFORMIO_INI = ROOT / "platformio.ini"
DIST_DIR = ROOT / "dist"
PLATFORMIO_EXE = Path.home() / ".platformio" / "penv" / "Scripts" / "platformio.exe"

ENVIRONMENTS = {
    "leonardo_raw_incremental": "raw-incremental",
    "leonardo_direction_incremental": "dir-incremental",
    "leonardo_raw_magnetic": "raw-magnetic",
    "leonardo_direction_magnetic": "dir-magnetic",
}


def read_versions() -> dict[str, str]:
    content = PLATFORMIO_INI.read_text(encoding="utf-8")
    versions: dict[str, str] = {}

    for env_name in ENVIRONMENTS:
        pattern = (
            rf"\[env:{re.escape(env_name)}\][^\[]*?"
            rf'BRSWDIY_FW_VERSION=\\"([^"]+)\\"'
        )
        match = re.search(pattern, content, re.DOTALL)
        if not match:
            raise RuntimeError(f"Could not find BRSWDIY_FW_VERSION for {env_name} in platformio.ini")
        versions[env_name] = match.group(1)

    return versions


def build_environment(env_name: str) -> None:
    subprocess.run(
        [str(PLATFORMIO_EXE), "run", "-e", env_name],
        cwd=ROOT,
        check=True,
    )


def copy_release_hex(env_name: str, variant_name: str, version: str) -> Path:
    source = ROOT / ".pio" / "build" / env_name / "firmware.hex"
    if not source.exists():
        raise FileNotFoundError(f"Build artifact not found: {source}")

    DIST_DIR.mkdir(parents=True, exist_ok=True)
    target = DIST_DIR / f"apus-{variant_name}-leonardo-v{version}.hex"
    shutil.copy2(source, target)
    return target


def main() -> int:
    if not PLATFORMIO_EXE.exists():
        raise FileNotFoundError(f"PlatformIO executable not found: {PLATFORMIO_EXE}")

    versions = read_versions()
    outputs: list[Path] = []

    for env_name, variant_name in ENVIRONMENTS.items():
        build_environment(env_name)
        outputs.append(copy_release_hex(env_name, variant_name, versions[env_name]))

    print("Release artifacts:")
    for output in outputs:
        print(output)

    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:  # pragma: no cover
        print(f"release build failed: {exc}", file=sys.stderr)
        raise SystemExit(1)
