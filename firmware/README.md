# Firmware (PT-BR)

Firmware do volante FFB `Apus` para `Arduino Leonardo`.

O projeto agora possui 4 variantes de build, separando `direction` e tipo de encoder:

- `leonardo_raw_incremental`
- `leonardo_direction_incremental`
- `leonardo_raw_magnetic`
- `leonardo_direction_magnetic`

## Variantes

- `Raw Incremental`
  FFB sem `direction`, para encoder incremental/óptico `A/B`
  Nome USB: `Apus Raw`

- `Direction Incremental`
  FFB com `direction`, para encoder incremental/óptico `A/B`
  Nome USB: `Apus Dir`

- `Raw Magnetic`
  FFB sem `direction`, com suporte a encoder magnético `AS5600`
  Nome USB: `Apus Raw Mag`

- `Direction Magnetic`
  FFB com `direction`, com suporte a encoder magnético `AS5600`
  Nome USB: `Apus Dir Mag`

Separar as variantes evita que a versão incremental carregue o custo de `Wire/I2C` e do suporte ao `AS5600`, o que ajuda bastante no `Arduino Leonardo`.

## Requisitos

- `Python 3`
- `PlatformIO Core`
- `Arduino Leonardo` ou placa compatível com `ATmega32U4`

O core USB customizado do projeto é aplicado automaticamente pelo script [`install_brswdiy_core.py`](/c:/Users/AdamsGt/Desktop/BR-SWDIY/firmware/scripts/install_brswdiy_core.py).

## Estrutura

- [`src/app`](/c:/Users/AdamsGt/Desktop/BR-SWDIY/firmware/src/app): loop principal e configuração
- [`src/ffb`](/c:/Users/AdamsGt/Desktop/BR-SWDIY/firmware/src/ffb): efeitos e mixer de FFB
- [`src/usb`](/c:/Users/AdamsGt/Desktop/BR-SWDIY/firmware/src/usb): HID/PID e USB
- [`src/proto`](/c:/Users/AdamsGt/Desktop/BR-SWDIY/firmware/src/proto): protocolo serial usado pelo software
- [`src/hw`](/c:/Users/AdamsGt/Desktop/BR-SWDIY/firmware/src/hw): encoder, pedais, motor e EEPROM
- [`core/brswdiy_avr_usb_core`](/c:/Users/AdamsGt/Desktop/BR-SWDIY/firmware/core/brswdiy_avr_usb_core): core USB customizado

## Configuração do `platformio.ini`

O arquivo [`platformio.ini`](/c:/Users/AdamsGt/Desktop/BR-SWDIY/firmware/platformio.ini) define:

- `default_envs`
- `upload_port`
- `monitor_port`
- flags de build de cada variante

Trecho atual:

```ini
[platformio]
default_envs = leonardo_raw_incremental

[env]
upload_port = COM19
monitor_port = COM18
```

Você deve ajustar principalmente:

- `upload_port`: porta usada para gravar o firmware
- `monitor_port`: porta do monitor serial
- `default_envs`: variante padrão do seu fluxo local

Para descobrir as portas no Windows:

```powershell
Get-CimInstance Win32_SerialPort | Select-Object DeviceID,Name,PNPDeviceID | Format-Table -AutoSize
```

## Como começar

1. Abra esta pasta no VS Code ou no terminal.
2. Confirme as portas no [`platformio.ini`](/c:/Users/AdamsGt/Desktop/BR-SWDIY/firmware/platformio.ini).
3. Escolha a variante desejada.
4. Rode `build`, `upload` ou `monitor`.

## Comandos principais

Build da variante padrão:

```powershell
platformio run
```

Build por variante:

```powershell
platformio run -e leonardo_raw_incremental
platformio run -e leonardo_direction_incremental
platformio run -e leonardo_raw_magnetic
platformio run -e leonardo_direction_magnetic
```

Upload por variante:

```powershell
platformio run -e leonardo_raw_incremental --target upload
platformio run -e leonardo_direction_incremental --target upload
platformio run -e leonardo_raw_magnetic --target upload
platformio run -e leonardo_direction_magnetic --target upload
```

Monitor serial:

```powershell
platformio device monitor
```

## Release para distribuição

O projeto já possui um fluxo de release que gera os `.hex` prontos para distribuição.

Script Python:

```powershell
python -m scripts.build_release
```

