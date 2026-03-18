# Compatibility

## Regra principal

O contrato de compatibilidade é guiado pela versão de protocolo.

## Handshake

O firmware deve responder `HELLO` com o campo `PROTO`.

O aplicativo:

- aceita `PROTO=1`
- rejeita versões incompatíveis
- ignora campos extras desconhecidos
