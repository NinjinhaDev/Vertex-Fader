#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPR121.h>

#define IRQ_PIN 6

Adafruit_MPR121 cap = Adafruit_MPR121();

const unsigned long TOUCH_RELEASE_DEBOUNCE_MS = 150;
const unsigned long TOUCH_IDLE_POLL_MS = 20;
const unsigned long TOUCH_ACTIVE_POLL_MS = 5;

bool touchControllerReady = false;
bool rawTouch[NUM_FADERS] = {false};
bool validTouch[NUM_FADERS] = {false};
unsigned long releaseTime[NUM_FADERS] = {0};
unsigned long lastTouchPollTime = 0;

bool hasAnyValidTouch() {
  for (int i = 0; i < NUM_FADERS; i++) {
    if (validTouch[i]) {
      return true;
    }
  }

  return false;
}

void refreshTouchState(unsigned long currentTime) {
  const bool activeTouch = hasAnyValidTouch();
  const unsigned long pollInterval = activeTouch ? TOUCH_ACTIVE_POLL_MS : TOUCH_IDLE_POLL_MS;

  if (digitalRead(IRQ_PIN) != LOW && (currentTime - lastTouchPollTime) < pollInterval) {
    return;
  }

  const uint16_t touchData = cap.touched();
  lastTouchPollTime = currentTime;

  for (int i = 0; i < NUM_FADERS; i++) {
    rawTouch[i] = (touchData & (1 << i)) != 0;
  }
}

void setupTouch() {
  pinMode(IRQ_PIN, INPUT_PULLUP);

  touchControllerReady = cap.begin(0x5A);

  if (touchControllerReady) {
    cap.setAutoconfig(true);
    cap.setThresholds(60, 30);
  }
}

void checkTouch() {
  if (!touchControllerReady) {
    return;
  }

  unsigned long currentTime = millis();
  refreshTouchState(currentTime);

  for (int i = 0; i < NUM_FADERS; i++) {
    if (rawTouch[i]) {
      releaseTime[i] = currentTime;

      if (!validTouch[i]) {
        validTouch[i] = true;
        beginFaderTouch(i, currentTime);

        sendTouchToRP2040(i, true);
        midiHandler.sendNoteOn(1, faders[i].touchNote, 127);
      }
    } else if (validTouch[i] && (currentTime - releaseTime[i] > TOUCH_RELEASE_DEBOUNCE_MS)) {
      validTouch[i] = false;
      finishFaderTouch(i);

      sendTouchToRP2040(i, false);
      midiHandler.sendNoteOff(1, faders[i].touchNote, 0);
    }
  }
}
