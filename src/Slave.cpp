/*
 * ===================================================================================
 * Vertex Fader - Slave Firmware
 * Desenvolvido por Marlon Rodrigues
 * ===================================================================================
 */

#include <Arduino.h>

// =========================
// PINOS MOTORES
// =========================
const int motorPins[8][2] = {
  {0, 1}, {2, 3}, {4, 5}, {6, 7},
  {8, 9}, {10, 11}, {12, 13}, {14, 15}
};

// =========================
// MUX
// =========================
const int MUX_S0 = 20;
const int MUX_S1 = 19;
const int MUX_S2 = 18;
const int MUX_SIG = 26;

// =========================
// CALIBRACAO
// =========================
int minPhysical[8] = {0};
int maxPhysical[8] = {4095};

// =========================
// ESTADO DOS FADERS
// =========================
float rawPos[8] = {0};
int currentPos[8] = {0};
int targetPos[8] = {0};
int lastSentPos[8] = {0};
byte lastDriveDir[8] = {0};
byte lastDriveSpeed[8] = {0};
bool positionInitialized[8] = {false};

bool isTouched[8] = {false};
bool isMoving[8] = {false};

// =========================
// TOUCH TIMEOUT
// =========================
unsigned long lastTouchMsg[8] = {0};
const unsigned long TOUCH_TIMEOUT = 80;

// =========================
// CONTROLE MOTOR
// =========================
const int MOTOR_START_GATE = 6;          // Nao acorda o motor por erro muito pequeno
const int MOTOR_DEADZONE = 10;           // Zona final onde o motor para
const int MOTOR_BRAKE_ZONE = 250;        // Zona onde reduz velocidade
const int MOTOR_MIN_SPEED = 160;         // Evita rodar abaixo da faixa de torque util
const int MOTOR_MAX_SPEED = 255;
const int POSITION_REPORT_DEADBAND = 10; // Filtra feedback serial do ADC
const int MOTOR_COMMAND_DEADBAND = 5;    // Evita ficar reescrevendo PWM por microvariacao

const float EMA_ALPHA = 0.4f;

unsigned long lastPhysicsUpdate = 0;

// =====================================================
// HELPERS
// =====================================================
void stopMotor(byte index) {
  analogWrite(motorPins[index][0], 0);
  analogWrite(motorPins[index][1], 0);
  lastDriveDir[index] = 0;
  lastDriveSpeed[index] = 0;
}

void selectMuxChannel(byte channel) {
  digitalWrite(MUX_S0, bitRead(channel, 0));
  digitalWrite(MUX_S1, bitRead(channel, 1));
  digitalWrite(MUX_S2, bitRead(channel, 2));
  delayMicroseconds(10);
}

int readStableFader(byte channel) {
  selectMuxChannel(channel);

  // A primeira leitura apos trocar o MUX costuma vir contaminada pelo canal anterior.
  analogRead(MUX_SIG);
  delayMicroseconds(5);

  return analogRead(MUX_SIG);
}

void applyMotorCommand(byte index, byte dir, byte speed, bool force = false) {
  if (dir == 0) {
    if (!force && lastDriveDir[index] == 0) {
      return;
    }

    stopMotor(index);
    return;
  }

  speed = constrain(speed, MOTOR_MIN_SPEED, MOTOR_MAX_SPEED);

  if (!force &&
      dir == lastDriveDir[index] &&
      abs((int)speed - (int)lastDriveSpeed[index]) < MOTOR_COMMAND_DEADBAND) {
    return;
  }

  if (dir == 1) {
    analogWrite(motorPins[index][0], speed);
    analogWrite(motorPins[index][1], 0);
  } else {
    analogWrite(motorPins[index][0], 0);
    analogWrite(motorPins[index][1], speed);
  }

  lastDriveDir[index] = dir;
  lastDriveSpeed[index] = speed;
}

// =====================================================
// CALIBRACAO
// =====================================================
void calibrateFaders() {
  // Sobe tudo
  for (int i = 0; i < 8; i++) {
    analogWrite(motorPins[i][0], 255);
    analogWrite(motorPins[i][1], 0);
  }
  delay(400);

  // MAX
  for (int i = 0; i < 8; i++) {
    long sum = 0;
    for (int r = 0; r < 16; r++) {
      sum += readStableFader(i);
    }
    maxPhysical[i] = (sum / 16) - 30;
  }

  // Desce tudo
  for (int i = 0; i < 8; i++) {
    analogWrite(motorPins[i][0], 0);
    analogWrite(motorPins[i][1], 255);
  }
  delay(400);

  // MIN
  for (int i = 0; i < 8; i++) {
    long sum = 0;
    for (int r = 0; r < 16; r++) {
      sum += readStableFader(i);
    }
    minPhysical[i] = (sum / 16) + 30;
  }

  // Stop
  for (int i = 0; i < 8; i++) {
    stopMotor(i);
  }
}

