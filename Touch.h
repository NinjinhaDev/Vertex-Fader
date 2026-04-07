/*
 * ===================================================================================
 * Arquivo:       Touch.h
 * Projeto:       Vertex Fader
 * Resumo:        Leitura Capacitiva (MPR121) com Filtro Anti-Tremor (Debounce)
 * ===================================================================================
 */

#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPR121.h>

// --- CONFIGURAÇÕES DO MPR121 ---
#define IRQ_PIN 6
Adafruit_MPR121 cap = Adafruit_MPR121();

// --- VARIÁVEIS DO FILTRO ANTI-TREMOR (DEBOUNCE) ---
#define DEBOUNCE_TIME 100 // Tempo (ms) para ignorar o ruído mecânico do fader
bool rawTouch[8] = {false, false, false, false, false, false, false, false};
bool validTouch[8] = {false, false, false, false, false, false, false, false};
unsigned long releaseTime[8] = {0, 0, 0, 0, 0, 0, 0, 0};

// --------------------------------------------------------
// SETUP TOUCH
// --------------------------------------------------------
void setupTouch() {
  pinMode(IRQ_PIN, INPUT_PULLUP);
  Wire.begin(); 
  
  // Tenta iniciar o chip. Se der sucesso, injeta a calibração do metal.
  if (cap.begin(0x5A)) {
    cap.setAutoconfig(true);
    cap.setThresholds(85, 50); 
  }
}

// --------------------------------------------------------
// LÓGICA DE LEITURA E FILTRO
// --------------------------------------------------------
void checkTouch() {
  // 1. LÊ O SENSOR APENAS SE O ALARME (IRQ) DISPARAR
  if (digitalRead(IRQ_PIN) == LOW) {
    uint16_t touchData = cap.touched();
    
    // Varre as portas 0 a 7
    for (int i = 0; i < 8; i++) {
      rawTouch[i] = (touchData & (1 << i)); 
    }
  }

  // 2. APLICA O FILTRO DE TEMPO (DEBOUNCE) PARA IGNORAR RUÍDO
  unsigned long currentTime = millis();

  for (int i = 0; i < 8; i++) {
    
    if (rawTouch[i] == true) {
      // O DEDO ESTÁ NO METAL (Instantâneo)
      releaseTime[i] = currentTime; // Segura o relógio
      
      if (validTouch[i] == false) {
        validTouch[i] = true;
        faders[i].isTouched = true; // Salva para a aba de Motores saber que tem dedo ali
        
        // Dispara o MIDI de Seleção pro REAPER
        midiHandler.sendNoteOn(1, faders[i].touchNote, 127);
      }
      
    } else {
      // O DEDO SAIU DO METAL (Aguarda o tempo de carência para ter certeza)
      if (validTouch[i] == true && (currentTime - releaseTime[i] > DEBOUNCE_TIME)) {
        validTouch[i] = false;
        faders[i].isTouched = false; // Libera o motor para voltar a trabalhar
        
        // Dispara o MIDI de Soltura pro REAPER
        midiHandler.sendNoteOn(1, faders[i].touchNote, 0);
      }
    }
  }
}