Atalho PowerShell:

```powershell
.\build_release.ps1
```

Esse fluxo:

- compila as 4 variantes
- copia os artefatos para [`dist/`](/c:/Users/AdamsGt/Desktop/BR-SWDIY/firmware/dist)

Exemplos de nomes gerados:

- `apus-raw-incremental-leonardo-v0.1.0.hex`
- `apus-dir-incremental-leonardo-v0.1.0-direction.hex`
- `apus-raw-magnetic-leonardo-v0.1.0-mag.hex`
- `apus-dir-magnetic-leonardo-v0.1.0-direction-mag.hex`

## Onde ficam os artefatos

Artefatos do PlatformIO:

- [`.pio/build/leonardo_raw_incremental/firmware.hex`](/c:/Users/AdamsGt/Desktop/BR-SWDIY/firmware/.pio/build/leonardo_raw_incremental/firmware.hex)
- [`.pio/build/leonardo_direction_incremental/firmware.hex`](/c:/Users/AdamsGt/Desktop/BR-SWDIY/firmware/.pio/build/leonardo_direction_incremental/firmware.hex)
- [`.pio/build/leonardo_raw_magnetic/firmware.hex`](/c:/Users/AdamsGt/Desktop/BR-SWDIY/firmware/.pio/build/leonardo_raw_magnetic/firmware.hex)
- [`.pio/build/leonardo_direction_magnetic/firmware.hex`](/c:/Users/AdamsGt/Desktop/BR-SWDIY/firmware/.pio/build/leonardo_direction_magnetic/firmware.hex)

Artefatos de release:

- [`dist/`](/c:/Users/AdamsGt/Desktop/BR-SWDIY/firmware/dist)

## Distribuição para usuários finais

Distribua os arquivos `.hex` usando ferramentas como:

- `XLoader`
- `XLoader++`
- `avrdude`
- `PlatformIO`

Ao distribuir, informe:

- placa alvo: `Arduino Leonardo`
- variante: incremental ou magnética
- estilo de FFB: `Raw` ou `Direction`
- versão do firmware

## Observações

- `encoder_ppr` continua existindo para encoder incremental
- as variantes magnéticas habilitam o suporte ao `AS5600`
- o software de configuração usa o protocolo serial em [`serial_protocol.cpp`](/c:/Users/AdamsGt/Desktop/BR-SWDIY/firmware/src/proto/serial_protocol.cpp)

---

# Firmware (EN)

`Apus` FFB wheel firmware for `Arduino Leonardo`.

The project now has 4 build variants, separated by `direction` mode and encoder type:

- `leonardo_raw_incremental`
- `leonardo_direction_incremental`
- `leonardo_raw_magnetic`
- `leonardo_direction_magnetic`

## Variants

- `Raw Incremental`
  FFB without `direction`, for incremental/optical `A/B` encoder
  USB name: `Apus Raw`

- `Direction Incremental`
  FFB with `direction`, for incremental/optical `A/B` encoder
  USB name: `Apus Dir`

- `Raw Magnetic`
  FFB without `direction`, with `AS5600` magnetic encoder support
  USB name: `Apus Raw Mag`

- `Direction Magnetic`
  FFB with `direction`, with `AS5600` magnetic encoder support
  USB name: `Apus Dir Mag`

Separating these variants prevents the incremental build from paying the `Wire/I2C` memory cost of `AS5600` support, which is especially important on `Arduino Leonardo`.

## Requirements

- `Python 3`
- `PlatformIO Core`
- `Arduino Leonardo` or compatible `ATmega32U4` board

The project custom USB core is applied automatically by [`install_brswdiy_core.py`](/c:/Users/AdamsGt/Desktop/BR-SWDIY/firmware/scripts/install_brswdiy_core.py).

## Structure

- [`src/app`](/c:/Users/AdamsGt/Desktop/BR-SWDIY/firmware/src/app): main loop and configuration
- [`src/ffb`](/c:/Users/AdamsGt/Desktop/BR-SWDIY/firmware/src/ffb): FFB effects and mixer
- [`src/usb`](/c:/Users/AdamsGt/Desktop/BR-SWDIY/firmware/src/usb): HID/PID and USB
- [`src/proto`](/c:/Users/AdamsGt/Desktop/BR-SWDIY/firmware/src/proto): serial protocol used by the PC software
- [`src/hw`](/c:/Users/AdamsGt/Desktop/BR-SWDIY/firmware/src/hw): encoder, pedals, motor, and EEPROM
- [`core/brswdiy_avr_usb_core`](/c:/Users/AdamsGt/Desktop/BR-SWDIY/firmware/core/brswdiy_avr_usb_core): custom USB core

