/*
 * ===================================================================================
 * Arquivo:       Faders.h
 * Projeto:       Vertex Fader
 * Resumo:        Leitura MUX Corrigida + UART Escravo RP2040
 * ===================================================================================
 */

#pragma once
#include <Arduino.h>
#include "MackieMCU.h"

// --------------------------------------------------------
// HARDWARE SETTINGS (Pinos Corrigidos pela Versão Estável)
// --------------------------------------------------------
const int MUX_S0 = 10;
const int MUX_S1 = 11;
const int MUX_S2 = 12;
const int MUX_SIG = 5;

// Pinos de Comunicação Serial com a RP2040 (Motor Engine)
const int UART_TX = 41; // Fio amarelo na RP2040 (GP17)
const int UART_RX = 42; // Fio verde na RP2040 (GP16)

const int NUM_FADERS = 8;
const int READ_THRESHOLD = 35;  
const float EMA_ALPHA = 0.1; 
const int MOTOR_DEADZONE = 25; 

// --------------------------------------------------------
// FADER STRUCTURE
// --------------------------------------------------------
struct Fader {
  byte muxChannel;
  byte midiChannel;
  int physicalPosition;
  float smoothedPosition;
  int lastSentPosition;
  int targetPosition;
  byte touchNote;
  int minPhysical;
  int maxPhysical;
  bool isTouched; // Controlado pela aba Touch.h
  byte lastDir;   
  byte lastSpeed; 
};

// Mapeamento dos 8 Faders
Fader faders[NUM_FADERS] = {
  { 0, 1, 0, 0.0, 0, 0, FADER_TOUCH_1, 50, 4050, false, 0, 0 },
  { 1, 2, 0, 0.0, 0, 0, FADER_TOUCH_2, 50, 4050, false, 0, 0 },
  { 2, 3, 0, 0.0, 0, 0, FADER_TOUCH_3, 50, 4050, false, 0, 0 },
  { 3, 4, 0, 0.0, 0, 0, FADER_TOUCH_4, 50, 4050, false, 0, 0 },
  { 4, 5, 0, 0.0, 0, 0, FADER_TOUCH_5, 50, 4050, false, 0, 0 },
  { 5, 6, 0, 0.0, 0, 0, FADER_TOUCH_6, 50, 4050, false, 0, 0 },
  { 6, 7, 0, 0.0, 0, 0, FADER_TOUCH_7, 50, 4050, false, 0, 0 },
  { 7, 8, 0, 0.0, 0, 0, FADER_TOUCH_8, 50, 4050, false, 0, 0 }
};

// --------------------------------------------------------
// MUX SELECTION
// --------------------------------------------------------
void selectMuxChannel(byte channel) {
  digitalWrite(MUX_S0, bitRead(channel, 0));
  digitalWrite(MUX_S1, bitRead(channel, 1));
  digitalWrite(MUX_S2, bitRead(channel, 2));
  delayMicroseconds(15);
}

// --------------------------------------------------------
// SETUP FADERS
// --------------------------------------------------------
void setupFaders() {
  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT);

  // Inicia a comunicação com a RP2040
  Serial1.begin(1000000, SERIAL_8N1, UART_RX, UART_TX);

  for (int i = 0; i < NUM_FADERS; i++) {
    selectMuxChannel(faders[i].muxChannel);
    int initialReading = analogRead(MUX_SIG);
    faders[i].smoothedPosition = initialReading;
    faders[i].lastSentPosition = initialReading;
    faders[i].targetPosition = initialReading;
  }
}

// --------------------------------------------------------
// READING LOGIC (Físico -> DAW)
// --------------------------------------------------------
void readFaderPositions() {
  for (int i = 0; i < NUM_FADERS; i++) {
    selectMuxChannel(faders[i].muxChannel);
    
    // Leitura fantasma
    analogRead(MUX_SIG);
    delayMicroseconds(5);
    
    int rawReading = analogRead(MUX_SIG);
    rawReading = constrain(rawReading, 50, 4050);

    faders[i].smoothedPosition = (EMA_ALPHA * rawReading) + ((1.0 - EMA_ALPHA) * faders[i].smoothedPosition);

    // REGRA DE OURO: Só envia se o fader estiver tocado (evita Fader Fighting)
    if (faders[i].isTouched && abs((int)faders[i].smoothedPosition - faders[i].lastSentPosition) > READ_THRESHOLD) {
      faders[i].lastSentPosition = (int)faders[i].smoothedPosition;

      int virtualPosition = (int)faders[i].smoothedPosition;
      if (virtualPosition < 60) virtualPosition = faders[i].minPhysical;
      if (virtualPosition > 4045) virtualPosition = faders[i].maxPhysical;

      // Escala corrigida para -8192 a 8191 para casar com a sua biblioteca
      int midiValue = map(virtualPosition, faders[i].minPhysical, faders[i].maxPhysical, -8192, 8191);
      midiValue = constrain(midiValue, -8192, 8191);

      midiHandler.sendPitchBend(faders[i].midiChannel, midiValue);
    }
  }
}

// --------------------------------------------------------
// CONTROLE DOS MOTORES (DAW -> RP2040)
// --------------------------------------------------------
void updateFaderTarget(byte receivedChannel, uint16_t receivedMidiValue) {
  int i = receivedChannel - 1;
  if (i >= 0 && i < NUM_FADERS) {
    // A DAW manda de 0 a 16383 (padrão absoluto), nós convertemos para a posição física
    faders[i].targetPosition = map(receivedMidiValue, 0, 16383, faders[i].minPhysical, faders[i].maxPhysical);
  }
}

void sendMotorCommand(byte motorIndex, byte direction, byte speed) {
  byte packet[4] = { 0xFF, motorIndex, direction, speed };
  Serial1.write(packet, 4);
}

void processMotors() {
  for (int i = 0; i < NUM_FADERS; i++) {
    
    if (faders[i].isTouched) {
      if (faders[i].lastDir != 0) { 
        sendMotorCommand(i, 0, 0);
        faders[i].lastDir = 0;
        faders[i].lastSpeed = 0;
      }
      continue; 
    }

    int currentPos = (int)faders[i].smoothedPosition;
    int error = faders[i].targetPosition - currentPos;
    int absError = abs(error);

    byte dir = 0;
    byte speed = 0;

    if (absError > MOTOR_DEADZONE) {
      dir = (error > 0) ? 1 : 2; 
      int calcSpeed = map(absError, MOTOR_DEADZONE, 1000, 120, 255);
      speed = constrain(calcSpeed, 120, 255);
    }

    if (dir != faders[i].lastDir || speed != faders[i].lastSpeed) {
      sendMotorCommand(i, dir, speed);
      faders[i].lastDir = dir;
      faders[i].lastSpeed = speed;
    }
  }
}