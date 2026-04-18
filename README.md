# Vertex Fader

Controlador MIDI com faders motorizados em arquitetura distribuida, inspirado em superfícies como Waves Fit e Behringer X-Touch.

## Visao geral

O projeto usa dois microcontroladores trabalhando juntos:

- `ESP32-S3`: USB MIDI, touch capacitivo, botoes, encoders e comunicacao com a DAW.
- `RP2040`: leitura fisica dos faders, malha fechada dos motores e feedback rapido por UART.

Essa separacao deixa o MIDI e a interface no ESP32 enquanto o RP2040 fica dedicado ao movimento dos faders.

## Recursos atuais

- 8 faders motorizados com feedback por UART
- Touch capacitivo com `MPR121`
- 8 encoders via `MCP23017`
- 8 botoes via `MCP23017`
- Protocolo baseado em Mackie Control
- Projeto organizado para `PlatformIO`

## Estrutura do repositorio

```text
.
|-- include/
|   |-- Button.h
|   |-- Encoder.h
|   |-- Fader.h
|   |-- Mackie.h
|   |-- Touch.h
|   `-- USBDeviceConnection.h
|-- src/
|   |-- VertexFader.cpp
|   `-- Slave.cpp
`-- platformio.ini
```

## Ambientes PlatformIO

O repositorio tem dois ambientes:

- `esp32s3`: firmware principal
- `rp2040`: firmware escravo dos motores

## Como compilar

```bash
pio run -e esp32s3
pio run -e rp2040
```

## Como gravar

Exemplos:

```bash
pio run -e esp32s3 -t upload
pio run -e rp2040 -t upload
```

## Observacoes

- A comunicacao entre os processadores usa UART em `1000000 baud`.
- O `ESP32-S3` envia e recebe MIDI por USB.
- O `RP2040` concentra a resposta fisica dos faders para reduzir latencia e ruido de controle.

## Estado do projeto

Esta versao substitui a estrutura antiga baseada em pastas `.ino` por uma organizacao moderna em `PlatformIO`, facilitando manutencao, build e evolucao do firmware.

## Autor

Desenvolvido por Marlon Rodrigues.
