# Apus Utility

Aplicativo desktop para configurar e monitorar o volante BRSWDIY / Apus.

Desktop utility for configuring and monitoring the BRSWDIY / Apus wheel.

## PT-BR

### Requisitos

- Python 3.11 ou superior
- Windows
- `pip` ou `poetry`
- Firmware compatível gravado no dispositivo

### Estrutura do projeto

- `src/brswdiy_app/` - codigo-fonte do aplicativo
- `assets/` - icones e recursos visuais
- `build_nuitka.py` - script de build do executavel
- `build_nuitka.ps1` - atalho PowerShell para build

### Instalar dependencias

Com `pip`:

```powershell
python -m venv .venv
.\.venv\Scripts\activate
pip install -e .
pip install -e .[dev]
```

Com `poetry`:

```powershell
poetry install --with dev
```

### Rodar em desenvolvimento

Via entrypoint do projeto:

```powershell
poetry run apus
```

Ou direto pelo Python:

```powershell
poetry run python -m src.brswdiy_app.main
```

Se estiver usando apenas `venv`:

```powershell
python -m src.brswdiy_app.main
```

### Build com Nuitka

Script Python:

```powershell
poetry run python -m build_nuitka
```

Atalho PowerShell:

```powershell
.\build_nuitka.ps1
```

O build gera:

- executavel em `dist/`
- arquivos intermediarios em `build/nuitka/`
- relatorio em `build/nuitka/nuitka-report.xml`

### Comandos uteis

Formatar com Black:

```powershell
poetry run black .\src
```

Lint com Ruff:

```powershell
poetry run ruff check .\src
```

Testes:

```powershell
poetry run pytest
```

### Observações

- O app usa `customtkinter` e `pyserial`.
- O build do Nuitka inclui o icone do app automaticamente a partir de `assets/icon/`.
- Se o dispositivo não conectar automaticamente, confira a porta serial e o firmware gravado.

## EN

### Requirements

- Python 3.11 or newer
- Windows
- `pip` or `poetry`
- Compatible firmware flashed to the device

### Project structure

- `src/brswdiy_app/` - application source code
- `assets/` - icons and visual assets
- `build_nuitka.py` - executable build script
- `build_nuitka.ps1` - PowerShell shortcut for builds

### Install dependencies

With `pip`:

```powershell
python -m venv .venv
.\.venv\Scripts\activate
pip install -e .
pip install -e .[dev]
```

With `poetry`:

```powershell
poetry install --with dev
```

### Run in development

Using the project entrypoint:

```powershell
poetry run apus
```

Or directly with Python:

```powershell
poetry run python -m src.brswdiy_app.main
```

If you are using a plain `venv`:

```powershell
python -m src.brswdiy_app.main
```

### Build with Nuitka

Python script:

```powershell
poetry run python -m build_nuitka
```

PowerShell shortcut:

```powershell
.\build_nuitka.ps1
```

The build outputs:

- executable in `dist/`
- intermediate files in `build/nuitka/`
- report in `build/nuitka/nuitka-report.xml`

### Useful commands

Format with Black:

```powershell
poetry run black .\src
```

Lint with Ruff:

```powershell
poetry run ruff check .\src
```

Run tests:

```powershell
poetry run pytest
```

### Notes

- The app uses `customtkinter` and `pyserial`.
- The Nuitka build automatically bundles the app icon from `assets/icon/`.
- If the device does not auto-connect, verify the serial port and flashed firmware.

## Linux (Ubuntu/Debian)

### Requirements

- Python 3.11 or newer
- `python3-tk`
- `pip` or `poetry`
- `patchelf`
- `gcc`, `g++`, `make`
- Compatible firmware flashed to the device

Suggested packages:

```bash
sudo apt update
sudo apt install python3 python3-pip python3-venv python3-tk build-essential patchelf
```

Optional serial access fix:

```bash
sudo usermod -aG dialout $USER
```

Then log out and back in.

### Run in development

With `poetry`:

```bash
poetry install --with dev
poetry run apus
```

Or directly with Python:

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -e .
python3 -m brswdiy_app.main
```

### Build with Nuitka on Linux

Python script:

```bash
python3 ./build_nuitka_linux.py
```

Shell shortcut:

```bash
bash ./build_nuitka_linux.sh
```

The Linux build outputs:

- staged app directory in `dist/linux/`
- intermediate files in `build/nuitka-linux/`
- report in `build/nuitka-linux/nuitka-report.xml`

Run the packaged app with:

```bash
./dist/linux/apus
```

A basic desktop entry template is available at:

- `packaging/linux/apus.desktop`

### Build Debian package (.deb)

Python script:

```bash
python3 ./build_deb_linux.py
```

Shell shortcut:

```bash
bash ./build_deb_linux.sh
```

The Debian package output is generated at:

- `dist/apus_<version>_<arch>.deb`

Install with:

```bash
sudo dpkg -i ./dist/apus_<version>_<arch>.deb
```

If Ubuntu/Debian reports missing dependencies:

```bash
sudo apt -f install
```

The Debian package installs:

- the app bundle in `/opt/apus`
- a launcher symlink at `/usr/bin/apus`
- a desktop entry in `/usr/share/applications/apus.desktop`
- the app icon in `/usr/share/icons/hicolor/256x256/apps/apus-icon.png`

It also refreshes desktop and icon caches on install/remove when the host system provides the standard tools.
