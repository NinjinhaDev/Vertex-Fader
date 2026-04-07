// --- Conexao.h ---
#pragma once  // Evita que o arquivo seja lido duas vezes pelo compilador
#include <WiFi.h>
#include <WiFiManager.h>
#include <ArduinoOTA.h>
#include <WebServer.h>

// Avisa as outras abas que o servidor existe, mas ele será criado no main
extern WebServer server;

void setupConexao() {
  WiFiManager wm;
  if (!wm.autoConnect("Vertex_Config")) {
    Serial.println("Falha na conexão");
    ESP.restart();
  }

  ArduinoOTA.setHostname("VERTEX_FADER");
  ArduinoOTA.begin();
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}