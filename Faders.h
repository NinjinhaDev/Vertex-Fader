/*
 * ===================================================================================
 * Project:       Vertex Fader
 * Author:        Marlon
 * Date:          March / 2026
 * Repository:    https://github.com/NinjinhaDev/Vertex-Fader
 * Version:       1.0.0
 * * Description: 
 * Controlador MIDI de alta performance desenvolvido para o ESP32-S3.
 * Possui arquitetura modular para controle de Faders Motorizados usando
 * gerador PWM I2C (PCA9685) e drivers de ponte-H (DRV8833). Inclui 
 * varredura analógica via CD4051, calibração automática de limites físicos,
 * detecção capacitiva de toque nativa (Touch), filtro EMA anti-ruído
 * e integração de automação via protocolo Mackie Control Universal (MCU).
 * * Credits & Acknowledgments:
 * - Saulo Veríssimo: Pela biblioteca 'ESP32-Host-MIDI'.
 * - Gustavo Silveira (MusicoNerd): Pela arquitetura de debounce e calibração.
 * - Adafruit Industries: Pelas bibliotecas base de controle I2C e PWM.
 * * License:       MIT License (Open Source)
 * ===================================================================================
 */

#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "MackieMCU.h"  // Nosso dicionário de protocolo de estúdio

// --------------------------------------------------------
// PCA9685 INSTANCE (Controlador de Motores I2C)
// --------------------------------------------------------
Adafruit_PWMServoDriver pca = Adafruit_PWMServoDriver(0x40);

// --------------------------------------------------------
// HARDWARE SETTINGS (Pinos e Limites)
// --------------------------------------------------------
const int MUX_S0 = 10;
const int MUX_S1 = 11;
const int MUX_S2 = 12;
const int MUX_SIG = 5;  // Pino Analógico (ADC) do ESP32-S3

const int NUM_FADERS = 8;
const int READ_THRESHOLD = 15;      // Threshold final para envio do MIDI
const int MOTOR_DEADZONE = 40;      // Tolerância para o motor parar (Evita tremor)
const int TOUCH_THRESHOLD = 30000;  // Limiar do Touch do ESP32-S3 (Ajuste na prática)
const float EMA_ALPHA = 0.2;        // Peso do filtro Anti-Ruído (0.0 a 1.0). Menor = Mais suave

// --------------------------------------------------------
// FADER STRUCTURE (O estado físico e lógico de cada canal)
// --------------------------------------------------------
struct Fader {
  byte muxChannel;         // Porta no CD4051 (0 a 7)
  byte midiChannel;        // Canal MIDI (1 a 8) para enviar PitchBend independente
  int physicalPosition;    // Leitura bruta atual do ADC
  float smoothedPosition;  // Leitura suavizada pela matemática do filtro EMA
  int lastSentPosition;    // Último valor de posição enviado para a DAW
  int targetPosition;      // Alvo solicitado pela DAW
  byte pcaPinA;            // Pino AIN1 no DRV8833 (Motor Sobe)
  byte pcaPinB;            // Pino AIN2 no DRV8833 (Motor Desce)
  byte touchPin;           // GPIO do ESP32-S3 com suporte a TouchRead
  byte touchNote;          // A nota MIDI Mackie específica para automação Touch
  int minPhysical;         // Chão físico (Calibrado no boot)
  int maxPhysical;         // Teto físico (Calibrado no boot)
  bool isTouched;          // Flag atual de dedo no knob
  bool lastTouchedState;   // Memória para disparar o MIDI apenas 1x ao tocar/soltar
  bool isMoving;           // Flag de motor ativado buscando alvo
};

