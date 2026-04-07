/*
 * ===================================================================================
 * Project:       Vertex Fader
 * File:          MackieMCU.h
 * Description:   Dicionário de constantes do protocolo Mackie Control Universal (MCU).
 * Substitui o uso de "números mágicos" no código.
 * ===================================================================================
 */

#pragma once
#include <Arduino.h>

// --------------------------------------------------------
// ENCODER - CC (Control Change)
// --------------------------------------------------------
constexpr uint8_t V_POT_1 = 0x10;  // V-Pot 1 (Relative)
constexpr uint8_t V_POT_2 = 0x11;  // V-Pot 2 (Relative)
constexpr uint8_t V_POT_3 = 0x12;  // V-Pot 3 (Relative)
constexpr uint8_t V_POT_4 = 0x13;  // V-Pot 4 (Relative)
constexpr uint8_t V_POT_5 = 0x14;  // V-Pot 5 (Relative)
constexpr uint8_t V_POT_6 = 0x15;  // V-Pot 6 (Relative)
constexpr uint8_t V_POT_7 = 0x16;  // V-Pot 7 (Relative)
constexpr uint8_t V_POT_8 = 0x17;  // V-Pot 8 (Relative)

// --------------------------------------------------------
// BUTTONS & TOUCH - NOTES (Note On/Off)
// --------------------------------------------------------
constexpr uint8_t REC_RDY_1 = 0x00;
constexpr uint8_t REC_RDY_2 = 0x01;
constexpr uint8_t REC_RDY_3 = 0x02;
constexpr uint8_t REC_RDY_4 = 0x03;
constexpr uint8_t REC_RDY_5 = 0x04;
constexpr uint8_t REC_RDY_6 = 0x05;
constexpr uint8_t REC_RDY_7 = 0x06;
constexpr uint8_t REC_RDY_8 = 0x07;

constexpr uint8_t SOLO_1 = 0x08;
constexpr uint8_t SOLO_2 = 0x09;
constexpr uint8_t SOLO_3 = 0x0A;
constexpr uint8_t SOLO_4 = 0x0B;
constexpr uint8_t SOLO_5 = 0x0C;
constexpr uint8_t SOLO_6 = 0x0D;
constexpr uint8_t SOLO_7 = 0x0E;
constexpr uint8_t SOLO_8 = 0x0F;

constexpr uint8_t MUTE_1 = 0x10;
constexpr uint8_t MUTE_2 = 0x11;
constexpr uint8_t MUTE_3 = 0x12;
constexpr uint8_t MUTE_4 = 0x13;
constexpr uint8_t MUTE_5 = 0x14;
constexpr uint8_t MUTE_6 = 0x15;
constexpr uint8_t MUTE_7 = 0x16;
constexpr uint8_t MUTE_8 = 0x17;

constexpr uint8_t SELECT_1 = 0x18;
constexpr uint8_t SELECT_2 = 0x19;
constexpr uint8_t SELECT_3 = 0x1A;
constexpr uint8_t SELECT_4 = 0x1B;
constexpr uint8_t SELECT_5 = 0x1C;
constexpr uint8_t SELECT_6 = 0x1D;
constexpr uint8_t SELECT_7 = 0x1E;
constexpr uint8_t SELECT_8 = 0x1F;

// --- NAVEGAÇÃO E BANCOS ---
constexpr uint8_t BANK_LEFT = 0x2E;
constexpr uint8_t BANK_RIGHT = 0x2F;
constexpr uint8_t CHANNEL_LEFT = 0x30;
constexpr uint8_t CHANNEL_RIGHT = 0x31;

// --- TRANSPORTE ---
constexpr uint8_t REWIND = 0x5B;
constexpr uint8_t FAST_FWD = 0x5C;
constexpr uint8_t STOP = 0x5D;
constexpr uint8_t PLAY = 0x5E;
constexpr uint8_t RECORD = 0x5F;

// --- FADER TOUCH (O Segredo da Automação) ---
constexpr uint8_t FADER_TOUCH_1 = 0x68;
constexpr uint8_t FADER_TOUCH_2 = 0x69;
constexpr uint8_t FADER_TOUCH_3 = 0x6A;
constexpr uint8_t FADER_TOUCH_4 = 0x6B;
constexpr uint8_t FADER_TOUCH_5 = 0x6C;
constexpr uint8_t FADER_TOUCH_6 = 0x6D;
constexpr uint8_t FADER_TOUCH_7 = 0x6E;
constexpr uint8_t FADER_TOUCH_8 = 0x6F;
constexpr uint8_t FADER_TOUCH_MASTER = 0x70;