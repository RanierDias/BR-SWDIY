# Firmware

Firmware da base FFB do projeto BRSWDIY.

## Objetivo do MVP

Entregar um firmware mínimo capaz de:

- responder handshake
- reportar estado e status
- manter configuração simples
- habilitar e parar motor com segurança
- persistir parâmetros básicos

## Organização

- `src/hw/` — acesso a hardware
- `src/proto/` — parser e respostas serial
- `src/control/` — lógica funcional
- `src/safety/` — watchdog, safe start e falhas
- `src/app/` — orquestração e máquina de estados

## Ambiente

- PlatformIO
- plataforma AVR
- placa inicial: Leonardo
- framework Arduino
