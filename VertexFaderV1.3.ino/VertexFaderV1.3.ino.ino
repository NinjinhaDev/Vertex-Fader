/*
 * ===================================================================================
 * Arquivo:       Vertex_Fader_Main.ino
 * Projeto:       Vertex Fader v1.3
 * Resumo:        Processamento via Fila Nativa (getQueue) - Sem Bit-Shift manual
 * ===================================================================================
 */

#include <Arduino.h>
#include <Wire.h> 

// 1. Bibliotecas de MIDI e Conexão
#define ESP32_HOST_MIDI_NO_USB_HOST
#include <ESP32_Host_MIDI.h>
#include <USBDeviceConnection.h>

// 2. Módulos
#include "Mackie.h"
#include "Fader.h"
#include "Touch.h"
#include "Button.h"    // MCP23017 (0x21)
#include "Encoder.h"  // MCP23017 (0x20)

// 3. Instâncias Globais
USBDeviceConnection usbMIDI("Vertex Controller");

// ===================================================================================
// SETUP
// ===================================================================================
void setup() {
  // 1. I2C Fast Mode (400kHz para Encoders lisos)
  Wire.begin();
  Wire.setClock(400000);  

  // 2. Inicia o USB MIDI
  midiHandler.addTransport(&usbMIDI);
  usbMIDI.begin();
  delay(1000); // Respiro para o Windows

  // 3. O CORAÇÃO DO MIDI (Agora muito mais limpo, sem callback bruto)
  MIDIHandlerConfig cfg;
  midiHandler.begin(cfg);

  // 4. Inicia o Hardware
  setupTouch();
  setupBotoes();
  setupEncoders();
  setupFaders();
}

// ===================================================================================
// LOOP
// ===================================================================================
void loop() {
  // 1. Mantém o Maestro MIDI vivo
  midiHandler.task(); 

  // 2. LÊ A FILA OFICIAL DA BIBLIOTECA
  for (const auto& ev : midiHandler.getQueue()) {
    
    // Se a DAW mandou os motores se mexerem (PitchBend)
    if (ev.statusCode == MIDI_PITCH_BEND) {
      // O ev.channel0 vem de 0 a 15. Somamos 1 porque nossos motores vão de 1 a 8.
      // O ev.pitchBend14 já entrega o valor exato de 0 a 16383!
      updateFaderTarget(ev.channel0 + 1, ev.pitchBend14);
    }
  }

  // 3. Processamento de Expansões (I2C)
  processBotoes();
  processEncoders();

  // 4. Processamento Físico de Alta Prioridade (Sensores e Motores via UART)
  checkTouch();
  readFaderPositions();
  processMotors();
}