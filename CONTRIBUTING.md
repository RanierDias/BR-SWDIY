# Contribuindo com o BRSWDIY

Obrigado por contribuir com o projeto.

## Princípios

- priorize clareza antes de complexidade
- mantenha firmware, protocolo e software desacoplados
- trate segurança do motor como requisito principal
- documente decisões que afetem compatibilidade

## Fluxo de branches

Branches principais:

- `main` — estável
- `develop` — integração

Branches de trabalho:

- `feature/...`
- `fix/...`
- `docs/...`
- `refactor/...`

Exemplos:

- `feature/serial-protocol-v1`
- `fix/watchdog-timeout`
- `docs/hardware-wiring`

## Padrão de commits

Use commits curtos e objetivos.

Prefixos sugeridos:

- `feat:`
- `fix:`
- `docs:`
- `refactor:`
- `test:`
- `chore:`

Exemplos:

```text
feat: add hello command to firmware parser
fix: prevent motor enable during boot
docs: describe telemetry response fields
refactor: split parser and response writer
```

## Pull requests

Antes de abrir um pull request:

- garanta que a mudança segue a arquitetura do repositório
- atualize documentação quando a mudança impactar uso ou compatibilidade
- evite misturar refactor grande com feature nova no mesmo PR
- descreva claramente risco, impacto e forma de teste

### Todo PR deve informar

- objetivo da mudança
- áreas afetadas
- risco potencial
- como validar
- se afeta protocolo, firmware, software ou hardware

## Regras por camada

### Protocolo

- mudanças em comandos e respostas exigem atualização em `protocol/`
- compatibilidade deve ser explicitada
- não introduza respostas soltas fora da especificação

### Firmware

- `main.cpp` deve permanecer enxuto
- acesso ao hardware deve ficar em `hw/`
- segurança deve ter prioridade sobre comando externo
- não acople lógica de UI ao firmware

### Software

- a UI não deve acessar a serial diretamente
- a lógica de negócio deve ficar em `core/`
- parsing e validação devem ficar em `protocol/`
- novas telas devem consumir serviços, não transporte bruto

## Estilo

### Firmware

- arquivos pequenos e focados
- nomes claros
- constantes centralizadas
- evitar lógica longa em funções genéricas

### Python

- use type hints quando fizer sentido
- prefira funções pequenas e testáveis
- mantenha separação entre transporte, protocolo e regra de negócio

## Testes

Ao contribuir, sempre que possível:

- adicione testes para parser e comandos no software
- valide handshake e respostas mínimas no firmware
- documente testes manuais quando ainda não houver cobertura automatizada

## Issues

Ao abrir issue, tente incluir:

- ambiente
- hardware utilizado
- passos para reproduzir
- comportamento esperado
- comportamento atual
- logs ou capturas relevantes

## Segurança

Falhas que possam causar:

- torque indevido
- travamento de motor
- habilitação inesperada
- corrupção de configuração crítica

Devem ser tratadas com prioridade.
