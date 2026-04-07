/*
 * ===================================================================================
 * Arquivo:       Vertex_Fader_Main.ino
 * Projeto:       Vertex Fader
 * Resumo:        Versão USB-MIDI Pura (Wi-Fi e OTA desativados)
 * ===================================================================================
 */

#include <Arduino.h>
#include <Wire.h> // Para o I2C (MCP23017)

// 1. Bibliotecas de MIDI e Conexão
#define ESP32_HOST_MIDI_NO_USB_HOST
#include <ESP32_Host_MIDI.h>
#include <USBDeviceConnection.h>

// 2. Módulos do Sistema (Abas)
// #include "Conexao.h"  // <-- DESATIVADO
// #include "Pagina.h"   // <-- DESATIVADO
#include "MackieMCU.h"
#include "Faders.h"
#include "Touch.h"
#include "Botoes.h"      // <-- MCP23017 (0x21)
#include "Encoders.h"    // <-- MCP23017 (0x20)

// 3. Instâncias Globais
USBDeviceConnection usbMIDI("Vertex Controller");

// ===================================================================================
// CALLBACKS MIDI
// ===================================================================================
void aoReceberMidiBruto(const uint8_t* raw, size_t rawLen, const uint8_t* midi3) {
  uint8_t status = midi3[0] & 0xF0;
  uint8_t canal  = (midi3[0] & 0x0F) + 1;

  if (status == 0xE0) { // Pitchbend (Motores)
    uint16_t valor14bits = (midi3[2] << 7) | midi3[1];
    updateFaderTarget(canal, valor14bits);
  }
}

// ===================================================================================
// SETUP
// ===================================================================================
void setup() {

  // 1. Inicia o Barramento I2C (Vital para os MCP23017 não travarem)
  Wire.begin(); 

  // 2. PRIORIDADE TOTAL AO USB MIDI
  midiHandler.addTransport(&usbMIDI);
  usbMIDI.begin();
  delay(1000); 

  // 3. Configurar o Maestro MIDI
  midiHandler.setRawMidiCallback(aoReceberMidiBruto);
  MIDIHandlerConfig cfg;
  midiHandler.begin(cfg);

  // 4. Inicializa Hardware Físico
  setupTouch();
  setupEncoders(); 
  setupBotoes();   
  setupFaders(); // RP2040 liga aqui
}

// ===================================================================================
// LOOP
// ===================================================================================
void loop() {
  // Mantém o MIDI vivo
  midiHandler.task(); 

  // Faders
  checkTouch();         
  readFaderPositions(); 
  processMotors();      

  // I2C Expansões (MCP23017)
  processEncoders();    
  processBotoes();      
}