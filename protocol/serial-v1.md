# BRSWDIY — Serial Protocol v1

## Visão geral

Este documento define o protocolo serial v1 usado entre:

- firmware da base
- aplicativo desktop

O protocolo é textual, orientado por linha e fácil de depurar.

## Transporte

- USB Serial (COM)
- codificação ASCII
- cada mensagem termina com `\n`

## Formato geral

### Comandos

```text
COMANDO ARG=VALOR ARG=VALOR
```

ou

```text
COMANDO VALOR
```

### Respostas

```text
OK
OK KEY=VALUE
ERR CODE=... NAME=...
STATUS ...
CONFIG ...
```

## Regras gerais

- comandos em maiúsculas
- campos em maiúsculas
- booleanos como `0` ou `1`
- uma resposta por comando
- firmware não deve enviar texto fora da especificação

## Handshake

### Requisição

```text
HELLO
```

### Resposta

```text
HELLO DEVICE=BRBASE PROTO=1 FW=0.1.0 HW=1 AXES=1 MCU=ATMEGA32U4 DRIVER=BTS7960
```

## Estado do dispositivo

### Comando

```text
GET_STATE
```

### Resposta

```text
STATE NAME=READY MOTOR=0 FAULT=0 CALIBRATED=1 CONFIG_SAVED=1
```

### Estados válidos

- `BOOT`
- `IDLE`
- `READY`
- `CALIBRATION`
- `ACTIVE`
- `FAULT`

## Telemetria

### Status resumido

```text
GET_STATUS
```

Resposta:

```text
STATUS STATE=READY ANGLE=1234 CENTER=2048 OUTPUT=0 LIMIT=25 PEDAL1=0 PEDAL2=0 BTN=0 FAULT=0
```

### Telemetria detalhada

```text
GET_TELEMETRY
```

Resposta:

```text
TELEMETRY ANGLE=1234 SPEED=12 OUTPUT=8 CLIP=0 PWM_L=0 PWM_R=8 PEDAL1=0 PEDAL2=0 BTN=0 TEMP=0 FLAGS=0
```

## Configuração

### Ler configuração

```text
GET_CONFIG
```

Resposta:

```text
CONFIG GAIN=25 DAMPER=10 FRICTION=5 INERTIA=0 SPRING=0 OUTPUT_LIMIT=25 SAFE_START=1 WATCHDOG_MS=250 DIR_INVERT=0 ENCODER_INVERT=0
```

### Comandos mínimos do MVP

- `HELLO`
- `PING`
- `GET_INFO`
- `GET_STATE`
- `GET_STATUS`
- `GET_CONFIG`
- `SET_GAIN`
- `SET_OUTPUT_LIMIT`
- `SET_SAFE_START`
- `SET_WATCHDOG_MS`
- `SAVE_CONFIG`
- `LOAD_CONFIG`
- `MOTOR_ENABLE`
- `MOTOR_STOP`
- `GET_ERRORS`
- `CLEAR_ERRORS`

## Erros

Formato:

```text
ERR CODE=3 NAME=INVALID_RANGE FIELD=GAIN
```

Códigos iniciais:

- `1` `UNKNOWN_COMMAND`
- `2` `INVALID_ARGUMENT`
- `3` `INVALID_RANGE`
- `4` `INVALID_STATE`
- `5` `MOTOR_DISABLED`
- `6` `NOT_CALIBRATED`
- `7` `EEPROM_ERROR`
- `8` `WATCHDOG_TIMEOUT`
- `9` `FAULT_ACTIVE`
- `10` `NOT_SUPPORTED`

## Compatibilidade

- o aplicativo deve enviar `HELLO` ao conectar
- o aplicativo deve validar `PROTO=1`
- campos extras devem ser ignorados quando desconhecidos
- comandos desconhecidos devem retornar erro padronizado
- o motor deve iniciar desabilitado
