/*
 * ===================================================================================
 * Project:       Vertex Fader
 * File:          MackieMCU.h
 * Description:   Dicionário COMPLETO do protocolo Mackie Control Universal (MCU).
 * ===================================================================================
 */

#pragma once
#include <Arduino.h>

// --------------------------------------------------------
// ENCODER - CC (Control Change) para Giração
// --------------------------------------------------------
constexpr uint8_t V_POT_1 = 0x10; 
constexpr uint8_t V_POT_2 = 0x11; 
constexpr uint8_t V_POT_3 = 0x12; 
constexpr uint8_t V_POT_4 = 0x13; 
constexpr uint8_t V_POT_5 = 0x14; 
constexpr uint8_t V_POT_6 = 0x15; 
constexpr uint8_t V_POT_7 = 0x16; 
constexpr uint8_t V_POT_8 = 0x17; 

// --------------------------------------------------------
// BUTTONS & TOUCH - NOTES (Note On/Off)
// --------------------------------------------------------
// --- CONTROLES DE CANAL ---
constexpr uint8_t REC_RDY_1 = 0x00; constexpr uint8_t REC_RDY_5 = 0x04;
constexpr uint8_t REC_RDY_2 = 0x01; constexpr uint8_t REC_RDY_6 = 0x05;
constexpr uint8_t REC_RDY_3 = 0x02; constexpr uint8_t REC_RDY_7 = 0x06;
constexpr uint8_t REC_RDY_4 = 0x03; constexpr uint8_t REC_RDY_8 = 0x07;

constexpr uint8_t SOLO_1 = 0x08; constexpr uint8_t SOLO_5 = 0x0C;
constexpr uint8_t SOLO_2 = 0x09; constexpr uint8_t SOLO_6 = 0x0D;
constexpr uint8_t SOLO_3 = 0x0A; constexpr uint8_t SOLO_7 = 0x0E;
constexpr uint8_t SOLO_4 = 0x0B; constexpr uint8_t SOLO_8 = 0x0F;

constexpr uint8_t MUTE_1 = 0x10; constexpr uint8_t MUTE_5 = 0x14;
constexpr uint8_t MUTE_2 = 0x11; constexpr uint8_t MUTE_6 = 0x15;
constexpr uint8_t MUTE_3 = 0x12; constexpr uint8_t MUTE_7 = 0x16;
constexpr uint8_t MUTE_4 = 0x13; constexpr uint8_t MUTE_8 = 0x17;

constexpr uint8_t SELECT_1 = 0x18; constexpr uint8_t SELECT_5 = 0x1C;
constexpr uint8_t SELECT_2 = 0x19; constexpr uint8_t SELECT_6 = 0x1D;
constexpr uint8_t SELECT_3 = 0x1A; constexpr uint8_t SELECT_7 = 0x1E;
constexpr uint8_t SELECT_4 = 0x1B; constexpr uint8_t SELECT_8 = 0x1F;

// --- V-POT CLICKS (Apertar o Encoder) ---
constexpr uint8_t V_POT_CLICK_1 = 0x20; constexpr uint8_t V_POT_CLICK_5 = 0x24;
constexpr uint8_t V_POT_CLICK_2 = 0x21; constexpr uint8_t V_POT_CLICK_6 = 0x25;
constexpr uint8_t V_POT_CLICK_3 = 0x22; constexpr uint8_t V_POT_CLICK_7 = 0x26;
constexpr uint8_t V_POT_CLICK_4 = 0x23; constexpr uint8_t V_POT_CLICK_8 = 0x27;

// --- BOTÕES DE ATRIBUIÇÃO (ASSIGN) ---
constexpr uint8_t ASSIGN_TRACK = 0x28;
constexpr uint8_t ASSIGN_SEND = 0x29;
constexpr uint8_t ASSIGN_PAN = 0x2A;
constexpr uint8_t ASSIGN_PLUGIN = 0x2B;
constexpr uint8_t ASSIGN_EQ = 0x2C;
constexpr uint8_t ASSIGN_INSTRUMENT = 0x2D;

// --- NAVEGAÇÃO E BANCOS ---
constexpr uint8_t BANK_LEFT = 0x2E;
constexpr uint8_t BANK_RIGHT = 0x2F;
constexpr uint8_t CHANNEL_LEFT = 0x30;
constexpr uint8_t CHANNEL_RIGHT = 0x31;

// --- NAVEGAÇÃO GERAL E ZOOM ---
constexpr uint8_t FLIP = 0x32;
constexpr uint8_t GLOBAL_VIEW = 0x33;
constexpr uint8_t NAME_VALUE = 0x34;
constexpr uint8_t SMPTE_BEATS = 0x35;

// --- FUNCTION KEYS (F1 - F8) ---
constexpr uint8_t F1 = 0x36; constexpr uint8_t F5 = 0x3A;
constexpr uint8_t F2 = 0x37; constexpr uint8_t F6 = 0x3B;
constexpr uint8_t F3 = 0x38; constexpr uint8_t F7 = 0x3C;
constexpr uint8_t F4 = 0x39; constexpr uint8_t F8 = 0x3D;

// --- AUTOMAÇÃO (FADERS) ---
constexpr uint8_t AUTO_READ = 0x4A;
constexpr uint8_t AUTO_WRITE = 0x4B;
constexpr uint8_t AUTO_TRIM = 0x4C;
constexpr uint8_t AUTO_TOUCH = 0x4D;
constexpr uint8_t AUTO_LATCH = 0x4E;
constexpr uint8_t AUTO_GROUP = 0x4F;

// --- UTILIDADES ---
constexpr uint8_t UTIL_SAVE = 0x50;
constexpr uint8_t UTIL_UNDO = 0x51;
constexpr uint8_t UTIL_CANCEL = 0x52;
constexpr uint8_t UTIL_ENTER = 0x53;

// --- MARCADORES E FLUXO ---
constexpr uint8_t MARKER = 0x54;
constexpr uint8_t NUDGE = 0x55;
constexpr uint8_t CYCLE = 0x56;     // Ligar/Desligar Loop
constexpr uint8_t DROP = 0x57;
constexpr uint8_t REPLACE = 0x58;
constexpr uint8_t CLICK = 0x59;     // Ligar Metrônomo
constexpr uint8_t SOLO_CLEAR = 0x5A;

// --- TRANSPORTE ---
constexpr uint8_t REWIND = 0x5B;
constexpr uint8_t FAST_FWD = 0x5C;
constexpr uint8_t STOP = 0x5D;
constexpr uint8_t PLAY = 0x5E;
constexpr uint8_t RECORD = 0x5F;

// --- JOG WHEEL E SCRUB ---
constexpr uint8_t SCRUB = 0x65;

// --- FADER TOUCH (O Segredo da Automação) ---
constexpr uint8_t FADER_TOUCH_1 = 0x68; constexpr uint8_t FADER_TOUCH_5 = 0x6C;
constexpr uint8_t FADER_TOUCH_2 = 0x69; constexpr uint8_t FADER_TOUCH_6 = 0x6D;
constexpr uint8_t FADER_TOUCH_3 = 0x6A; constexpr uint8_t FADER_TOUCH_7 = 0x6E;
constexpr uint8_t FADER_TOUCH_4 = 0x6B; constexpr uint8_t FADER_TOUCH_8 = 0x6F;
constexpr uint8_t FADER_TOUCH_MASTER = 0x70;