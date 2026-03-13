/*
 * ===================================================================================
 * Project:       Controlador MIDI ESP32-S3 (Superfície de Controle)
 * File:          Encoders.h
 * Description:   Controle de Encoders via I2C usando MCP23017 e Interrupções.
 * ===================================================================================
 */

#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MCP23X17.h>
#include "MackieMCU.h"

Adafruit_MCP23X17 mcp;

// [!] MUDE AQUI: Pino físico do ESP32 que vai ser ligado no pino INTA do MCP23017
const byte MCP_INT_PIN = 4; 

// [!] MUDE AQUI: Quantidade de Encoders ligados ao MCP23017 (Máximo 8 por chip)
const int NUM_ENCODERS = 8;

struct EncoderParams {
  byte pinA;           
  byte pinB;           
  byte ccNumber;       
  bool lastStateA;     
};

// [!] MUDE AQUI: Pinos do chip MCP23017 (Lembre-se: O chip tem pinos lógicos de 0 a 15)
EncoderParams encoders[NUM_ENCODERS] = {
// PinA (MUDE) | PinB (MUDE) | Nota V_POT | lastState
  { 0,           1,            V_POT_1,     HIGH },
  { 2,           3,            V_POT_2,     HIGH },
  { 4,           5,            V_POT_3,     HIGH },
  { 6,           7,            V_POT_4,     HIGH },
  { 8,           9,            V_POT_5,     HIGH },
  { 10,          11,           V_POT_6,     HIGH },
  { 12,          13,           V_POT_7,     HIGH },
  { 14,          15,           V_POT_8,     HIGH }
};

volatile bool encoderMovido = false;

void IRAM_ATTR mcpInterrupt() {
  encoderMovido = true;
}

void setupEncoders() {
  // [!] MUDE AQUI: Endereço I2C do MCP23017 (0x20 é o padrão de fábrica)
  if (!mcp.begin_I2C(0x20)) {
    return; 
  }

  pinMode(MCP_INT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(MCP_INT_PIN), mcpInterrupt, FALLING);

  for (int i = 0; i < NUM_ENCODERS; i++) {
    mcp.pinMode(encoders[i].pinA, INPUT_PULLUP);
    mcp.pinMode(encoders[i].pinB, INPUT_PULLUP);
    mcp.setupInterruptPin(encoders[i].pinA, CHANGE);
    mcp.setupInterruptPin(encoders[i].pinB, CHANGE);
    encoders[i].lastStateA = mcp.digitalRead(encoders[i].pinA);
  }

  mcp.setupInterrupts(true, false, LOW); 
  mcp.clearInterrupts();
}

void processarEncoders() {
  if (encoderMovido) {
    encoderMovido = false; 
    
    for (int i = 0; i < NUM_ENCODERS; i++) {
      bool stateA = mcp.digitalRead(encoders[i].pinA);
      bool stateB = mcp.digitalRead(encoders[i].pinB);

      if (stateA != encoders[i].lastStateA) {
        if (stateA == LOW) {
          if (stateB == LOW) {
            midiHandler.sendControlChange(1, encoders[i].ccNumber, 65);
          } else {
            midiHandler.sendControlChange(1, encoders[i].ccNumber, 1);
          }
        }
        encoders[i].lastStateA = stateA;
      }
    }
    mcp.clearInterrupts(); 
  }
}