// Mapeamento dos 8 Faders
// NOTA: Os 'touchPin' usam GPIOs do S3 que suportam toque. Ajuste para a sua placa.
Fader faders[NUM_FADERS] = {
  // Mux| Ch| Phys | Smooth | LastSent | Target | PcaA | PcaB | T_Pin | T_Note         | Min | Max  | isTch | lastTch | isMov
  { 0, 1, 0, 0.0, 0, 0, 0, 1, 1, FADER_TOUCH_1, 0, 4095, false, false, false },    // Fader 1
  { 1, 2, 0, 0.0, 0, 0, 2, 3, 2, FADER_TOUCH_2, 0, 4095, false, false, false },    // Fader 2
  { 2, 3, 0, 0.0, 0, 0, 4, 5, 3, FADER_TOUCH_3, 0, 4095, false, false, false },    // Fader 3
  { 3, 4, 0, 0.0, 0, 0, 6, 7, 4, FADER_TOUCH_4, 0, 4095, false, false, false },    // Fader 4
  { 4, 5, 0, 0.0, 0, 0, 8, 9, 6, FADER_TOUCH_5, 0, 4095, false, false, false },    // Fader 5
  { 5, 6, 0, 0.0, 0, 0, 10, 11, 7, FADER_TOUCH_6, 0, 4095, false, false, false },  // Fader 6
  { 6, 7, 0, 0.0, 0, 0, 12, 13, 8, FADER_TOUCH_7, 0, 4095, false, false, false },  // Fader 7
  { 7, 8, 0, 0.0, 0, 0, 14, 15, 9, FADER_TOUCH_8, 0, 4095, false, false, false }   // Fader 8
};

// --------------------------------------------------------
// MUX SELECTION
// --------------------------------------------------------
void selectMuxChannel(byte channel) {
  digitalWrite(MUX_S0, bitRead(channel, 0));
  digitalWrite(MUX_S1, bitRead(channel, 1));
  digitalWrite(MUX_S2, bitRead(channel, 2));
  delayMicroseconds(15);  // Tempo para o sinal estabilizar
}

// --------------------------------------------------------
// AUTO-CALIBRATION (Roda 1x no void setup)
// --------------------------------------------------------
void calibrateFaders() {
  Serial.println("Iniciando calibração dos Faders...");

  for (int i = 0; i < NUM_FADERS; i++) {
    // 1. Envia o fader para o TETO
    pca.setPWM(faders[i].pcaPinA, 0, 4095);
    pca.setPWM(faders[i].pcaPinB, 0, 0);
    delay(350);                           // Tempo mecânico para chegar no topo
    pca.setPWM(faders[i].pcaPinA, 0, 0);  // Desliga motor
    delay(50);

    selectMuxChannel(faders[i].muxChannel);
    faders[i].maxPhysical = analogRead(MUX_SIG) - 20;  // Lê e dá margem de segurança

    // 2. Envia o fader para o CHÃO
    pca.setPWM(faders[i].pcaPinA, 0, 0);
    pca.setPWM(faders[i].pcaPinB, 0, 4095);
    delay(350);                           // Tempo mecânico para chegar no fundo
    pca.setPWM(faders[i].pcaPinB, 0, 0);  // Desliga motor
    delay(50);

    selectMuxChannel(faders[i].muxChannel);
    faders[i].minPhysical = analogRead(MUX_SIG) + 20;  // Lê e dá margem de segurança

    // Inicializa o filtro suavizador com a posição real de repouso atual
    faders[i].smoothedPosition = faders[i].minPhysical;

    Serial.printf("Fader %d calibrado. Min: %d | Max: %d\n", i + 1, faders[i].minPhysical, faders[i].maxPhysical);
  }
  Serial.println("Calibração concluída!");
}

// --------------------------------------------------------
// INITIALIZATION
// --------------------------------------------------------
void setupFaders() {
  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT);

  Wire.begin();
  pca.begin();
  pca.setOscillatorFrequency(27000000);
  pca.setPWMFreq(1000);  // 1kHz é silencioso para motores DC

  // Executa a calibração física ao ligar
  calibrateFaders();
}

// --------------------------------------------------------
// TOUCH SENSING (Hardware Protection + DAW Automation)
// --------------------------------------------------------
void checkTouch() {
  for (int i = 0; i < NUM_FADERS; i++) {
    uint32_t touchValue = touchRead(faders[i].touchPin);

    // O valor do ESP32 cai quando há capacitância (dedo) no pino
    faders[i].isTouched = (touchValue < TOUCH_THRESHOLD);

    // 1. PROTEÇÃO DE HARDWARE (Desarma a ponte H)
    if (faders[i].isTouched) {
      faders[i].isMoving = false;           // Cancela qualquer movimento agendado
      pca.setPWM(faders[i].pcaPinA, 0, 0);  // Trava o motor instantaneamente
      pca.setPWM(faders[i].pcaPinB, 0, 0);
    }

    // 2. INTEGRAÇÃO MACKIE CONTROL (Avisa a DAW para gravar automação)
    if (faders[i].isTouched != faders[i].lastTouchedState) {

      if (faders[i].isTouched) {
        // Acabou de encostar: Envia Note ON, Velocidade 127
        midiHandler.sendNoteOn(1, faders[i].touchNote, 127);
      } else {
        // Acabou de soltar: Envia Note OFF (Note ON com Velocidade 0)
        midiHandler.sendNoteOn(1, faders[i].touchNote, 0);
      }

      // Atualiza a memória para não flodar a porta MIDI
      faders[i].lastTouchedState = faders[i].isTouched;
    }
  }
}

