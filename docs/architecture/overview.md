# Arquitetura

## Objetivo

Construir uma solução modular para volante Force Feedback no Windows, com:

- firmware próprio
- protocolo próprio
- biblioteca Python de comunicação
- aplicação gráfica leve
- foco em estabilidade, desempenho e depuração

## Princípios

1. Separar responsabilidades com rigor.
2. O firmware não depende da GUI.
3. A GUI nunca fala com a serial bruta diretamente.
4. Toda comunicação passa por uma camada de protocolo.
5. Segurança do motor.
6. Cada etapa deve ser testável isoladamente, se possível.

## Camadas do projeto

### 1. Firmware embarcado

Responsável por:

- leitura do encoder do volante
- leitura de pedais e botões
- controle do motor
- gerenciamento de estados
- calibração
- persistência de configuração
- protocolo serial
- watchdog e rotinas de segurança

### 2. Protocolo de comunicação

Responsável por:

- identificar versão de hardware e firmware
- padronizar comandos e respostas
- transportar telemetria
- garantir compatibilidade futura
- permitir logs e depuração

### 3. Biblioteca Python

Responsável por:

- detectar porta serial
- conectar e reconectar
- enviar comandos
- validar respostas
- expor API de alto nível
- registrar logs

### 4. Aplicação gráfica

Responsável por:

- interface de configuração
- calibração guiada
- monitoramento em tempo real
- perfis por jogo
- atualização de firmware no futuro

## Estrutura de módulos

### Firmware

- `src/hw/encoder.cpp`
- `src/hw/pedals.cpp`
- `src/hw/buttons.cpp`
- `src/hw/motor.cpp`
- `src/control/ffb_core.cpp`
- `include/config/config_model.cpp`
- `src/proto/serial_protocol.cpp`
- `src/safety/`
- `src/main.cpp`

### Python

- `transport/serial_port.py`
- `protocol/frames.py`
- `protocol/parser.py`
- `core/device.py`
- `core/config.py`
- `core/telemetry.py`
- `profiles/profile_manager.py`
- `ui/app.py`
- `ui/views/*.py`

## Fluxo de comunicação

1. PC abre a porta serial.
2. PC envia `HELLO`.
3. Firmware responde com identificação.
4. Biblioteca valida versão de protocolo.
5. GUI carrega status inicial.
6. Usuário altera parâmetros.
7. Biblioteca envia comando.
8. Firmware confirma com `OK` ou erro.
9. Telemetria é atualizada em paralelo.

## Protocolo inicial

### Estado

- `GET_STATUS`
- `GET_CONFIG`
- `GET_STATE`

### Configuração

- `SET_GAIN <0-100>`
- `SET_DAMPER <0-100>`
- `SET_FRICTION <0-100>`
- `SET_INERTIA <0-100>`
- `SAVE_CONFIG`
- `LOAD_CONFIG`
- `RESET_CONFIG`

### Calibração

- `CALIB_START`
- `CALIB_SET_CENTER <valor>`
- `CALIB_SET_MIN <valor>`
- `CALIB_SET_MAX <valor>`
- `CALIB_FINISH`

### Segurança

- `MOTOR_STOP`
- `MOTOR_ENABLE`
- `SET_SAFE_TORQUE <valor>`
- `SET_WATCHDOG_MS <valor>`

### Diagnóstico

- `GET_TELEMETRY`
- `GET_ERRORS`
- `CLEAR_ERRORS`

## Formato de resposta

### Sucesso

- `OK`
- `OK KEY=VALUE KEY=VALUE`

### Erro

- `ERR CODE=03 MSG=INVALID_RANGE`

### Telemetria

- `STATUS ANGLE=1234 SPEED=12 TORQUE=40 BTN=0 PEDAL1=512 PEDAL2=1010 TEMP=31 FLAGS=0`

## Estratégia de segurança

O firmware deve priorizar:

- desligar torque ao perder comunicação crítica
- limitar torque máximo
- neutralizar motor na inicialização
- bloquear saída em caso de erro de sensor
- expor flags de falha para a GUI

## Estados internos do firmware

- `BOOT`
- `IDLE`
- `READY`
- `CALIBRATION`
- `ACTIVE`
- `FAULT`

## Roadmap de implementação

### Fase 1 — Base funcional

- protocolo serial
- leitura do volante
- leitura de pedais
- configuração persistente
- comandos básicos
- telemetria
- motor teste simples

### Fase 2 — Biblioteca Python

- conexão serial
- parser
- API de alto nível
- logs
- testes de integração

### Fase 3 — GUI leve

- painel principal
- calibração
- monitor em tempo real
- salvar/carregar perfis

### Fase 4 — FFB básico

- gain global
- spring
- damper
- friction
- torque limiter

### Fase 5 — FFB avançado

- clipping monitor
- filtros
- perfis automáticos por jogo
- diagnósticos avançados

