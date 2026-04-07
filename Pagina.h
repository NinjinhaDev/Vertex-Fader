// --- Pagina.h ---
#pragma once
#include <WebServer.h>

extern WebServer server;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Vertex Fader Controller</title>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <style>
        body { font-family: sans-serif; text-align: center; background: #1a1a1a; color: white; margin: 0; padding: 20px; }
        .container { background: #333; padding: 20px; border-radius: 20px; max-width: 400px; margin: auto; box-shadow: 0 10px 30px rgba(0,0,0,0.5); }
        
        .nav { display: flex; justify-content: space-around; margin-bottom: 20px; background: #444; border-radius: 10px; overflow: hidden; }
        .nav button { background: none; border: none; color: white; padding: 15px; width: 50%; cursor: pointer; font-weight: bold; }
        .nav button.active { background: #007bff; }
        .tab-content { display: none; }
        .tab-content.active { display: block; }
        
        /* Estilos pro painel de Status */
        .status-box { background: #222; padding: 15px; border-radius: 10px; margin-bottom: 15px; text-align: left; }
        .status-box span { float: right; font-weight: bold; color: #28a745; }
    </style>
</head>
<body>
    <div class="container">
        <h2>VERTEX FADER</h2>
        <div class="nav">
            <button id="btnStatus" class="active" onclick="openTab('status')">STATUS</button>
            <button id="btnWifi" onclick="openTab('wifi')">WI-FI</button>
        </div>

        <div id="status" class="tab-content active">
            <div class="status-box">
                Conexão USB MIDI: <span id="usbStatus">ATIVO</span>
            </div>
            <div class="status-box">
                Faders Conectados: <span>8 Canais</span>
            </div>
            <div class="status-box">
                Firmware OTA: <span style="color: #007bff;">PRONTO</span>
            </div>
            
            <button style="background: #555; color: white; padding: 15px; border: none; border-radius: 10px; width: 100%; margin-top: 10px; cursor: not-allowed;">
                CALIBRAR MOTORES (Em breve)
            </button>
        </div>

        <div id="wifi" class="tab-content">
            <h3>Configurações de Rede</h3>
            <p style="font-size: 14px; color: #aaa;">Isso apagará a rede salva e reiniciará o controlador em modo Access Point.</p>
            <button style="background: #dc3545; color: white; padding: 15px; border: none; border-radius: 10px; width: 100%; cursor: pointer; font-weight: bold;" onclick="if(confirm('Tem certeza que deseja resetar as configurações de Wi-Fi?')) location.href='/reset_wifi'">
                RESETAR WI-FI
            </button>
        </div>
    </div>

    <script>
        function openTab(tabName) {
            document.querySelectorAll('.tab-content').forEach(t => t.classList.remove('active'));
            document.querySelectorAll('.nav button').forEach(b => b.classList.remove('active'));
            document.getElementById(tabName).classList.add('active');
            event.currentTarget.classList.add('active');
        }
    </script>
</body>
</html>
)rawliteral";

// Rotas da Web
void handleRoot() {
  server.send(200, "text/html", index_html);
}

void handleResetWifi() {
  server.send(200, "text/html", "<h1 style='color:white; font-family:sans-serif; text-align:center;'>Limpando WiFi e Reiniciando...</h1>");
  delay(1000);
  WiFiManager wm;
  wm.resetSettings();
  ESP.restart();
}

void setupRotasWeb() {
  server.on("/", handleRoot);
  server.on("/reset_wifi", handleResetWifi);
  server.begin();
}