from __future__ import annotations

import platform
import shutil
import subprocess
import sys
from pathlib import Path


APP_ID = 'apus'
PACKAGE_NAME = 'apus'
DESCRIPTION = 'BR-SWDIY wheel configuration utility'
MAINTAINER = 'Lótus Azul <lotusazul@users.noreply.github.com>'
SECTION = 'utils'
PRIORITY = 'optional'
HOMEPAGE = 'https://github.com/AdamsGt/BR-SWDIY'
INSTALLED_SIZE_PADDING_KB = 1024


def read_version(pyproject_path: Path) -> str:
    for line in pyproject_path.read_text(encoding='utf-8').splitlines():
        stripped = line.strip()
        if stripped.startswith('version ='):
            return stripped.split('=', 1)[1].strip().strip('"')
    raise RuntimeError('Could not find project version in pyproject.toml')


def map_arch(machine: str) -> str:
    machine = machine.lower()
    if machine in {'x86_64', 'amd64'}:
        return 'amd64'
    if machine in {'aarch64', 'arm64'}:
        return 'arm64'
    if machine.startswith('armv7'):
        return 'armhf'
    return machine


def write_text(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding='utf-8', newline='\n')


def compute_installed_size_kb(path: Path) -> int:
    total = 0
    for child in path.rglob('*'):
        if child.is_file():
            total += child.stat().st_size
    return max(1, (total + 1023) // 1024 + INSTALLED_SIZE_PADDING_KB)


def copy_script_if_exists(src: Path, dst: Path) -> None:
    if src.exists():
        shutil.copy2(src, dst)
        dst.chmod(0o755)


def main() -> int:
    if sys.platform != 'linux':
        print('This build script is intended for Linux (Ubuntu/Debian).', file=sys.stderr)
        return 1

    project_root = Path(__file__).resolve().parent
    output_dir = project_root / 'dist'
    linux_build_dir = output_dir / 'linux'
    pyproject_path = project_root / 'pyproject.toml'
    packaging_dir = project_root / 'packaging' / 'linux'
    desktop_template = packaging_dir / 'apus.desktop'
    icon_png = project_root / 'assets' / 'icon' / 'apus-icon.png'

    version = read_version(pyproject_path)
    arch = map_arch(platform.machine())

    build_result = subprocess.call([sys.executable, str(project_root / 'build_nuitka_linux.py')], cwd=project_root)
    if build_result != 0:
        return build_result

    if not linux_build_dir.exists():
        print('Linux standalone build not found at dist/linux', file=sys.stderr)
        return 1

    deb_root = project_root / 'build' / 'deb' / f'{PACKAGE_NAME}_{version}_{arch}'
    if deb_root.exists():
        shutil.rmtree(deb_root)

    opt_dir = deb_root / 'opt' / APP_ID
    bin_dir = deb_root / 'usr' / 'bin'
    app_dir = deb_root / 'usr' / 'share' / 'applications'
    icon_dir = deb_root / 'usr' / 'share' / 'icons' / 'hicolor' / '256x256' / 'apps'
    doc_dir = deb_root / 'usr' / 'share' / 'doc' / PACKAGE_NAME
    debian_dir = deb_root / 'DEBIAN'

    shutil.copytree(linux_build_dir, opt_dir)
    bin_dir.mkdir(parents=True, exist_ok=True)
    app_dir.mkdir(parents=True, exist_ok=True)
    icon_dir.mkdir(parents=True, exist_ok=True)
    doc_dir.mkdir(parents=True, exist_ok=True)
    debian_dir.mkdir(parents=True, exist_ok=True)

    launcher_path = bin_dir / APP_ID
    launcher_path.symlink_to(Path('/opt') / APP_ID / APP_ID)

    desktop_content = desktop_template.read_text(encoding='utf-8')
    write_text(app_dir / 'apus.desktop', desktop_content)
    shutil.copy2(icon_png, icon_dir / 'apus-icon.png')

    readme_src = project_root / 'README.md'
    if readme_src.exists():
        shutil.copy2(readme_src, doc_dir / 'README.md')

    installed_size = compute_installed_size_kb(deb_root)

    control = f'''Package: {PACKAGE_NAME}
Version: {version}
Section: {SECTION}
Priority: {PRIORITY}
Architecture: {arch}
Maintainer: {MAINTAINER}
Homepage: {HOMEPAGE}
Depends: python3, python3-tk
Installed-Size: {installed_size}
Description: {DESCRIPTION}
 Desktop utility for configuring and monitoring the BRSWDIY / Apus wheel.
 It provides serial-based setup, calibration and live monitoring for the
 BR-SWDIY wheel firmware on Linux desktop environments.
'''
    write_text(debian_dir / 'control', control)

    copy_script_if_exists(packaging_dir / 'postinst', debian_dir / 'postinst')
    copy_script_if_exists(packaging_dir / 'prerm', debian_dir / 'prerm')
    copy_script_if_exists(packaging_dir / 'postrm', debian_dir / 'postrm')

    deb_output = output_dir / f'{PACKAGE_NAME}_{version}_{arch}.deb'
    if deb_output.exists():
        deb_output.unlink()

    subprocess.run(['dpkg-deb', '--build', str(deb_root), str(deb_output)], check=True)

    print('Debian package ready at:', deb_output)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
