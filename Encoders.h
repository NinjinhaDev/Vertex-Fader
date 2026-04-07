/*
 * ===================================================================================
 * Arquivo:       Encoders.h
 * Resumo:        Leitura Encoders via MCP23017 (0x20) -> Mackie MCU V-Pots (PAN)
 * ===================================================================================
 */

#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MCP23X17.h>

Adafruit_MCP23X17 mcpEncoders;

uint16_t lastEncoderState = 0xFFFF; 

void setupEncoders() {
  if (mcpEncoders.begin_I2C(0x20)) {
    for (int i = 0; i < 16; i++) {
      mcpEncoders.pinMode(i, INPUT_PULLUP);
    }
    // Lê o estado inicial para não dar um "pulo" falso ao ligar
    lastEncoderState = mcpEncoders.readGPIOAB();
  }
}

void processEncoders() {
  uint16_t currentState = mcpEncoders.readGPIOAB();

  if (currentState == lastEncoderState) return;

  // Verifica os 8 encoders (cada um usa 2 pinos: A e B)
  for (int i = 0; i < 8; i++) {
    int pinA = i * 2;
    int pinB = i * 2 + 1;

    bool a = bitRead(currentState, pinA);
    bool b = bitRead(currentState, pinB);
    bool lastA = bitRead(lastEncoderState, pinA);

    // Se o pino A mudou de estado, houve rotação
    if (a != lastA) { 
      if (a != b) {
        // Girou para a DIREITA -> Envia CC (16 a 23), Valor 1, Canal 1
        midiHandler.sendControlChange(16 + i, 1, 1);
      } else {
        // Girou para a ESQUERDA -> Envia CC (16 a 23), Valor 65 (0x41), Canal 1
        midiHandler.sendControlChange(16 + i, 65, 1);
      }
    }
  }
  
  lastEncoderState = currentState;
}