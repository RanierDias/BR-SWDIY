from __future__ import annotations

import os
import shutil
import subprocess
import sys
from pathlib import Path


def main() -> int:
    if sys.platform != 'linux':
        print('This build script is intended for Linux (Ubuntu/Debian).', file=sys.stderr)
        return 1

    project_root = Path(__file__).resolve().parent
    src_root = project_root / 'src'
    main_script = src_root / 'brswdiy_app' / 'main.py'
    icon_png = project_root / 'assets' / 'icon' / 'apus-icon.png'
    output_dir = project_root / 'dist'
    build_dir = project_root / 'build' / 'nuitka-linux'
    cache_dir = build_dir / 'cache'
    stage_dir = output_dir / 'linux'

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
        '--linux-icon=' + str(icon_png),
        '--company-name=BRSWDIY',
        '--product-name=Apus Utility',
        '--file-version=0.1.0.0',
        '--product-version=0.1.0.0',
        '--output-filename=apus',
        '--jobs=4',
        '--report=' + str(build_dir / 'nuitka-report.xml'),
        str(main_script),
    ]

    build_dir.mkdir(parents=True, exist_ok=True)
    cache_dir.mkdir(parents=True, exist_ok=True)
    output_dir.mkdir(parents=True, exist_ok=True)

    result = subprocess.call(command, cwd=project_root, env=env)
    if result != 0:
        return result

    built_dir = output_dir / 'main.dist'
    if not built_dir.exists():
        built_dir = output_dir / 'apus.dist'

    if built_dir.exists():
        if stage_dir.exists():
            shutil.rmtree(stage_dir)
        shutil.copytree(built_dir, stage_dir)
        print('Linux build ready at:', stage_dir)
        print('Run with:', stage_dir / 'apus')

    return 0


if __name__ == '__main__':
    raise SystemExit(main())