// =====================================================
// SETUP
// =====================================================
void setup() {
  Serial1.setTX(16);
  Serial1.setRX(17);
  Serial1.begin(1000000);

  analogReadResolution(12);
  analogWriteFreq(25000);

  for (int i = 0; i < 8; i++) {
    pinMode(motorPins[i][0], OUTPUT);
    pinMode(motorPins[i][1], OUTPUT);
    stopMotor(i);
  }

  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT);
  pinMode(MUX_SIG, INPUT);
}

// =====================================================
// LOOP PRINCIPAL
// =====================================================
void loop() {
  // =========================
  // LEITURA FADERS
  // =========================
  for (int i = 0; i < 8; i++) {
    int raw = readStableFader(i);

    if (!positionInitialized[i]) {
      rawPos[i] = raw;
      positionInitialized[i] = true;
    } else {
      rawPos[i] = (EMA_ALPHA * raw) + ((1.0f - EMA_ALPHA) * rawPos[i]);
    }

    int mapped = map((int)rawPos[i], minPhysical[i], maxPhysical[i], 0, 4095);
    currentPos[i] = constrain(mapped, 0, 4095);
  }

  // =========================
  // SERIAL INPUT
  // =========================
  while (Serial1.available() >= 4) {
    byte header = Serial1.read();

    // TARGET POSITION
    if (header == 0xFF) {
      byte index = Serial1.read();
      int value = (Serial1.read() << 8) | Serial1.read();

      if (index < 8) {
        targetPos[index] = constrain(value, 0, 4095);

        if (abs(targetPos[index] - currentPos[index]) > MOTOR_START_GATE) {
          isMoving[index] = true;
        } else {
          isMoving[index] = false;
          stopMotor(index);
        }
      }
    }

    // TOUCH STATE
    else if (header == 0xFD) {
      byte index = Serial1.read();
      byte state = Serial1.read();
      Serial1.read();

      if (index < 8) {
        isTouched[index] = (state == 1);
        lastTouchMsg[index] = millis();

        if (isTouched[index]) {
          isMoving[index] = false;
          stopMotor(index);
        }
      }
    }

    // CALIBRATION
    else if (header == 0xFC) {
      Serial1.read();
      Serial1.read();
      Serial1.read();
      calibrateFaders();
    }
  }

  // =========================
  // MOTOR CONTROL (1ms LOOP)
  // =========================
  if (millis() - lastPhysicsUpdate >= 1) {
    lastPhysicsUpdate = millis();

    for (int i = 0; i < 8; i++) {
      bool touchActive =
        isTouched[i] &&
        (millis() - lastTouchMsg[i] < TOUCH_TIMEOUT);

      // Stop imediato ao tocar
      if (touchActive) {
        stopMotor(i);
        continue;
      }

      if (!isMoving[i]) {
        stopMotor(i);
        continue;
      }

      int error = targetPos[i] - currentPos[i];
      int absError = abs(error);

      if (absError <= MOTOR_DEADZONE) {
        isMoving[i] = false;
        applyMotorCommand(i, 0, 0, true);
        continue;
      }

      int speed = MOTOR_MAX_SPEED;

      if (absError < MOTOR_BRAKE_ZONE) {
        speed = map(absError, MOTOR_DEADZONE, MOTOR_BRAKE_ZONE, MOTOR_MIN_SPEED, MOTOR_MAX_SPEED);
      }

      speed = constrain(speed, MOTOR_MIN_SPEED, MOTOR_MAX_SPEED);

      if (error > 0) {
        applyMotorCommand(i, 1, speed);
      } else {
        applyMotorCommand(i, 2, speed);
      }
    }
  }

  // =========================
  // FEEDBACK PARA ESP32
  // =========================
  for (int i = 0; i < 8; i++) {
    if (abs(currentPos[i] - lastSentPos[i]) > POSITION_REPORT_DEADBAND) {
      byte packet[4] = {
        0xFE,
        (byte)i,
        (byte)((currentPos[i] >> 8) & 0xFF),
        (byte)(currentPos[i] & 0xFF)
      };

      Serial1.write(packet, 4);
      lastSentPos[i] = currentPos[i];
    }
  }
}