// --------------------------------------------------------
// READING LOGIC (Fader -> DAW) com Filtro EMA Anti-Ruído
// --------------------------------------------------------
void readFaderPositions() {
  for (int i = 0; i < NUM_FADERS; i++) {

    // Só lê e envia MIDI se o motor não estiver buscando alvo (evita feedback loop)
    if (!faders[i].isMoving) {
      selectMuxChannel(faders[i].muxChannel);

      int rawReading = analogRead(MUX_SIG);

      // Aplica o filtro EMA (Exponential Moving Average)
      // Mistura uma fração do valor novo com a maior parte do valor antigo
      faders[i].smoothedPosition = (EMA_ALPHA * rawReading) + ((1.0 - EMA_ALPHA) * faders[i].smoothedPosition);

      // Verifica se a mudança filtrada passou do Threshold antes de enviar MIDI
      if (abs((int)faders[i].smoothedPosition - faders[i].lastSentPosition) > READ_THRESHOLD) {

        // Atualiza a memória da última posição enviada
        faders[i].lastSentPosition = (int)faders[i].smoothedPosition;

        // Mapeia para 14-bits (Pitchbend) usando os limites reais calibrados daquele fader
        int midiValue = map((int)faders[i].smoothedPosition, faders[i].minPhysical, faders[i].maxPhysical, 0, 16383);
        midiValue = constrain(midiValue, 0, 16383);  // Garante a integridade do protocolo de 14 bits

        // Envia PITCHBEND passando o CANAL e o VALOR
        midiHandler.sendPitchBend(faders[i].midiChannel, midiValue);
      }
    }
  }
}

// --------------------------------------------------------
// TARGET UPDATE (DAW -> Fader)
// --------------------------------------------------------
void updateFaderTarget(byte receivedChannel, uint16_t receivedMidiValue) {
  for (int i = 0; i < NUM_FADERS; i++) {
    // Busca qual fader está associado a este canal MIDI (Pitchbend)
    if (faders[i].midiChannel == receivedChannel) {

      // Converte os 14-bits (16383) de volta para a mecânica do ADC do ESP32 calibrado
      faders[i].targetPosition = map(receivedMidiValue, 0, 16383, faders[i].minPhysical, faders[i].maxPhysical);
      faders[i].isMoving = true;
      break;
    }
  }
}

// --------------------------------------------------------
// MOTOR EXECUTION (Algoritmo de aproximação PID simplificado)
// --------------------------------------------------------
void processMotors() {
  for (int i = 0; i < NUM_FADERS; i++) {

    // Motor só move se tiver ordem E se não houver dedo no fader metálico
    if (faders[i].isMoving && !faders[i].isTouched) {

      selectMuxChannel(faders[i].muxChannel);
      // Aqui usamos a leitura bruta (rápida) para o motor não ter delay de resposta
      faders[i].physicalPosition = analogRead(MUX_SIG);

      int positionError = faders[i].targetPosition - faders[i].physicalPosition;

      // Se entrou na zona morta (chegou no alvo), desliga o motor
      if (abs(positionError) <= MOTOR_DEADZONE) {
        pca.setPWM(faders[i].pcaPinA, 0, 0);
        pca.setPWM(faders[i].pcaPinB, 0, 0);

        // Atualiza o suavizador para ele não "puxar" o fader na próxima leitura humana
        faders[i].smoothedPosition = faders[i].physicalPosition;
        faders[i].lastSentPosition = faders[i].physicalPosition;
        faders[i].isMoving = false;
      }
      // Precisa Subir (Erro positivo)
      else if (positionError > 0) {
        pca.setPWM(faders[i].pcaPinA, 0, 4095);
        pca.setPWM(faders[i].pcaPinB, 0, 0);
      }
      // Precisa Descer (Erro negativo)
      else {
        pca.setPWM(faders[i].pcaPinA, 0, 0);
        pca.setPWM(faders[i].pcaPinB, 0, 4095);
      }
    }
  }
}