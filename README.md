# BRSWDIY

BRSWDIY é uma plataforma open source para bases de volante Force Feedback no Windows, com foco em estabilidade, baixo consumo de recursos e arquitetura modular.

## Objetivo

Construir uma solução completa e limpa, composta por:

- firmware próprio para microcontroladores baseados em ATmega32u4
- protocolo serial próprio para configuração e diagnóstico
- aplicativo desktop em Python para Windows
- documentação clara para hardware, software e evolução do projeto

## Escopo inicial

O MVP é centrado em uma base de 1 eixo com:

- ATmega32u4 (Leonardo, Micro ou Pro Micro compatível)
- driver BTS7960 / IBT-2
- encoder incremental em quadratura
- motor DC compatível com a faixa prática do BTS7960
- USB HID + canal serial de configuração

## Princípios do projeto

- firmware e interface são desacoplados
- a GUI nunca acessa a serial diretamente
- o protocolo define o contrato oficial entre firmware e aplicativo
- segurança do motor tem prioridade sobre qualquer comando
- recursos opcionais não bloqueiam o funcionamento do núcleo
- o sistema deve nascer simples de depurar e evoluir

## Componentes principais

### `protocol/`

Documentação oficial do protocolo serial e regras de compatibilidade.

### `firmware/`

Firmware da base FFB, organizado por módulos de hardware, protocolo, controle e segurança.

### `software/`

Aplicativo desktop em Python para configuração, diagnóstico, telemetria e perfis.

### `hardware/`

Referências de pinagem, diagramas e recursos opcionais.

### `docs/`

Arquitetura, roadmap, decisões de projeto e documentação técnica de apoio.

## Requisitos de desenvolvimento

### Firmware

- PlatformIO
- C/C++

### Software

- Python 3.11+
- pyserial
- DearPyGui ou PySide6

## Segurança

Ao ligar:

- o motor começa desabilitado
- torque inicial é conservador
- watchdog e limites de saída devem ser respeitados
- o firmware pode negar comandos inseguros

## Começando

### Protocolo

Consulte `protocol/serial-v1.md`.

### Firmware

Consulte `firmware/README.md`.

### Software

Consulte `software/README.md`.
