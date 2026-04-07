/*
 * ===================================================================================
 * Arquivo:       Botoes.h
 * Resumo:        Botoes 1-6 (V-Pot Click) | Botao 7 (Bank L) | Botao 8 (Bank R)
 * Endereço:      MCP23017 (0x21)
 * ===================================================================================
 */

#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MCP23X17.h>

Adafruit_MCP23X17 mcpBotoes;

uint16_t lastButtonState = 0xFFFF;

void setupBotoes() {
  if (mcpBotoes.begin_I2C(0x21)) {
    for (int i = 0; i < 8; i++) {
      mcpBotoes.pinMode(i, INPUT_PULLUP);
    }
    lastButtonState = mcpBotoes.readGPIOAB();
  }
}

void processBotoes() {
  uint16_t currentState = mcpBotoes.readGPIOAB();

  if (currentState != lastButtonState) {
    for (int i = 0; i < 8; i++) {
      bool pressedNow = !bitRead(currentState, i); 
      bool pressedBefore = !bitRead(lastButtonState, i);

      if (pressedNow && !pressedBefore) {
        byte noteToSend;

        // Lógica de atribuição das notas Mackie
        if (i < 6) {
          noteToSend = 32 + i; // Botões 1 a 6 -> V-Pot Click (Reset Pan)
        } else if (i == 6) {
          noteToSend = 46;     // Botão 7 -> Bank Left
        } else {
          noteToSend = 47;     // Botão 8 -> Bank Right
        }

        midiHandler.sendNoteOn(noteToSend, 127, 1);
      } 
      else if (!pressedNow && pressedBefore) {
        byte noteToSend;
        
        if (i < 6) {
          noteToSend = 32 + i;
        } else if (i == 6) {
          noteToSend = 46;
        } else {
          noteToSend = 47;
        }

        midiHandler.sendNoteOff(noteToSend, 0, 1);
      }
    }
    lastButtonState = currentState;
  }
}