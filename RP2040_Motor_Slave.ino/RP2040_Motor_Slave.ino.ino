/*
 * ===================================================================================
 * Arquivo:       RP2040_Motor_Slave.ino
 * Projeto:       Vertex Fader v1.3 (Escravo de Potência via UART)
 * Funcionalidade: Drive de Motores Silencioso (25kHz) + Watchdog de Segurança
 * ===================================================================================
 */

#include <Arduino.h>

const int motorPins[8][2] = {
  { 0, 1 }, { 2, 3 }, { 4, 5 }, { 6, 7 },
  { 8, 9 }, { 10, 11 }, { 12, 13 }, { 14, 15 }
};

// --- VARIÁVEIS DE SEGURANÇA (WATCHDOG) ---
unsigned long ultimoComandoTime = 0;
bool motoresEnergizados = false;

void setup() {
  Serial1.setTX(16);
  Serial1.setRX(17);
  Serial1.begin(1000000); 

  analogWriteFreq(25000); 

  for (int i = 0; i < 8; i++) {
    pinMode(motorPins[i][0], OUTPUT);
    pinMode(motorPins[i][1], OUTPUT);
    analogWrite(motorPins[i][0], 0);
    analogWrite(motorPins[i][1], 0);
  }
}

void loop() {
  // 1. RECEPÇÃO DE DADOS (O MÚSCULO)
  while (Serial1.available() > 0) {
    if (Serial1.read() == 0xFF) {
      
      unsigned long timeoutStart = millis();
      while (Serial1.available() < 3) {
        if (millis() - timeoutStart > 5) return;
      }
      
      byte motorIndex = Serial1.read();
      byte dir = Serial1.read();
      byte speed = Serial1.read();

      if (motorIndex < 8) {
        int pinA = motorPins[motorIndex][0]; 
        int pinB = motorPins[motorIndex][1]; 

        if (dir == 0) {
          analogWrite(pinA, 0);
          analogWrite(pinB, 0);
        } else if (dir == 1) {
          analogWrite(pinB, 0);
          analogWrite(pinA, speed);
        } else if (dir == 2) {
          analogWrite(pinA, 0);
          analogWrite(pinB, speed);
        }
        
        // Registra o momento exato em que o cérebro falou com o músculo
        ultimoComandoTime = millis();
        motoresEnergizados = true;
      }
    }
  }

  // 2. WATCHDOG DE SEGURANÇA (O FREIO DE EMERGÊNCIA)
  // Se passou de 100ms sem receber ordens e os motores estão ligados, freia tudo!
  if (motoresEnergizados && (millis() - ultimoComandoTime > 100)) {
    for (int i = 0; i < 8; i++) {
      analogWrite(motorPins[i][0], 0);
      analogWrite(motorPins[i][1], 0);
    }
    motoresEnergizados = false;
  }
}