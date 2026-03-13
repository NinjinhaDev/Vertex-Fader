/*
 * ===================================================================================
 * Project:       Vertex Fader (Controlador MIDI ESP32-S3)
 * Author:        Marlon
 * Date:          March / 2026
 * Repository:    https://github.com/NinjinhaDev/Vertex-Fader
 * Version:       1.0.0
 * * Description: 
 * Arquivo principal (Maestro) do controlador MIDI Vertex.
 * Integra a pilha de comunicação (USB Nativo + BLE MIDI), o servidor Web/OTA
 * e a lógica de hardware para Faders Motorizados, Botões e Encoders.
 * * License:       MIT License (Open Source)
 * ===================================================================================
 */

#include <Arduino.h>

// 1. Bibliotecas de MIDI do Saulo (Devem vir primeiro)
#define ESP32_HOST_MIDI_NO_USB_HOST
#include <ESP32_Host_MIDI.h>
#include <USBDeviceConnection.h>
#include <BLEConnection.h>

// 2. Módulos do Sistema (Suas Abas)
#include "Conexao.h"
#include "Pagina.h"
#include "MackieMCU.h"
#include "Faders.h"
#include "Botoes.h"
#include "Encoders.h"

// 3. Instâncias Globais
WebServer server(80);
USBDeviceConnection usbMIDI("Vertex USB");
BLEConnection bleMIDI;

// ===================================================================================
// CALLBACKS MIDI (O "Ouvido" do ESP32 para a DAW)
// ===================================================================================
/* * Esta função recebe os bytes brutos do MIDI. É a forma mais robusta de 
 * comunicação com a biblioteca do Saulo para evitar erros de compilação.
 */
void aoReceberMidiBruto(const uint8_t* raw, size_t rawLen, const uint8_t* midi3) {
  // midi3[0] -> Status (Tipo da mensagem + Canal)
  // midi3[1] -> Dado 1 (LSB no caso de Pitchbend)
  // midi3[2] -> Dado 2 (MSB no caso de Pitchbend)

  uint8_t status = midi3[0] & 0xF0;
  uint8_t canal  = (midi3[0] & 0x0F) + 1; // Converte canal 0-15 para 1-16

  // Se o status for 0xE0, significa que o Reaper mandou um Pitchbend (Fader se mexendo)
  if (status == 0xE0) {
    // Reconstrói o valor de 14 bits (combinando os dois bytes de dados)
    uint16_t valor14bits = (midi3[2] << 7) | midi3[1];
    
    // Manda a ordem de movimento para o fader correspondente na aba Faders.h
    updateFaderTarget(canal, valor14bits);
  }
}

// ===================================================================================
// SETUP (Inicialização)
// ===================================================================================
void setup() {
  // [!] MUDE AQUI: Velocidade do Monitor Serial
  Serial.begin(115200);
  delay(1000); 
  Serial.println("\n--- Iniciando Vertex Fader ---");

  // 1. Inicializa Rede e Painel Web
  setupConexao();
  setupRotasWeb();

  // 2. Inicializa o Hardware (Faders, Botões e Encoders via MCP23017)
  setupFaders();
  setupBotoes();
  setupEncoders();

  // 3. Configura e engata os Transportes MIDI (Cabo e Bluetooth)
  midiHandler.addTransport(&usbMIDI);
  usbMIDI.begin();

  midiHandler.addTransport(&bleMIDI);
  bleMIDI.begin("Vertex BLE");

  // 4. Registra o Callback de entrada usando o método Raw para máxima compatibilidade
  midiHandler.setRawMidiCallback(aoReceberMidiBruto);

  // 5. Dá o start na central de tráfego do MIDI com as configurações padrão
  MIDIHandlerConfig cfg;
  cfg.bleName = "Vertex BLE"; // Nome que aparecerá no Bluetooth
  midiHandler.begin(cfg);

  Serial.println("Sistema pronto e rodando!");
}

// ===================================================================================
// LOOP (Ciclo de Vida)
// ===================================================================================
void loop() {
  // Mantém a comunicação viva (WiFi, OTA, USB e BLE)
  ArduinoOTA.handle();
  server.handleClient();
  midiHandler.task(); 

  // Executa o hardware em alta velocidade
  checkTouch();         // Lê os dedos (Desarma motor e avisa DAW)
  readFaderPositions(); // Lê faders e envia Pitchbend para a DAW
  processMotors();      // Move os motores para o alvo recebido da DAW
  processarBotoes();    // Lê os botões de transporte/banco
  processarEncoders();  // Lê os encoders (Pan) via MCP23017
}