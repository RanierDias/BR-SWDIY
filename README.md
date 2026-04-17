# BRSWDIY - Apus (Alpha)

**Brazil Steering Wheel DIY**

Lightweight and modular force feedback wheel project based on the Arduino Leonardo, designed to run with low overhead and without requiring the desktop app during gameplay.

Projeto de volante com force feedback leve e modular baseado em Arduino Leonardo, projetado para funcionar com baixo overhead e sem depender da GUI durante o uso.

## Aviso / Notice

O projeto BRSWDIY chegou ao fim apos uma sequência de obstaculos que, somados, tornaram sua continuidade inviavel. Desde o inicio, enfrentei problemas relacionados a memoria expansiva na reparticao de disco onde os jogos estavam instalados por um PID extremamente fragil criado pela minha parte, o que gerava inconsistências e dificultava o funcionamento estavel do sistema. Mesmo apos diversas tentativas de contornar essas limitacoes, a falta de experiencia e conhecimento mais aprofundado acabou agravando a situaçao.

Durante esse processo, uma tentativa de ajuste acabou comprometendo o funcionamento do Arduino Leonardo, que passou a apresentar falha na porta USB, impossibilitando qualquer forma de comunicação ou regravação direta. Sem outro Arduino disponivel para realizar um possivel reflash ou recuperação via ISP, o dispositivo ficou inutilizado.

Ao mesmo tempo, ao compreender melhor as limitações da arquitetura utilizada e as dificuldades inerentes ao suporte de force feedback nesse contexto, ficou claro que insistir na mesma plataforma nao seria a melhor decisao. Investir em outro Arduino Leonardo apenas para tentar contornar essas barreiras deixou de fazer sentido. Diante disso, optei por encerrar o projeto nessa base e buscar uma alternativa mais adequada — uma plataforma que lide de forma nativa e mais robusta com os reports de FFB, permitindo uma evolução mais consistente e alinhada com os objetivos iniciais. O código ficará aberto para estudo e para aventureiros que queiram se desafiar e provar que esse limite também podem ser superado, mas infelizmente aqui eu ergo minha bandeira branca por inabilidade e falta de experiência, pois esse foi o meu primeiro projeto embarcado e acho que aprendi muito com ele.

The BRSWDIY project came to an end after a series of obstacles that, combined, made its continuation unfeasible. From the beginning, I faced problems related to the expandable memory in the disk partition where the games were installed due to an extremely fragile PID created by me, which generated inconsistencies and hindered the stable operation of the system. Even after several attempts to overcome these limitations, the lack of experience and more in-depth knowledge ended up aggravating the situation.

During this process, an attempt at adjustment ended up compromising the operation of the Arduino Leonardo, which began to exhibit a failure in the USB port, making any form of communication or direct reprogramming impossible. Without another Arduino available to perform a possible reflash or recovery via ISP, the device became unusable.

At the same time, upon better understanding the limitations of the architecture used and the difficulties inherent in supporting force feedback in this context, it became clear that insisting on the same platform would not be the best decision. Investing in another Arduino Leonardo just to try to overcome these barriers no longer made sense. Given this, I opted to end the project on this basis and seek a more suitable alternative — a platform that natively and more robustly handles FFB reports, allowing for more consistent evolution aligned with the initial objectives. The code will remain open for study and for adventurers who want to challenge themselves and prove that this limit can also be overcome, but unfortunately, here I raise my white flag due to inability and lack of experience, as this was my first embedded project and I believe I learned a lot from it.

## PT-BR

### Visao geral

O **BRSWDIY** e um projeto de volante DIY com force feedback focado em:

- firmware leve e eficiente
- arquitetura simples e modular
- uso plug-and-play
- configuracao opcional por software

Esta versao alfa se chama **Apus**. O nome faz parte do versionamento para projetos alternativos, baseado em constelacoes.

### Objetivos do projeto

