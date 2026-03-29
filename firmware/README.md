# Firmware

Firmware da base FFB do projeto BRSWDIY.

**Roadmap Técnico**

Vamos organizar isso como uma trilha de implementação real dentro do seu firmware, para sair do estado atual de bancada e chegar em “volante reconhecido pelo jogo + recebendo FFB + aplicando torque”.

O repositório hoje já tem uma base boa para sensores, config e acionamento local. O que falta é encaixar a pilha de jogo em cima disso, sem misturar tudo no app.cpp.

**Fase 1: Separar as responsabilidades do firmware**

Objetivo: parar de concentrar tudo em app.cpp e preparar o terreno para USB/FFB.

Criar estas áreas novas:

- src/control/
- src/safety/
- src/ffb/
- src/usb/

Sugestão de arquivos:

- src/control/control_loop.h
- src/control/control_loop.cpp
- src/safety/safety_manager.h
- src/safety/safety_manager.cpp
- src/ffb/ffb_types.h
- src/ffb/ffb_effects.h
- src/ffb/ffb_effects.cpp
- src/ffb/ffb_mixer.h
- src/ffb/ffb_mixer.cpp
- src/usb/usb_wheel.h
- src/usb/usb_wheel.cpp

O app.cpp vira só orquestração:

- ler entradas
- atualizar USB
- atualizar efeitos FFB
- rodar safety
- calcular torque
- aplicar no motor

**Fase 2: Fechar o modelo interno do volante**

Objetivo: criar as estruturas que o firmware final vai usar, mesmo antes da USB estar pronta.

Hoje você já tem DeviceStatus e DeviceConfig em:

- telemetry_types.h
- config_model.h

Eu sugiro adicionar:

- WheelInputState
  - angle
  - angular_velocity
  - throttle
  - brake
  - clutch
  - buttons

- FfbDeviceState
  - device_gain
  - ffb_enabled
  - last_ffb_packet_ms
  - host_control_state

- MotorCommand
  - target_torque
  - clamped_output
  - saturated
  - source_flags

- FfbEffectSlot
  - effect_id
  - type
  - enabled
  - gain
  - duration
  - direction
  - magnitude
  - offset
  - phase
  - period
  - coefficient_positive
  - coefficient_negative
  - deadband
  - center

Isso já elimina a dependência do g_output manual atual em app.cpp (line 399).

**Fase 3: Implementar a pipeline de controle**

Objetivo: definir o loop final antes de ligar ao jogo.

Fluxo sugerido para update_app():

1.  sample_inputs()
2.  estimate_wheel_velocity()
3.  usb_wheel_poll()
4.  ffb_effects_update()
5.  safety_update()
6.  control_compute_motor_command()
7.  motor_apply_command()

Aqui vale criar uma função de velocidade angular do volante, porque damper, friction e vários efeitos dependem disso. Hoje o firmware só lê posição em app.cpp (line 109).

**Fase 4: Manter a serial só para debug e calibração**

Objetivo: não confundir protocolo de bancada com protocolo de jogo.

O módulo serial_protocol.cpp deve continuar existindo para:

- handshake de manutenção
- leitura de telemetria
- calibração
- salvar/carregar config
- testes manuais de motor

Mas o comando FFB 0/1 atual deve ser tratado apenas como “enable local”, não como integração com jogo.

Sugestão:

- manter TEL, CAL, SAVE, LOAD, RESET
- adicionar comandos de debug como:
  - DBG FFB
  - DBG FX
  - DBG USB
  - OUT  para teste manual

- não usar mais serial como caminho principal de FFB real

**Fase 5: Implementar o gerenciador de efeitos FFB**

Objetivo: criar o “coração” do FFB.

Arquivos:

- src/ffb/ffb_effects.\*
- src/ffb/ffb_types.h

Esse módulo deve:

- armazenar N slots de efeito
- criar/atualizar efeitos
- iniciar/parar efeito
- liberar bloco
- limpar todos
- aplicar ganho global
- reportar se há efeitos ativos

Comece com estes tipos:

- Constant Force
- Spring
- Damper

Eles já permitem um MVP muito útil. Deixe para depois:

- Sine
- Square
- Triangle
- Sawtooth
- Ramp
- envelopes completos

**Fase 6: Criar o mixer de torque**

