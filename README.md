# 💧 Monitoramento Inteligente do Nível de Água via MQTT

Este projeto implementa um sistema de monitoramento de nível de água baseado em IoT, com foco na conservação e uso eficiente da água em reservatórios residenciais e comerciais. Ele utiliza o sensor ultrassônico HC-SR04, um microcontrolador ESP32, e um LED de alerta, comunicando-se em tempo real com o protocolo MQTT.

---

## 🔧 Componentes utilizados

- **Microcontrolador ESP32 (ex: ESP32-S3 ou ESP32 DevKit C):** Plataforma de prototipagem com Wi-Fi integrado, ideal para aplicações IoT.
- Sensor ultrassônico HC-SR04: Para medição do nível de água.
- LED vermelho: Atuador visual de alerta.
- Resistor de 220Ω: Para proteção do LED.
- Simulação via plataforma [Wokwi](https://wokwi.com/): Ambiente de simulação online que permite testar o circuito e a conectividade em tempo real.
- Protocolo de comunicação: MQTT (`test.mosquitto.org`): Utilizado para envio de dados em tempo real para a nuvem.

---

## 📊 Funcionalidade

Este sistema monitora continuamente o nível de água em reservatórios:

- O sensor HC-SR04 detecta a distância até a superfície da água.
- O ESP32 processa essa distância e a compara com limites pré-definidos.
- **Se a distância for inferior a 10 cm (indicando um nível de água alto/crítico), o LED vermelho é acionado e permanece aceso enquanto essa condição persistir, servindo como um alerta visual.**
- Os dados do nível de água (distância em cm e status) são publicados em tempo real no broker MQTT (`test.mosquitto.org`), permitindo monitoramento remoto.

**Os status enviados via MQTT são:**
- `CRITICO`: Nível de água alto (distância menor que 10 cm).
- `NORMAL`: Nível de água dentro da faixa aceitável.
- `LEITURA_FALHA`: O sensor não conseguiu obter uma leitura válida (por exemplo, objeto fora de alcance, falha na leitura).

---

## 🔄 Fluxograma

O fluxograma a seguir detalha a lógica de funcionamento do sistema, desde a inicialização até a publicação contínua dos dados e o acionamento do alerta.

![Fluxograma](Imagens/fluxo.jpeg)

---

## 📈 Gráfico de Tempo de Resposta

Este gráfico visualiza os tempos de resposta medidos, comparando a rapidez do acionamento do LED e da operação de envio MQTT.

![Gráfico](Imagens/grafico_tempo.jpeg)

---

## 💡 Como testar

Você pode testar este projeto diretamente no simulador Wokwi, que permite a simulação da conectividade Wi-Fi e MQTT em tempo real.

1.  Acesse o projeto no Wokwi: [https://wokwi.com/projects/431862831259841537]
2.  Inicie a simulação (botão "Play" verde).
3.  Abra o "Serial Monitor" no Wokwi para ver os logs de conexão e os dados publicados.
4.  Clique no sensor HC-SR04 no diagrama e ajuste a distância simulada do "alvo" para observar o acionamento do LED e a mudança de status no Serial Monitor e no MQTT Explorer.
5.  Utilize um cliente MQTT (como o [MQTT Explorer](https://mqtt-explorer.com/)) conectado a `test.mosquitto.org` (porta 1883) e assine o tópico `nivelagua/status` para visualizar os dados em tempo real.

---

## 🔗 Vídeo de Demonstração

Assista a uma demonstração completa do projeto, incluindo o funcionamento do hardware no Wokwi, o código e a comunicação MQTT em tempo real.

[![Assista ao vídeo](https://img.youtube.com/vi/SEU_ID_DO_VIDEO/0.jpg)](https://youtube.com/watch?v=SEU_ID_DO_VIDEO)

---

## 📁 Código-fonte

O código-fonte está disponível em [`/Codigos/monitoramento_nivel.ino`](Codigos/monitoramento_nivel.ino).

---

## 📝 Licença

Projeto acadêmico desenvolvido para a disciplina **Objetos Inteligentes Conectados** – Universidade Presbiteriana Mackenzie.
