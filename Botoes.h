/*
 * ===================================================================================
 * Project:       Vertex Fader
 * File:          Botoes.h
 * Description:   Gerenciamento de botões físicos (Transporte, Bancos, etc) com 
 * debounce de hardware e envio de notas Mackie Control.
 * ===================================================================================
 */

#pragma once
#include <Arduino.h>
#include "MackieMCU.h" 

// Tempo em milissegundos para ignorar os "tremiliques" mecânicos da mola do botão
const unsigned long DEBOUNCE_TIME = 30; 
const int NUM_BOTOES = 5; // Ajuste para a quantidade real de botões do seu painel

struct Botao {
  byte pin;                 // Pino físico do ESP32-S3
  byte note;                // Nota Mackie que esse botão vai disparar
  bool currentState;        // Estado estabilizado atual
  bool lastState;           // Última leitura bruta
  unsigned long lastDebounceTime; 
};

// Mapeamento dos Botões (Exemplo: Pinos 15 a 19. Altere para o seu hardware)
Botao botoes[NUM_BOTOES] = {
// Pino | Nota Mackie | State | L_State | DebounceTime
  { 15,   BANK_LEFT,    HIGH,   HIGH,     0 },
  { 16,   BANK_RIGHT,   HIGH,   HIGH,     0 },
  { 17,   PLAY,         HIGH,   HIGH,     0 },
  { 18,   STOP,         HIGH,   HIGH,     0 },
  { 19,   RECORD,       HIGH,   HIGH,     0 }
};

void setupBotoes() {
  for (int i = 0; i < NUM_BOTOES; i++) {
    // INPUT_PULLUP ativa o resistor interno do ESP32. 
    // O botão só precisa conectar o pino ao GND.
    pinMode(botoes[i].pin, INPUT_PULLUP);
  }
}

void processarBotoes() {
  for (int i = 0; i < NUM_BOTOES; i++) {
    bool reading = digitalRead(botoes[i].pin);

    // Se o sinal mudou (mesmo que por ruído), reseta o temporizador
    if (reading != botoes[i].lastState) {
      botoes[i].lastDebounceTime = millis();
    }

    // Se o sinal se manteve estável além do tempo de Debounce
    if ((millis() - botoes[i].lastDebounceTime) > DEBOUNCE_TIME) {
      
      if (reading != botoes[i].currentState) {
        botoes[i].currentState = reading;

        // LOW significa que o botão foi APERTADO (conectou no GND)
        if (botoes[i].currentState == LOW) {
          midiHandler.sendNoteOn(1, botoes[i].note, 127);
        } 
        // HIGH significa que o botão foi SOLTO
        else {
          midiHandler.sendNoteOn(1, botoes[i].note, 0);
        }
      }
    }
    botoes[i].lastState = reading; // Salva para o próximo loop
  }
}