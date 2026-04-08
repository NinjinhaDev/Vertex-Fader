/*
 * ===================================================================================
 * File:          Button.h
 * Description:   Leitura dos Botões no MCP23017 (0x21) - Lado B
 * ===================================================================================
 */

#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MCP23X17.h>
#include "Mackie.h"

Adafruit_MCP23X17 mcpBotoes;
uint16_t lastButtonState = 0xFFFF;

// MAPA NA ORDEM NATURAL (Pinos Físicos 8 a 15 do MCP Lado B)
const uint8_t MAPA_BOTOES[8] = {
  SELECT_1,      // Botão Físico 1: Seleciona Track 1
  SELECT_2,      // Botão Físico 2: Seleciona Track 2
  SELECT_3,      // Botão Físico 3: Seleciona Track 3
  ASSIGN_PAN,    // Botão Físico 4: Volta os encoders para PAN (Resgate)
  FLIP,          // Botão Físico 5: Inverte Faders com Encoders
  ASSIGN_EQ,     // Botão Físico 6: Modo Equalizador
  BANK_LEFT,     // Botão Físico 7: Página para a Esquerda
  BANK_RIGHT     // Botão Físico 8: Página para a Direita
};

void setupBotoes() {
  if (mcpBotoes.begin_I2C(0x21)) {
    // Mantemos os 16 pinos com Pull-Up para silenciar o barramento
    for (int i = 0; i < 16; i++) {
      mcpBotoes.pinMode(i, INPUT_PULLUP);
    }
    lastButtonState = mcpBotoes.readGPIOAB();
  }
}

void processBotoes() {
  uint16_t currentState = mcpBotoes.readGPIOAB();

  if (currentState != lastButtonState) {
    
    for (int i = 0; i < 8; i++) {
      
      // Lendo os pinos do Lado B (8 a 15)
      int pinoFisico = i + 8; 
      
      bool pressedNow = !bitRead(currentState, pinoFisico); 
      bool pressedBefore = !bitRead(lastButtonState, pinoFisico);

      uint8_t notaMidi = MAPA_BOTOES[i];

      if (pressedNow && !pressedBefore) {
        midiHandler.sendNoteOn(1, notaMidi, 127);
      } 
      else if (!pressedNow && pressedBefore) {
        midiHandler.sendNoteOff(1, notaMidi, 0);
      }
    }
    lastButtonState = currentState;
  }
}