## Premissas de hardware do projeto

### Plataforma-base suportada no MVP

- microcontrolador ATmega32u4 em placa compatível com Leonardo/Micro/Pro Micro
- driver de motor BTS7960/IBT-2
- um eixo FFB
- encoder incremental em quadratura
- comunicação USB HID + canal serial de configuração

### Escopo de compatibilidade do motor

O projeto não será ajustado para um motor específico.

A compatibilidade será definida assim:

- o firmware e o software assumem apenas motores DC compatíveis com a faixa prática do BTS7960
- o projeto não cobre motores fora da capacidade elétrica do BTS7960
- o sistema terá limites padrão conservadores de corrente/torque na configuração inicial
- usuários avançados poderão alterar esses limites manualmente depois

### Recursos opcionais que não entram como dependência do MVP

- pinout avançado de enable do BTS7960
- wiring trick pinout
- botão de re-centralização
- LED de clipping FFB
- leitura de corrente por pinos IS

Esses itens podem ser adicionados depois como opções de hardware, mas o núcleo do sistema deve funcionar sem eles.

### Política de segurança padrão

Na ausência de dados específicos do motor, o firmware deve iniciar com perfil conservador:

- torque inicial reduzido
- rampa suave de aplicação
- limite de saída configurável
- motor desabilitado ao ligar até handshake completo
- parada segura ao perder comunicação crítica ou detectar falha

## Tecnologias sugeridas

### Firmware

- C/C++ para microcontrolador
- foco em código enxuto e previsível

### App Windows

- Python 3.11+
- `pyserial`
- `DearPyGui` ou `PySide6`
- `pydantic` opcional para modelos de configuração
- `json` para perfis

## Regras de qualidade

- comandos idempotentes quando possível
- logs legíveis
- erro sempre com código e mensagem
- versionamento explícito do protocolo
- testes por módulo
- nenhuma lógica de negócio solta na interface

## Convenções do projeto

### Branches

- `main` para estável
- `develop` para integração
- `feature/...` para novas funcionalidades
- `fix/...` para correções
- `docs/...` para documentação

### Commits

Padrão sugerido:

- `feat:`
- `fix:`
- `docs:`
- `refactor:`
- `test:`
- `chore:`

### Versionamento

Separar três versões:

- firmware
- software
- protocolo

Exemplo:

- firmware `0.1.0`
- software `0.1.0`
- protocolo `1.0.0`

## Ordem de criação do repositório

### Etapa 1

Criar a estrutura raiz com:

- `README.md`
- `docs/`
- `protocol/`
- `firmware/`
- `software/`
- `hardware/`

### Etapa 2

Adicionar documentação mínima:

- visão geral
- arquitetura
- roadmap
- protocolo v1 em rascunho

### Etapa 3

Subir firmware mínimo com handshake e status.

### Etapa 4

Subir biblioteca Python mínima com conexão serial.

### Etapa 5

Subir GUI mínima.

## Especificação do Protocolo Serial v1

### Objetivos do protocolo

- ser simples de depurar
- funcionar bem via porta serial USB no Windows
- permitir evolução futura sem quebrar tudo
- manter respostas legíveis durante o desenvolvimento inicial

### Estratégia inicial

O protocolo v1 será textual, orientado por linha.

Cada comando enviado pelo PC termina com `
`.
Cada resposta do firmware também termina com `
`.

Formato geral:

```text
COMANDO ARG=VALOR ARG=VALOR
```

Ou, quando fizer mais sentido:

```text
COMANDO VALOR
```

### Regras gerais

- comandos e palavras-chave em maiúsculas
- nomes de campos em maiúsculas
- valores booleanos como `0` ou `1`
- separação por espaço simples
- uma resposta por linha
- toda operação responde com `OK`, `ERR` ou estrutura específica
- firmware nunca envia texto livre não documentado

## Handshake e identificação

### Comando

```text
HELLO
```

### Resposta

```text
HELLO DEVICE=BRBASE PROTO=1 FW=0.1.0 HW=1 AXES=1 MCU=ATMEGA32U4 DRIVER=BTS7960
```

### Regras

- `PROTO` define compatibilidade entre app e firmware
- `FW` é a versão do firmware
- `HW` identifica revisão de hardware suportada
- `AXES` começa em `1`

## Comandos básicos de conectividade

### Obter uptime

```text
GET_UPTIME
```

Resposta:

```text
UPTIME MS=123456
```

## Estado do dispositivo

### Ler estado atual

```text
GET_STATE
```

Resposta:

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

### Campos mínimos do v1

- `ANGLE`
- `CENTER`
- `OUTPUT`
- `LIMIT`
- `PEDAL1`
- `PEDAL2`
- `BTN`
- `FAULT`

Campos opcionais no v1:

- `SPEED`
- `CLIP`
- `PWM_L`
- `PWM_R`
- `TEMP`
- `FLAGS`

