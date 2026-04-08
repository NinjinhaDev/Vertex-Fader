/*
 * ===================================================================================
 * File:          Touch.h
 * Description:   Capacitive Touch (MPR121) + Auto-Select (Touch to Select)
 * ===================================================================================
 */

#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPR121.h>
#include "Mackie.h" // Incluído para puxar as notas SELECT_1 até SELECT_8

#define IRQ_PIN 6
Adafruit_MPR121 cap = Adafruit_MPR121();

#define DEBOUNCE_TIME 100 
bool rawTouch[8] = {false, false, false, false, false, false, false, false};
bool validTouch[8] = {false, false, false, false, false, false, false, false};
unsigned long releaseTime[8] = {0, 0, 0, 0, 0, 0, 0, 0};

void setupTouch() {
  pinMode(IRQ_PIN, INPUT_PULLUP);
  
  if (cap.begin(0x5A)) {
    cap.setAutoconfig(true);
    cap.setThresholds(85, 50); 
  }
}

void checkTouch() {
  if (digitalRead(IRQ_PIN) == LOW) {
    uint16_t touchData = cap.touched();
    for (int i = 0; i < 8; i++) {
      rawTouch[i] = (touchData & (1 << i)); 
    }
  }

  unsigned long currentTime = millis();

  for (int i = 0; i < 8; i++) {
    if (rawTouch[i] == true) {
      releaseTime[i] = currentTime; 
      
      if (validTouch[i] == false) {
        validTouch[i] = true;
        faders[i].isTouched = true; 
        
        // 1. O Touch original para a automação e corte do motor
        midiHandler.sendNoteOn(faders[i].touchNote, 127, 1); 
        
        // 2. A MÁGICA DO AUTO-SELECT: Grita pra DAW qual canal você pegou!
        midiHandler.sendNoteOn(SELECT_1 + i, 127, 1);
      }
      
    } else {
      if (validTouch[i] == true && (currentTime - releaseTime[i] > DEBOUNCE_TIME)) {
        validTouch[i] = false;
        faders[i].isTouched = false; 
        
        // 1. Solta o Touch original do motor
        midiHandler.sendNoteOn(faders[i].touchNote, 0, 1); 
        
        // 2. Solta o botão físico do Select
        midiHandler.sendNoteOn(SELECT_1 + i, 0, 1);
      }
    }
  }
}