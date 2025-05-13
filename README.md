# 💧 Monitoramento Inteligente do Nível de Água via MQTT

Este projeto implementa um sistema de monitoramento de nível de água baseado em IoT, com foco na conservação e no uso eficiente da água em reservatórios residenciais e comerciais. Ele utiliza o sensor ultrassônico HC-SR04, Arduino UNO, LED de alerta e simulação do protocolo MQTT via mensagens.

---

## 🔧 Componentes utilizados

- Arduino Uno (plataforma de prototipagem)
- Sensor ultrassônico HC-SR04
- LED vermelho (atuador)
- Resistor de 220Ω
- Simulação via plataforma [Wokwi](https://wokwi.com/)
- Protocolo de comunicação: MQTT (`test.mosquitto.org`)

---

## 📊 Funcionalidade

- O sensor detecta o nível de água simulando uma distância crítica.
- Quando a distância for menor que 10 cm, o LED acende como alerta.
- O sistema simula a publicação dos dados no broker MQTT usando `Serial.print()`:


---

## 🔄 Fluxograma

![Fluxograma](Imagens/fluxo.png)

---

## 📈 Gráfico de Tempo de Resposta

![Gráfico](Imagens/grafico_tempo.png)

---

## 💡 Como testar

Você pode testar esse projeto diretamente no simulador Wokwi com o código disponível na pasta `Codigo/`. Basta colar no `sketch.ino` e ajustar a distância no sensor HC-SR04.

---

## 🔗 Vídeo demonstração

[![Assista ao vídeo](https://img.youtube.com/vi/SEU_ID_DO_VIDEO/0.jpg)](https://youtube.com/watch?v=SEU_ID_DO_VIDEO)

---

## 📁 Código-fonte

O código-fonte está disponível em `/Codigos/monitoramento_nivel.ino`.

---

## 📝 Licença

Projeto acadêmico desenvolvido para a disciplina "Objetos Inteligentes Conectados" – Universidade Presbiteriana Mackenzie.