Objetivo: converter efeitos ativos em um torque final por ciclo.

Arquivo:

- src/ffb/ffb_mixer.cpp

Funções esperadas:

- compute_constant_force()
- compute_spring_force()
- compute_damper_force()
- mix_all_effects()

Lógica esperada:

- somar contribuições dos efeitos
- aplicar device_gain
- aplicar config.gain
- limitar saída
- respeitar angle_limit
- passar pelo safety antes de acionar motor

Isso substitui a lógica simples atual de update_motor_output() (line 136).

**Fase 7: Fazer o safety sair do papel**

Objetivo: impedir comportamento perigoso quando o host parar, travar ou mandar lixo.

Arquivo:

- src/safety/safety_manager.cpp

Regras mínimas:

- se não chegar FFB por watchdog_ms, zerar torque
- se USB desconectar, zerar torque
- se ffb_enabled == false, zerar torque
- se falha de sensor/encoder, entrar em FAULT
- exigir safe_start antes de ativar torque
- transição real entre READY, ACTIVE e FAULT

Hoje esses conceitos existem mais como config/estado do que como comportamento efetivo.

**Fase 8: Entrar na camada USB HID**

Objetivo: o PC/jogo enxergar o dispositivo como volante real.

Arquivo:

- src/usb/usb_wheel.\*

Aqui está o maior divisor de águas do projeto. Você vai precisar:

- expor eixos e botões via USB HID
- definir um HID report descriptor compatível com wheel + PID/FFB
- receber output reports de FFB do host
- passar esses reports para ffb_effects

Na prática, o fluxo será:

- jogo envia report USB
- usb_wheel recebe
- parser converte report em ação
- ffb_effects atualiza slot
- ffb_mixer calcula torque
- motor aplica

**Fase 9: Implementar o parser dos reports FFB**

Objetivo: ler os “packs FFB” reais do jogo.

Sugestão de funções:

- handle_device_control_report()
- handle_device_gain_report()
- handle_set_effect_report()
- handle_effect_operation_report()
- handle_set_constant_force_report()
- handle_set_condition_report()
- handle_block_free_report()

Ordem recomendada:

1.  Device Gain
2.  Device Control
3.  Set Effect
4.  Effect Operation
5.  Constant Force
6.  Condition para spring/damper

Essa é a menor sequência que já começa a produzir força útil em jogo.

**Fase 10: Evoluir o módulo do motor**

Objetivo: sair de “PWM percentual cru” para um acionamento mais estável.

Hoje motor.cpp faz uma ponte H simples. Para o estágio final, sugiro pelo menos:

- mapa torque -> PWM
- compensação de zona morta
- rampa de subida/descida
- saturação controlada
- freio/zero mais previsível

Se depois houver sensor de corrente, dá para evoluir para controle de torque real. Sem isso, ainda dá para chegar num MVP funcional, mas com menos fidelidade.

**Fase 11: Teste por camadas**

Não testar tudo junto. A sequência ideal é:

1.  Firmware continua compilando com serial/debug.
2.  Volante aparece no PC como HID sem FFB.
3.  Eixos e botões funcionam.
4.  Output reports FFB chegam.
5.  Device Gain altera ganho interno.
6.  Constant Force gera torque.
7.  Spring recentra.
8.  Damper reage à velocidade.
9.  Safety corta torque ao perder host.
10. Teste em jogo real.

**Backlog priorizado**

Ordem que eu seguiria no código:

1.  Refatorar app.cpp para pipeline modular.
2.  Criar tipos internos de FFB e comando de motor.
3.  Implementar ffb_effects e ffb_mixer sem USB ainda.
4.  Fazer teste local via serial para injetar efeitos falsos.
5.  Implementar safety_manager.
6.  Criar camada usb_wheel.
7.  Adicionar HID básico de volante.
8.  Adicionar recepção de reports FFB.
9.  Ligar parser USB ao effect manager.
10. Ajustar motor e tunar resposta.

**Marco de entrega**

Eu dividiria seu projeto em 3 marcos:

- M1Firmware modular, mixer funcionando, testes via serial, sem jogo ainda.
- M2Volante HID reconhecido pelo PC, inputs funcionando, FFB chegando e Constant Force operando.
- M3Spring, damper, watchdog, safety e ajuste fino para jogo real.