- funcionar sem depender da GUI aberta
- manter bom desempenho no Arduino Leonardo
- oferecer configuracao via protocolo serial e utilitario proprio
- permitir evolucao futura sem reescrever toda a base

### Como o projeto esta organizado

- [`firmware/`](firmware/) - firmware do volante
- [`software/`](software/) - GUI `Apus Utility`
- [`hardware/`](hardware/) - diagramas e referencias de montagem

### Diagramas da versao Apus

- Encoder incremental (Versao Apus Icremental Raw/Dir): ![Incremental Encoder Diagram](hardware/diagrams/optical-encoder.png)
- Encoder magnetico (Versao Apus Magnetic Raw/Dir): ![Magnetic Encoder Diagram](hardware/diagrams/magnetic-encoder.png)
- Motor driver e pedais: ![Motor Driver Diagram](hardware/diagrams/motor-driver-and-pedal.png)
- Paddle buttons com `GND + INPUT_PULLUP`: ![Button Diagram](hardware/diagrams/button.png)

### Variantes de firmware

Atualmente o projeto possui quatro variantes de firmware:

- `Raw Incremental`
- `Dir Incremental`
- `Raw Magnetic`
- `Dir Magnetic`

#### Raw

Versao com resposta mais bruta e mais direta, preservando o maximo possivel da sensacao vinda do jogo. E indicada para quem prefere sentir mais textura, vibracao e agressividade no volante, recomendado para uso em cockpit.

#### Dir

Versao mais refinada, com uso de `direction` para polir parte da forca e reduzir ruidos desnecessarios. E voltada para quem prefere uma experiencia mais suave e controlada sem perder fidelidade.

#### Incremental

Voltada para encoders incrementais em quadratura.

#### Magnetic

Voltada para AS5600 via `I2C`.

### Caracteristicas principais

- firmware otimizado para Arduino Leonardo
- dispositivo HID com force feedback
- configuracao persistente em EEPROM
- GUI opcional para calibracao e ajustes
- suporte a filtros como `gain`, `damper`, `friction`, `inertia` e `spring`
- suporte a multiplas variantes de encoder e firmware

### Status

> **Alpha - Apus**

O projeto ja possui:

- FFB funcional
- protocolo serial estavel
- GUI funcional
- builds separadas por variante
- release em `.hex` para firmware e `.exe` para software

Ainda e uma versao alfa e pode receber refinamentos adicionais.

### Pacote de distribuicao

O pacote `.zip` de release e organizado em tres pastas:

- `Xload/`
  Contem a aplicacao do **XLoader**, usada para gravar o firmware `.hex` na placa.

- `Firmware/`
  Contem as quatro variantes do firmware em `.hex`.

- `Software/`
  Contem o executavel `.exe` da GUI **Apus Utility**.

##### [Baixar Pacote](https://github.com/RanierDias/BR-SWDIY/releases/download/v0.2.1-beta/BRSWDIY.zip) / [Conferir Versão](https://github.com/RanierDias/BR-SWDIY/releases)

### Como usar o pacote `.zip`

#### 1. Instalar o firmware

1. Conecte a Arduino Leonardo ao PC.
2. Abra o **XLoader** dentro da pasta `Xload/`.
3. Escolha o arquivo `.hex` desejado dentro da pasta `Firmware/`.
4. Selecione a placa e a porta corretas.
5. Grave o firmware.

#### 2. Usar a GUI

1. Abra a pasta `Software/`.
2. Execute o `.exe` do **Apus Utility**.
3. Conecte ao volante.
4. Ajuste filtros, calibracao e limites conforme necessario.

### Desenvolvimento

Para instrucoes detalhadas de build e desenvolvimento:

- veja [`firmware/README.md`](firmware/README.md)
- veja [`software/README.md`](software/README.md)

### Creditos

Este projeto existe gracas ao trabalho de base de:

- Peter Barrett - HID and USB core for Arduino
- ranenbg - [Arduino-FFB-wheel](https://github.com/ranenbg/Arduino-FFB-wheel)

Muito obrigado por tornarem esse tipo de projeto possivel.

## EN

### Overview

**BRSWDIY** is a DIY force feedback wheel project focused on:

- lightweight and efficient firmware
- a clean modular architecture
- plug-and-play usage
- optional desktop configuration

This alpha version is called **Apus**. The name is part of the versioning for alternative projects, based on constellations.

### Project goals

- work without requiring the GUI to stay open
- keep stable performance on the Arduino Leonardo
- provide configuration through a serial protocol and a dedicated utility
- allow future evolution without rewriting the whole base

### Repository layout

- [`firmware/`](firmware/) - wheel firmware
- [`software/`](software/) - `Apus Utility` desktop app
- [`hardware/`](hardware/) - wiring diagrams and hardware references

### Apus hardware diagrams

- Incremental encoder: ![Incremental Encoder Diagram](hardware/diagrams/optical-encoder.png)
- Magnetic encoder: ![Magnetic Encoder Diagram](hardware/diagrams/magnetic-encoder.png)
- Motor driver and pedals: ![Motor Driver Diagram](hardware/diagrams/motor-driver-and-pedal.png)
- Paddle buttons using `GND + INPUT_PULLUP`: ![Button Diagram](hardware/diagrams/button.png)

### Firmware variants

The project currently ships four firmware variants:

- `Raw Incremental`
- `Dir Incremental`
- `Raw Magnetic`
- `Dir Magnetic`

#### Raw

A more visceral and direct version that preserves as much track texture and game force as possible. Recommended for users who want a more aggressive steering feel. Recommended for use in cockpit simulators!!

#### Dir

A more refined version that uses `direction` to smooth unwanted force noise while keeping a strong and responsive feel.

#### Incremental

Designed for quadrature incremental encoders.

#### Magnetic

Designed for AS5600 over `I2C`.

### Main features

- optimized firmware for Arduino Leonardo
- HID force feedback device
- persistent EEPROM configuration
- optional GUI for calibration and tuning
- support for filters such as `gain`, `damper`, `friction`, `inertia`, and `spring`
- multiple encoder and firmware variants

### Status

> **Alpha - Apus**

The project already includes:

- working FFB
- stable serial protocol
- working GUI
- separate builds per variant
- release artifacts in `.hex` for firmware and `.exe` for software

It is still an alpha release and may receive additional refinement.

### Distribution package

The release `.zip` package is organized into three folders:

- `Xload/`
  Contains the **XLoader** application used to flash the `.hex` firmware to the board.

- `Firmware/`
  Contains the four firmware variants as `.hex` files.

- `Software/`
  Contains the `.exe` build of the **Apus Utility** GUI.

##### [Download Package](https://github.com/RanierDias/BR-SWDIY/releases/download/v0.2.1-beta/BRSWDIY.zip) / [Check Version](https://github.com/RanierDias/BR-SWDIY/releases)

### How to use the `.zip` package

#### 1. Flash the firmware

1. Connect the Arduino Leonardo to the PC.
2. Open **XLoader** from the `Xload/` folder.
3. Choose the desired `.hex` file from the `Firmware/` folder.
4. Select the correct board and serial port.
5. Flash the firmware.

#### 2. Use the GUI

1. Open the `Software/` folder.
2. Run the **Apus Utility** `.exe`.
3. Connect to the wheel.
4. Adjust filters, calibration, and limits as needed.

### Development

For detailed development and build instructions:

- see [`firmware/README.md`](firmware/README.md)
- see [`software/README.md`](software/README.md)

### Credits

This project builds on the work of:

- Peter Barrett - HID and USB core for Arduino
- ranenbg - [Arduino-FFB-wheel](https://github.com/ranenbg/Arduino-FFB-wheel)

Thank you for making projects like this possible.
