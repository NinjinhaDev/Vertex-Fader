# 🎚️ Vertex Fader
**High-Precision Motorized Fader Controller for DAWs**

O **Vertex Fader** é um projeto de controladora MIDI construída com uma arquitetura de processamento duplo, projetada para oferecer a mesma precisão, suavidade e velocidade de consoles de áudio profissionais. 

---

## 🧠 Arquitetura de Processamento Duplo
Para garantir zero latência (*jitter*) e movimentos limpos, o projeto divide as tarefas complexas e elétricas em dois processadores separados que conversam entre si a incríveis **1 Megabaud**:

* **O Cérebro (ESP32):** Gerencia a inteligência. Conversa com a DAW via USB, processa o protocolo *Mackie MCU*, gerencia o estado dos botões/encoders e calcula a física de aceleração matemática dos motores.
* **O Músculo (RP2040):** O "Co-processador Robótico". Roda 100% focado na geração de sinais PWM de altíssima frequência (25kHz), garantindo que os motores tenham torque e operem de forma absolutamente silenciosa, sem apitos eletrônicos.

## ✨ Recursos Principais (v1.3)

* 🤖 **Controle Proporcional de Distância (PID):** Faders não dão "trancos". A velocidade é calculada em tempo real com base na distância do alvo. O motor arranca com força máxima e aplica um "freio ABS" milímetros antes de chegar ao destino.
* 🕺 **Fader Dance (Auto-Calibração):** A cada inicialização, o sistema bate nos extremos da placa para mapear os limites físicos reais dos faders, compensando automaticamente o desgaste natural das trilhas de carbono.
* 🎛️ **Integração Nativa Mackie MCU:** Reconhecido automaticamente pelo REAPER (e outras DAWs principais) de forma "Plug and Play".
* 👆 **Touch-to-Select (Smart Focus):** Tocar na parte metálica do fader corta a energia do motor instantaneamente e aciona a seleção automática do canal na DAW, garantindo que o seu equalizador/plugin esteja sempre focado na trilha correta sem precisar de cliques extras.
* 🔄 **Modos Avançados (FLIP e EQ):** Suporte nativo aos Assignment Buttons da Mackie para inverter funções de faders com encoders e mapear plugins dinamicamente.

---
*Desenvolvido com foco em agilidade de estúdio e mixagem ao vivo.*