## Configuração

### Ler configuração inteira

```text
GET_CONFIG
```

Resposta:

```text
CONFIG GAIN=25 DAMPER=10 FRICTION=5 INERTIA=0 SPRING=0 OUTPUT_LIMIT=25 SAFE_START=1 WATCHDOG_MS=250 DIR_INVERT=0 ENCODER_INVERT=0
```

### Ler limites da plataforma

```text
GET_LIMITS
```

Resposta:

```text
LIMITS GAIN_MIN=0 GAIN_MAX=100 OUTPUT_LIMIT_MIN=0 OUTPUT_LIMIT_MAX=100 WATCHDOG_MIN=50 WATCHDOG_MAX=5000
```

### Definir ganho global

```text
SET_GAIN 25
```

ou

```text
SET_GAIN VALUE=25
```

Resposta:

```text
OK GAIN=25
```

### Definir damper

```text
SET_DAMPER 10
```

Resposta:

```text
OK DAMPER=10
```

### Definir friction

```text
SET_FRICTION 5
```

Resposta:

```text
OK FRICTION=5
```

### Definir inertia

```text
SET_INERTIA 0
```

Resposta:

```text
OK INERTIA=0
```

### Definir spring

```text
SET_SPRING 0
```

Resposta:

```text
OK SPRING=0
```

### Definir limite de saída

```text
SET_OUTPUT_LIMIT 25
```

Resposta:

```text
OK OUTPUT_LIMIT=25
```

### Inverter direção do motor

```text
SET_DIR_INVERT 1
```

Resposta:

```text
OK DIR_INVERT=1
```

### Inverter leitura do encoder

```text
SET_ENCODER_INVERT 0
```

Resposta:

```text
OK ENCODER_INVERT=0
```

### Ativar ou desativar safe start

```text
SET_SAFE_START 1
```

Resposta:

```text
OK SAFE_START=1
```

### Ajustar watchdog

```text
SET_WATCHDOG_MS 250
```

Resposta:

```text
OK WATCHDOG_MS=250
```

## Persistência

### Salvar configuração

```text
SAVE_CONFIG
```

Resposta:

```text
OK SAVED=1
```

### Carregar configuração salva

```text
LOAD_CONFIG
```

Resposta:

```text
OK LOADED=1
```

### Restaurar padrões

```text
RESET_CONFIG
```

Resposta:

```text
OK RESET=1
```

## Segurança

### Habilitar motor

```text
MOTOR_ENABLE
```

Resposta:

```text
OK MOTOR=1
```

### Desabilitar motor

```text
MOTOR_STOP
```

Resposta:

```text
OK MOTOR=0
```

### Ler falhas

```text
GET_ERRORS
```

Resposta:

```text
ERRORS COUNT=0
```

Exemplo com falha:

```text
ERRORS COUNT=2 CODE1=OVER_LIMIT CODE2=WATCHDOG_TIMEOUT
```

### Limpar falhas recuperáveis

```text
CLEAR_ERRORS
```

Resposta:

```text
OK CLEARED=1
```

## Calibração

### Iniciar calibração

```text
CALIB_START
```

Resposta:

```text
OK CALIBRATION=1
```

### Definir centro

```text
CALIB_SET_CENTER 2048
```

Resposta:

```text
OK CENTER=2048
```

### Definir mínimo

```text
CALIB_SET_MIN 0
```

Resposta:

```text
OK MIN=0
```

### Definir máximo

```text
CALIB_SET_MAX 4095
```

Resposta:

```text
OK MAX=4095
```

### Finalizar calibração

```text
CALIB_FINISH
```

Resposta:

```text
OK CALIBRATION=0 CALIBRATED=1
```

## Diagnóstico e desenvolvimento

### Ler versão resumida

```text
GET_INFO
```

Resposta:

```text
INFO DEVICE=BRBASE FW=0.1.0 PROTO=1 HW=1
```

### Loopback de debug

```text
ECHO TESTE
```

Resposta:

```text
ECHO VALUE=TESTE
```

## Erros

### Formato padrão de erro

```text
ERR CODE=3 NAME=INVALID_RANGE FIELD=GAIN
```

### Códigos iniciais

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

## Regras de compatibilidade

- o app deve enviar `HELLO` ao conectar
- o app só opera se `PROTO=1`
- campos extras em respostas devem ser ignorados pelo app se desconhecidos
- comandos desconhecidos devem retornar erro padronizado
- o protocolo textual v1 prioriza clareza, não densidade

## Regras de implementação do firmware

- comandos sem efeito colateral podem ser chamados repetidamente
- comandos de escrita devem validar estado antes de aplicar
- `safety/` pode negar qualquer comando que comprometa proteção
- `MOTOR_ENABLE` só funciona sem falha ativa
- ao ligar, o motor inicia desabilitado

## MVP de protocolo

A implementação mínima do protocolo v1 deve incluir:

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
