/*
 * ===================================================================================
 * File:          Fader.h
 * Description:   Proportional Distance Control (Robotics) + Auto Calibration
 * ===================================================================================
 */

#pragma once
#include <Arduino.h>
#include "Mackie.h"

// --------------------------------------------------------
// HARDWARE SETTINGS
// --------------------------------------------------------
const int MUX_S0 = 10;
const int MUX_S1 = 11;
const int MUX_S2 = 12;
const int MUX_SIG = 5;

const int UART_TX = 41; 
const int UART_RX = 42; 

const int NUM_FADERS = 8;

// 1. O FILTRO DE LEITURA (Velocidade aumentada para matar o lag de posição)
const int READ_THRESHOLD = 20;  
const float EMA_ALPHA = 0.4; // Agora pega 40% da nova leitura na hora!

// 2. CONFIGURAÇÕES DA ROBÓTICA (O Freio ABS do motor)
const int MOTOR_DEADZONE = 18; // Tolerância apertada para altíssima precisão
const int BRAKE_DISTANCE = 500; // Começa a pisar no freio quando estiver a 500 pontos do alvo

const int MOTOR_MIN_SPEED = 130; // Força segura para o motor não encalhar
const int MOTOR_MAX_SPEED = 255;

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
  
  bool isTouched; 
  byte lastDir;   
  byte lastSpeed; 

  bool isMoving;           
};

Fader faders[NUM_FADERS] = {
  { 0, 1, 0, 0.0, 0, 0, FADER_TOUCH_1, 50, 4050, false, 0, 0, false },
  { 1, 2, 0, 0.0, 0, 0, FADER_TOUCH_2, 50, 4050, false, 0, 0, false },
  { 2, 3, 0, 0.0, 0, 0, FADER_TOUCH_3, 50, 4050, false, 0, 0, false },
  { 3, 4, 0, 0.0, 0, 0, FADER_TOUCH_4, 50, 4050, false, 0, 0, false },
  { 4, 5, 0, 0.0, 0, 0, FADER_TOUCH_5, 50, 4050, false, 0, 0, false },
  { 5, 6, 0, 0.0, 0, 0, FADER_TOUCH_6, 50, 4050, false, 0, 0, false },
  { 6, 7, 0, 0.0, 0, 0, FADER_TOUCH_7, 50, 4050, false, 0, 0, false },
  { 7, 8, 0, 0.0, 0, 0, FADER_TOUCH_8, 50, 4050, false, 0, 0, false }
};

void selectMuxChannel(byte channel) {
  digitalWrite(MUX_S0, bitRead(channel, 0));
  digitalWrite(MUX_S1, bitRead(channel, 1));
  digitalWrite(MUX_S2, bitRead(channel, 2));
  delayMicroseconds(15);
}

void sendMotorCommand(byte motorIndex, byte direction, byte speed) {
  byte packet[4] = { 0xFF, motorIndex, direction, speed };
  Serial1.write(packet, 4);
}

// Puxa a Rotina de Calibração
#include "Calibration.h"

void setupFaders() {
  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT);

  Serial1.begin(1000000, SERIAL_8N1, UART_RX, UART_TX);

  calibrateFaders(); 

  for (int i = 0; i < NUM_FADERS; i++) {
    selectMuxChannel(faders[i].muxChannel);
    int initialReading = analogRead(MUX_SIG);
    faders[i].smoothedPosition = initialReading;
    faders[i].lastSentPosition = initialReading;
    faders[i].targetPosition = initialReading;
  }
}

void readFaderPositions() {
  for (int i = 0; i < NUM_FADERS; i++) {
    selectMuxChannel(faders[i].muxChannel);
    
    analogRead(MUX_SIG);
    delayMicroseconds(5);
    
    int rawReading = analogRead(MUX_SIG);
    rawReading = constrain(rawReading, faders[i].minPhysical, faders[i].maxPhysical);

    faders[i].smoothedPosition = (EMA_ALPHA * rawReading) + ((1.0 - EMA_ALPHA) * faders[i].smoothedPosition);

    if (faders[i].isTouched && abs((int)faders[i].smoothedPosition - faders[i].lastSentPosition) > READ_THRESHOLD) {
      faders[i].lastSentPosition = (int)faders[i].smoothedPosition;

      int midiValue = map((int)faders[i].smoothedPosition, faders[i].minPhysical, faders[i].maxPhysical, -8192, 8191);
      midiValue = constrain(midiValue, -8192, 8191);

      midiHandler.sendPitchBend(faders[i].midiChannel, midiValue);
    }
  }
}

void updateFaderTarget(byte receivedChannel, uint16_t receivedMidiValue) {
  int i = receivedChannel - 1;
  if (i >= 0 && i < NUM_FADERS) {
    // Agora o alvo é engolido na mesma hora, sem resetar cronômetros!
    faders[i].targetPosition = map(receivedMidiValue, 0, 16383, faders[i].minPhysical, faders[i].maxPhysical);
    faders[i].isMoving = true;
  }
}

void processMotors() {
  for (int i = 0; i < NUM_FADERS; i++) {
    
    // SEGURANÇA 1: Se estiver com o dedo no metal, corta a energia e cancela a viagem
    if (faders[i].isTouched) {
      if (faders[i].lastDir != 0) { 
        sendMotorCommand(i, 0, 0); 
        faders[i].lastDir = 0;
        faders[i].lastSpeed = 0;
        faders[i].isMoving = false; 
      }
      continue; 
    }

    if (!faders[i].isMoving) continue; 

    int currentPos = (int)faders[i].smoothedPosition;
    int error = faders[i].targetPosition - currentPos;
    int absError = abs(error);

    byte dir = 0;
    byte speed = 0;

    // SEGURANÇA 2: ZONA MORTA. Se chegou no alvo (com margem de 18 pontos), desliga o motor.
    if (absError <= MOTOR_DEADZONE) {
      faders[i].isMoving = false;
      sendMotorCommand(i, 0, 0);
      faders[i].lastDir = 0;
      faders[i].lastSpeed = 0;
      continue;
    }

    // --- A MATEMÁTICA PROPORCIONAL DE ROBÓTICA ---
    int calcSpeed = MOTOR_MAX_SPEED; 
    
    // Se estiver se aproximando do alvo (a menos de 500 pontos), começa a tirar a força
    if (absError < BRAKE_DISTANCE) {
      calcSpeed = map(absError, MOTOR_DEADZONE, BRAKE_DISTANCE, MOTOR_MIN_SPEED, MOTOR_MAX_SPEED);
    }
    
    speed = constrain(calcSpeed, MOTOR_MIN_SPEED, MOTOR_MAX_SPEED);
    dir = (error > 0) ? 1 : 2; 

    // OTIMIZAÇÃO DE REDE: Só manda mensagem pro RP2040 se a direção mudou 
    // ou se a velocidade alterou consideravelmente (evita congestionar os cabos com lixo de 1%)
    if (dir != faders[i].lastDir || abs(speed - faders[i].lastSpeed) > 5) {
      sendMotorCommand(i, dir, speed);
      faders[i].lastDir = dir;
      faders[i].lastSpeed = speed;
    }
  }
}