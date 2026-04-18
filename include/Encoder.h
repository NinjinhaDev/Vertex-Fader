/*
 * ===================================================================================
 * Arquivo:       Encoders.h
 * Resumo:        Leitura Encoders via MCP23017 (0x20)
 * Funcionalidade: Correção de Fase (Giro) via Software e Ordem Direta
 * ===================================================================================
 */

#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MCP23X17.h>
#include "Mackie.h"

Adafruit_MCP23X17 mcpEncoders;
uint16_t lastEncoderState = 0xFFFF; 

// 1. ORDEM CORRIGIDA: Como você arrumou os fios, o pino 0 é o Canal 1 de novo!
const uint8_t MAPA_ENCODERS[8] = {
  V_POT_1, V_POT_2, V_POT_3, V_POT_4, 
  V_POT_5, V_POT_6, V_POT_7, V_POT_8
};

// 2. MAPA DE INVERSÃO DE GIRO: 'true' inverte a esquerda/direita via software
const bool INVERTER_GIRO[8] = {
  true,  // Encoder 1 (Fio A/B invertido fisicamente)
  true,  // Encoder 2 (Fio A/B invertido fisicamente)
  true,  // Encoder 3 (Fio A/B invertido fisicamente)
  true,  // Encoder 4 (Fio A/B invertido fisicamente)
  false, // Encoder 5 (Normal)
  false, // Encoder 6 (Normal)
  false, // Encoder 7 (Normal)
  false  // Encoder 8 (Normal)
};

void setupEncoders() {
  if (mcpEncoders.begin_I2C(0x20)) {
    for (int i = 0; i < 16; i++) {
      mcpEncoders.pinMode(i, INPUT_PULLUP);
    }
    lastEncoderState = mcpEncoders.readGPIOAB();
  }
}

void processEncoders() {
  uint16_t currentState = mcpEncoders.readGPIOAB();
  if (currentState == lastEncoderState) return; 

  for (int i = 0; i < 8; i++) {
    int pinA = i * 2;
    int pinB = i * 2 + 1;

    bool a = bitRead(currentState, pinA);
    bool b = bitRead(currentState, pinB);
    bool lastA = bitRead(lastEncoderState, pinA);

    if (lastA == 1 && a == 0) { 
      uint8_t ccMidi = MAPA_ENCODERS[i];
      
      // Descobre para que lado girou
      bool girouDireita = (b == 1);
      
      // O SEGREDO: Se este encoder estiver no mapa de inversão, inverte a lógica!
      if (INVERTER_GIRO[i]) {
        girouDireita = !girouDireita; 
      }

      if (girouDireita) { 
        midiHandler.sendControlChange(1, ccMidi, 1);  // DIREITA
      } else {      
        midiHandler.sendControlChange(1, ccMidi, 65); // ESQUERDA
      }
    }
  }
  lastEncoderState = currentState;
}