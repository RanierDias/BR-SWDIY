from __future__ import annotations

import os
import shutil
import stat
import subprocess
import sys
import tomllib
from pathlib import Path


def load_version(pyproject_path: Path) -> str:
    with pyproject_path.open('rb') as f:
        data = tomllib.load(f)
    return data['project']['version']


def detect_arch() -> str:
    try:
        result = subprocess.run(
            ['dpkg', '--print-architecture'],
            check=True,
            capture_output=True,
            text=True,
        )
        arch = result.stdout.strip()
        if arch:
            return arch
    except Exception:
        pass

    machine = os.uname().machine.lower()
    mapping = {
        'x86_64': 'amd64',
        'amd64': 'amd64',
        'aarch64': 'arm64',
        'arm64': 'arm64',
        'armv7l': 'armhf',
    }
    return mapping.get(machine, machine)


def copy_tree(src: Path, dst: Path) -> None:
    if dst.exists():
        shutil.rmtree(dst)
    shutil.copytree(src, dst)


def write_executable(path: Path, content: str) -> None:
    path.write_text(content, encoding='utf-8', newline='\n')
    mode = path.stat().st_mode
    path.chmod(mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)


def main() -> int:
    project_root = Path(__file__).resolve().parent
    version = load_version(project_root / 'pyproject.toml')
    arch = detect_arch()

    subprocess.run([sys.executable, str(project_root / 'build_nuitka_linux.py')], check=True)

    dist_root = project_root / 'dist'
    linux_dist = dist_root / 'linux'
    app_dist = linux_dist / 'apus.dist'
    if not app_dist.exists():
        raise FileNotFoundError(f'Nuitka standalone output not found: {app_dist}')

    build_root = project_root / 'build' / 'deb'
    pkg_root = build_root / 'apus'
    debian_dir = pkg_root / 'DEBIAN'
    opt_dir = pkg_root / 'opt' / 'apus'
    bin_dir = pkg_root / 'usr' / 'bin'
    app_dir = pkg_root / 'usr' / 'share' / 'applications'
    icon_dir = pkg_root / 'usr' / 'share' / 'icons' / 'hicolor' / '256x256' / 'apps'
    doc_dir = pkg_root / 'usr' / 'share' / 'doc' / 'apus'

    if pkg_root.exists():
        shutil.rmtree(pkg_root)

    for path in [debian_dir, opt_dir.parent, bin_dir, app_dir, icon_dir, doc_dir]:
        path.mkdir(parents=True, exist_ok=True)

    copy_tree(app_dist, opt_dir)

    launcher = "#!/usr/bin/env sh\nexec /opt/apus/apus \"$@\"\n"
    write_executable(bin_dir / 'apus', launcher)

    shutil.copy2(project_root / 'packaging' / 'linux' / 'apus.desktop', app_dir / 'apus.desktop')
    shutil.copy2(project_root / 'assets' / 'icon' / 'apus-icon.png', icon_dir / 'apus-icon.png')
    shutil.copy2(project_root / 'README.md', doc_dir / 'README.md')

    control = f"""Package: apus
Version: {version}
Section: utils
Priority: optional
Architecture: {arch}
Maintainer: AdamsGt <adamsgt@users.noreply.github.com>
Depends: python3, python3-tk
Installed-Size: 20480
Homepage: https://github.com/AdamsGt/BR-SWDIY
Description: Apus Utility for BRSWDIY wheels
 Desktop utility for configuring and monitoring the BRSWDIY / Apus wheel.
"""
    (debian_dir / 'control').write_text(control, encoding='utf-8', newline='\n')

    for script_name in ['postinst', 'prerm', 'postrm']:
        src = project_root / 'packaging' / 'linux' / script_name
        dst = debian_dir / script_name
        shutil.copy2(src, dst)
        mode = dst.stat().st_mode
        dst.chmod(mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)

    output_deb = dist_root / f'apus_{version}_{arch}.deb'
    if output_deb.exists():
        output_deb.unlink()

    subprocess.run(['dpkg-deb', '--build', str(pkg_root), str(output_deb)], check=True)
    print(output_deb)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
