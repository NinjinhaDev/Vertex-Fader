#pragma once
#include <Arduino.h>
#include "Mackie.h"

const int NUM_FADERS = 8;

const unsigned long TOUCH_REFERENCE_CAPTURE_MS = 20;  // Janela curta para estabilizar apos o toque
const uint8_t TOUCH_REFERENCE_MIN_SAMPLES = 3;        // Minimo de amostras para travar a referencia
const int TOUCH_NOISE_GATE = 12;                      // Ignora micro variacoes no inicio do toque
const int MIDI_DEADBAND = 15;                         // Evita flood MIDI por micro variacao
const unsigned long POST_TOUCH_AUTOMATION_HOLDOFF_MS = 250;  // Segura o retorno da DAW logo apos soltar

enum class FaderInputState : uint8_t {
  Idle = 0,
  TouchPriming,
  TouchHold,
  TouchActive
};

struct Fader {
  byte midiChannel;
  int currentPosition;
  int lastSentPosition;
  int targetPosition;
  byte touchNote;

  int minLogical = 0;
  int maxLogical = 4095;

  bool isTouched = false;
  bool wasTouched = false;              // Mantido por compatibilidade
  int touchOffset = 0;
  int touchReferencePosition = 0;
  bool touchMovementUnlocked = false;   // Mantido por compatibilidade
  unsigned long touchStartTime = 0;
  unsigned long touchReleaseTime = 0;

  FaderInputState state = FaderInputState::Idle;
  long touchReferenceAccumulator = 0;
  uint8_t touchReferenceSamples = 0;
};

Fader faders[NUM_FADERS] = {
  {1, 0, 0, 0, FADER_TOUCH_1, 0, 4095, false, false, 0, 0, false, 0, 0, FaderInputState::Idle, 0, 0},
  {2, 0, 0, 0, FADER_TOUCH_2, 0, 4095, false, false, 0, 0, false, 0, 0, FaderInputState::Idle, 0, 0},
  {3, 0, 0, 0, FADER_TOUCH_3, 0, 4095, false, false, 0, 0, false, 0, 0, FaderInputState::Idle, 0, 0},
  {4, 0, 0, 0, FADER_TOUCH_4, 0, 4095, false, false, 0, 0, false, 0, 0, FaderInputState::Idle, 0, 0},
  {5, 0, 0, 0, FADER_TOUCH_5, 0, 4095, false, false, 0, 0, false, 0, 0, FaderInputState::Idle, 0, 0},
  {6, 0, 0, 0, FADER_TOUCH_6, 0, 4095, false, false, 0, 0, false, 0, 0, FaderInputState::Idle, 0, 0},
  {7, 0, 0, 0, FADER_TOUCH_7, 0, 4095, false, false, 0, 0, false, 0, 0, FaderInputState::Idle, 0, 0},
  {8, 0, 0, 0, FADER_TOUCH_8, 0, 4095, false, false, 0, 0, false, 0, 0, FaderInputState::Idle, 0, 0}
};

void sendTargetToRP2040(byte index, int target) {
  if (index >= NUM_FADERS) return;

  target = constrain(target, 0, 4095);

  byte packet[4] = {
    0xFF,
    index,
    (byte)((target >> 8) & 0xFF),
    (byte)(target & 0xFF)
  };

  Serial1.write(packet, 4);
}

void sendTouchToRP2040(byte index, bool touched) {
  if (index >= NUM_FADERS) return;

  byte packet[4] = {
    0xFD,
    index,
    (byte)(touched ? 1 : 0),
    0x00
  };

  Serial1.write(packet, 4);
}

void resetTouchTracking(Fader &f) {
  f.wasTouched = false;
  f.touchOffset = 0;
  f.touchMovementUnlocked = false;
  f.touchReferenceAccumulator = 0;
  f.touchReferenceSamples = 0;
  f.state = FaderInputState::Idle;
}

void beginFaderTouch(byte index, unsigned long now) {
  if (index >= NUM_FADERS) return;

  Fader &f = faders[index];
  f.isTouched = true;
  f.wasTouched = true;
  f.touchStartTime = now;
  f.touchReleaseTime = 0;
  f.touchReferencePosition = f.currentPosition;
  f.touchReferenceAccumulator = f.currentPosition;
  f.touchReferenceSamples = 1;
  f.touchOffset = 0;
  f.touchMovementUnlocked = false;
  f.state = FaderInputState::TouchPriming;
}

