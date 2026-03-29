# BRSWDIY — Apus (Alpha)

**Brasil Steering Wheel DIY**

Uma implementação leve, eficiente e modular de volante com force feedback baseada em Arduino Leonardo, projetada para funcionar de forma simples, confiável e sem dependência de software em tempo real.

## ✨ Visão Geral

O **BRSWDIY** é um projeto que reimagina o conceito do Arduino FFB tradicional com foco em:

- **Eficiência real de firmware**
- **Baixo consumo de recursos**
- **Arquitetura limpa e modular**
- **Experiência plug-and-play**

Esta versão alfa, chamada **Apus**, marca o início de uma linha evolutiva onde cada versão derivada recebe o nome de uma constelação.

## 🎯 Objetivo

O objetivo do BRSWDIY é oferecer um sistema de volante com force feedback que:

- Funcione de forma **independente da interface gráfica**
- Seja **leve o suficiente para rodar com estabilidade no Arduino Leonardo**
- Permita **controle total via firmware e protocolo serial**
- Seja **facilmente modificável e expansível** para outros projetos

## 🔌 Plug-and-Play de Verdade

Diferente de outras implementações:

> ⚡ O volante funciona completamente sem a GUI aberta.

- A GUI **não é necessária para uso**
- O dispositivo se comporta como um **HID pronto para uso**
- Valoriza os jogadores que estão começando no mundo competitivo
- Respostas a 1ms com uma constância maior

A interface **Apus Utility** existe apenas para:

- Ajustar ganho e limites
- Configurar filtros
- Calibrar inputs
- Salvar configurações

## 🧠 Base do Projeto

O BRSWDIY é inspirado no projeto original:

- Arduino-FFB-wheel (ranenbg)

Mas foi reconstruído com foco em:

- Redução de overhead
- Melhor controle de memória (RAM/Flash)
- Organização de código orientada a sistemas mais fracos (Intel I3 13400F)
- Independência de GUI

## 🧩 Diagrama da versão Apus

- ### Diagrama do Encoder Optical

  ![Optical Encoder Diagram](hardware/diagrams/optical-encoder.png "Diagrama para Optical Encoder")

- ### Diagrama do Motor Driver e Pedais
  ![Motor Driver Diagram](hardware/diagrams/motor-driver-and-pedal.png "Diagrama para Motor Driver")

## ⚙️ Características Principais

- ✔ Firmware otimizado para Arduino Leonardo suporta pilhas de reports (1Kb de RAM livre)
- ✔ Comunicação serial própria (baixo overhead - 200hz)
- ✔ Sistema de calibração por pedal
- ✔ Estrutura preparada para filtros básicos
- ✔ Watchdog configurável
- ✔ Sem dependência de GUI em runtime
- ✔ Uso direto como dispositivo HID

## 🧪 Status

> 🚧 **Alpha — Apus**

- Estrutura base funcional
- Protocolo serial implementado
- Sistema de inputs e outputs operacional
- Interface inicial disponível (Apus Utility)

Esta versão ainda está em evolução e sujeita a mudanças.

## 🪐 Versionamento

Cada versão alternativa do BRSWDIY segue uma sequência baseada em constelações:

- **Apus**: Leve e otimizada, focada em desempenhar com o mínimo possível viável no Arduino Leonardo.

A ideia é manter uma progressão clara de cada projeto, não mudando seu objetivo inicial.

## 🙏 Créditos

Este projeto só existe graças ao trabalho de base de:

- Arduino-FFB-wheel — por ranenbg
- FFB HID and USB core for Arduino — por Peter Barrett

Vocês são demais, muito obrigado por fazerem esses projetos!!
