# 🎚️ Vertex Fader - ESP32-S3 MIDI Controller

O **Vertex Fader** é um controlador MIDI de alta performance desenvolvido para o **ESP32-S3**, focado em automação de áudio profissional e mixagem. Ele utiliza o protocolo **Mackie Control Universal (MCU)** para integração nativa com DAWs como Reaper, Ableton Live e Logic Pro.

## 🚀 Funcionalidades
* **Controle de 14-bits:** Suporte a Pitchbend de alta resolução para faders motorizados.
* **Protocolo Mackie MCU:** Mapeamento nativo de transporte (Play, Stop, Record) e bancos.
* **Conectividade Híbrida:** USB Nativo, Bluetooth (BLE MIDI) e Wi-Fi (RTP-MIDI).
* **Hardware Avançado:** * Leitura de 8 faders via Multiplexador CD4051.
    * Controle PWM via PCA9685.
    * Expansão de Encoders via MCP23017 com interrupções de hardware.
    * Automação *Touch-Sensitive* nativa do ESP32-S3.

## 🛠️ Tecnologias Utilizadas
* C++ / Arduino IDE
* Biblioteca `ESP32-Host-MIDI` (by Saulo Veríssimo)
* Interface Web para calibração e configuração via OTA.

## 👤 Autor
* **Marlon (Ninjinha)** - [NinjinhaDev](https://github.com/NinjinhaDev)

---
*Este projeto é Open Source sob a licença MIT.*