void finishFaderTouch(byte index) {
  if (index >= NUM_FADERS) return;

  Fader &f = faders[index];
  f.isTouched = false;
  f.touchReleaseTime = millis();
  resetTouchTracking(f);
  f.touchReferencePosition = f.currentPosition;

  // Sincroniza o target local com a posicao fisica ao soltar
  f.targetPosition = f.currentPosition;
  sendTargetToRP2040(index, f.currentPosition);
}

int mapPositionToPitchBend(const Fader &f, int physicalPosition) {
  const int logicalPosition = constrain(
    physicalPosition + f.touchOffset,
    f.minLogical,
    f.maxLogical
  );

  return constrain(
    map(logicalPosition, 0, 4095, -8192, 8191),
    -8192,
    8191
  );
}

void updateTouchedFader(Fader &f, unsigned long now) {
  if (f.state == FaderInputState::Idle) {
    f.touchStartTime = now;
    f.touchReferencePosition = f.currentPosition;
    f.touchReferenceAccumulator = f.currentPosition;
    f.touchReferenceSamples = 1;
    f.state = FaderInputState::TouchPriming;
  }

  if (f.state == FaderInputState::TouchPriming) {
    f.touchReferenceAccumulator += f.currentPosition;

    if (f.touchReferenceSamples < 255) {
      f.touchReferenceSamples++;
    }

    const bool windowElapsed = (now - f.touchStartTime) >= TOUCH_REFERENCE_CAPTURE_MS;
    const bool hasEnoughSamples = f.touchReferenceSamples >= TOUCH_REFERENCE_MIN_SAMPLES;

    if (!windowElapsed || !hasEnoughSamples) {
      return;
    }

    f.touchReferencePosition = (int)(f.touchReferenceAccumulator / f.touchReferenceSamples);
    f.touchOffset = f.targetPosition - f.touchReferencePosition;
    f.state = FaderInputState::TouchHold;
  }

  if (f.state == FaderInputState::TouchHold) {
    if (abs(f.currentPosition - f.touchReferencePosition) <= TOUCH_NOISE_GATE) {
      return;
    }

    f.touchMovementUnlocked = true;
    f.state = FaderInputState::TouchActive;
  }

  if (f.state != FaderInputState::TouchActive) {
    return;
  }

  const int midiValue = mapPositionToPitchBend(f, f.currentPosition);

  if (abs(midiValue - f.lastSentPosition) >= MIDI_DEADBAND) {
    f.lastSentPosition = midiValue;
    midiHandler.sendPitchBend(f.midiChannel, midiValue);
  }
}

void setupFaders() {
  Serial1.begin(1000000, SERIAL_8N1, 42, 41);

  for (int i = 0; i < NUM_FADERS; i++) {
    faders[i].currentPosition = 0;
    faders[i].lastSentPosition = 0;
    faders[i].targetPosition = 0;
    faders[i].isTouched = false;
    faders[i].touchReferencePosition = 0;
    faders[i].touchStartTime = 0;
    faders[i].touchReleaseTime = 0;
    resetTouchTracking(faders[i]);
  }

  byte packet[4] = { 0xFC, 0x00, 0x00, 0x00 };
  Serial1.write(packet, 4);
}

void readFaderPositions() {
  while (Serial1.available() >= 4) {
    byte header = Serial1.read();

    if (header == 0xFE) {
      byte index = Serial1.read();
      int incomingPos = (Serial1.read() << 8) | Serial1.read();

      if (index < NUM_FADERS) {
        Fader &f = faders[index];
        f.currentPosition = constrain(incomingPos, 0, 4095);

        if (f.isTouched) {
          updateTouchedFader(f, millis());
        } else {
          resetTouchTracking(f);
          f.touchReferencePosition = f.currentPosition;
        }
      }
    } else {
      Serial1.read();
      Serial1.read();
      Serial1.read();
    }
  }
}

void updateFaderTarget(byte receivedChannel, uint16_t receivedMidiValue) {
  int i = receivedChannel - 1;

  if (i >= 0 && i < NUM_FADERS) {
    if (faders[i].isTouched) return;  // Anti-feedback shield
    if (faders[i].touchReleaseTime != 0 &&
        (millis() - faders[i].touchReleaseTime) < POST_TOUCH_AUTOMATION_HOLDOFF_MS) {
      return;  // Segura o retorno imediato da DAW apos soltar
    }

    receivedMidiValue = constrain(receivedMidiValue, 0, 16383);
    faders[i].targetPosition = map(receivedMidiValue, 0, 16383, 0, 4095);

    sendTargetToRP2040(i, faders[i].targetPosition);
  }
}

void processMotors() {}
