// ===================================================== 
// Vertex Fader V1.4 Firmware
// Desenvolvido por Marlon Rodrigues
// =====================================================

#include <Arduino.h>
#include <Wire.h>

#define ESP32_HOST_MIDI_NO_USB_HOST
#include <ESP32_Host_MIDI.h>
#include <USBDeviceConnection.h>

#include "Mackie.h"
#include "Fader.h"
#include "Touch.h"
#include "Button.h"
#include "Encoder.h"

USBDeviceConnection usbMIDI("Vertex Controller");
static int lastProcessedMidiIndex = 0;

// =====================================================
// SETUP
// =====================================================
void setup() {

  Wire.begin();
  Wire.setClock(400000);

  midiHandler.addTransport(&usbMIDI);
  usbMIDI.begin();

  MIDIHandlerConfig cfg;
  midiHandler.begin(cfg);

  setupTouch();
  setupBotoes();
  setupEncoders();

  setupFaders();
}

// =====================================================
// LOOP PRINCIPAL
// =====================================================
void loop() {

  // =====================================================
  // 1. MIDI TASK (prioridade máxima)
  // =====================================================
  midiHandler.task();

  // =====================================================
  // 2. CONSUMO SEGURO DE EVENTOS MIDI
  // =====================================================
  const auto &queue = midiHandler.getQueue();

  for (const auto &ev : queue) {
    if (ev.index <= lastProcessedMidiIndex) continue;

    if (ev.statusCode == MIDI_PITCH_BEND) {

      int channel = ev.channel0 + 1;

      if (channel > 0 && channel <= NUM_FADERS) {
        updateFaderTarget(channel, ev.pitchBend14);
      }
    }

    lastProcessedMidiIndex = ev.index;
  }

  // =====================================================
  // 3. INPUTS (expansões I2C)
  // =====================================================
  processBotoes();
  processEncoders();

  // =====================================================
  // 4. TOUCH (controle de estado crítico)
  // =====================================================
  checkTouch();

  // =====================================================
  // 5. FEEDBACK FÍSICO DO RP2040
  // =====================================================
  readFaderPositions();

  // =====================================================
  // 6. PLACEHOLDER (mantido só por compatibilidade)
  // =====================================================
  // ESP32 não executa controle de motor
}
