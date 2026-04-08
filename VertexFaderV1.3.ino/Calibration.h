/*
 * ===================================================================================
 * File:          Calibration.h
 * Description:   Auto-calibration routine for motorized faders. Silencioso!
 * ===================================================================================
 */

#pragma once
#include <Arduino.h>

void calibrateFaders() {
  // 1. FORÇA TODOS PARA O ZERO (BAIXO)
  for (int i = 0; i < NUM_FADERS; i++) sendMotorCommand(i, 2, 255);
  delay(800); 
  
  // Lê e salva os Mínimos Físicos
  for (int i = 0; i < NUM_FADERS; i++) {
    selectMuxChannel(faders[i].muxChannel);
    analogRead(MUX_SIG); delayMicroseconds(10); 
    
    long soma = 0;
    for(int r = 0; r < 16; r++) soma += analogRead(MUX_SIG);
    
    faders[i].minPhysical = (soma / 16) + 30; // Margem de segurança
  }

  // 2. FORÇA TODOS PARA O TOPO (CIMA)
  for (int i = 0; i < NUM_FADERS; i++) sendMotorCommand(i, 1, 255);
  delay(800); 
  
  // Lê e salva os Máximos Físicos
  for (int i = 0; i < NUM_FADERS; i++) {
    selectMuxChannel(faders[i].muxChannel);
    analogRead(MUX_SIG); delayMicroseconds(10);
    
    long soma = 0;
    for(int r = 0; r < 16; r++) soma += analogRead(MUX_SIG);
    
    faders[i].maxPhysical = (soma / 16) - 30; // Margem de segurança
  }

  // 3. RETORNA AO ZERO (PRONTO PRA USO)
  for (int i = 0; i < NUM_FADERS; i++) sendMotorCommand(i, 2, 255);
  delay(500);
  for (int i = 0; i < NUM_FADERS; i++) sendMotorCommand(i, 0, 0); // Solta os freios
}