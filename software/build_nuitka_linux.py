from __future__ import annotations

import os
import subprocess
import sys
import tomllib
from pathlib import Path


def load_version(pyproject_path: Path) -> str:
    with pyproject_path.open('rb') as f:
        data = tomllib.load(f)
    return data['project']['version']


def main() -> int:
    project_root = Path(__file__).resolve().parent
    src_root = project_root / 'src'
    main_script = src_root / 'brswdiy_app' / 'main.py'
    icon_png = project_root / 'assets' / 'icon' / 'apus-icon.png'
    output_dir = project_root / 'dist' / 'linux'
    build_dir = project_root / 'build' / 'nuitka-linux'
    cache_dir = build_dir / 'cache'
    version = load_version(project_root / 'pyproject.toml')

    env = os.environ.copy()
    env['PYTHONPATH'] = str(src_root) + os.pathsep + env.get('PYTHONPATH', '')
    env['NUITKA_CACHE_DIR'] = str(cache_dir)

    command = [
        sys.executable,
        '-m',
        'nuitka',
        '--standalone',
        '--enable-plugin=tk-inter',
        '--assume-yes-for-downloads',
        '--output-dir=' + str(output_dir),
        '--remove-output',
        '--nofollow-import-to=pytest,setuptools,wheel,black,ruff',
        '--include-data-file=' + str(icon_png) + '=assets/icon/apus-icon.png',
        '--company-name=BRSWDIY',
        '--product-name=Apus Utility',
        '--file-version=' + version,
        '--product-version=' + version,
        '--output-filename=apus',
        '--jobs=4',
        '--report=' + str(build_dir / 'nuitka-report.xml'),
        str(main_script),
    ]

    build_dir.mkdir(parents=True, exist_ok=True)
    cache_dir.mkdir(parents=True, exist_ok=True)
    output_dir.mkdir(parents=True, exist_ok=True)

    return subprocess.call(command, cwd=project_root, env=env)


if __name__ == '__main__':
    raise SystemExit(main())