## `platformio.ini` setup

[`platformio.ini`](/c:/Users/AdamsGt/Desktop/BR-SWDIY/firmware/platformio.ini) defines:

- `default_envs`
- `upload_port`
- `monitor_port`
- build flags for each variant

Current example:

```ini
[platformio]
default_envs = leonardo_raw_incremental

[env]
upload_port = COM19
monitor_port = COM18
```

You should mainly adjust:

- `upload_port`: port used to flash the firmware
- `monitor_port`: serial monitor port
- `default_envs`: your default local variant

To discover ports on Windows:

```powershell
Get-CimInstance Win32_SerialPort | Select-Object DeviceID,Name,PNPDeviceID | Format-Table -AutoSize
```

## How to start

1. Open this folder in VS Code or in a terminal.
2. Confirm the ports in [`platformio.ini`](/c:/Users/AdamsGt/Desktop/BR-SWDIY/firmware/platformio.ini).
3. Choose the variant you want.
4. Run build, upload, or monitor.

## Main commands

Build the default variant:

```powershell
platformio run
```

Build by variant:

```powershell
platformio run -e leonardo_raw_incremental
platformio run -e leonardo_direction_incremental
platformio run -e leonardo_raw_magnetic
platformio run -e leonardo_direction_magnetic
```

Upload by variant:

```powershell
platformio run -e leonardo_raw_incremental --target upload
platformio run -e leonardo_direction_incremental --target upload
platformio run -e leonardo_raw_magnetic --target upload
platformio run -e leonardo_direction_magnetic --target upload
```

Serial monitor:

```powershell
platformio device monitor
```

## Release distribution

The project already includes a release flow that generates the `.hex` files ready for distribution.

Python script:

```powershell
python -m scripts.build_release
```

PowerShell shortcut:

```powershell
.\build_release.ps1
```

This flow:

- builds all 4 variants
- copies the artifacts to [`dist/`](/c:/Users/AdamsGt/Desktop/BR-SWDIY/firmware/dist)

Example generated names:

- `apus-raw-incremental-leonardo-v0.1.0.hex`
- `apus-dir-incremental-leonardo-v0.1.0-direction.hex`
- `apus-raw-magnetic-leonardo-v0.1.0-mag.hex`
- `apus-dir-magnetic-leonardo-v0.1.0-direction-mag.hex`

## Where artifacts are generated

PlatformIO artifacts:

- [`.pio/build/leonardo_raw_incremental/firmware.hex`](/c:/Users/AdamsGt/Desktop/BR-SWDIY/firmware/.pio/build/leonardo_raw_incremental/firmware.hex)
- [`.pio/build/leonardo_direction_incremental/firmware.hex`](/c:/Users/AdamsGt/Desktop/BR-SWDIY/firmware/.pio/build/leonardo_direction_incremental/firmware.hex)
- [`.pio/build/leonardo_raw_magnetic/firmware.hex`](/c:/Users/AdamsGt/Desktop/BR-SWDIY/firmware/.pio/build/leonardo_raw_magnetic/firmware.hex)
- [`.pio/build/leonardo_direction_magnetic/firmware.hex`](/c:/Users/AdamsGt/Desktop/BR-SWDIY/firmware/.pio/build/leonardo_direction_magnetic/firmware.hex)

Release artifacts:

- [`dist/`](/c:/Users/AdamsGt/Desktop/BR-SWDIY/firmware/dist)

## Distribution to end users

Distribute the `.hex` files with tools such as:

- `XLoader`
- `XLoader++`
- `avrdude`
- `PlatformIO`

When distributing, make sure to specify:

- target board: `Arduino Leonardo`
- encoder family: incremental or magnetic
- FFB style: `Raw` or `Direction`
- firmware version

## Notes

- `encoder_ppr` still exists for incremental encoders
- magnetic variants enable `AS5600` support
- the desktop configuration software talks to [`serial_protocol.cpp`](/c:/Users/AdamsGt/Desktop/BR-SWDIY/firmware/src/proto/serial_protocol.cpp)
