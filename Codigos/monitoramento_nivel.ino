#include <WiFi.h>          // Inclui a biblioteca para gerenciar a conexão Wi-Fi no ESP32.
#include <PubSubClient.h>  // Inclui a biblioteca para comunicação via protocolo MQTT.

// --- Definições de Pinos do ESP32 ---
// Estes são os pinos GPIO (General Purpose Input/Output) do seu ESP32.
#define TRIG_PIN 4  // Pino digital (GPIO) 4: Usado para enviar o pulso do sensor ultrassônico.
#define ECHO_PIN 16 // Pino digital (GPIO) 16: Usado para receber o eco do sensor ultrassônico.
#define LED_PIN 17  // Pino digital (GPIO) 17: Usado para controlar o LED de alerta.

// --- Configurações do Sensor e Limites de Nível de Água ---
// Alcance máximo confiável do sensor HC-SR04 em centímetros.
// Leituras acima disso podem ser ruído ou fora da capacidade do sensor.
#define MAX_SENSOR_RANGE_CM 400 
// Alcance mínimo confiável do sensor HC-SR04 em centímetros.
// Leituras abaixo disso podem ser imprecisas ou indicar um erro.
#define MIN_SENSOR_RANGE_CM 2   
// Nível de água crítico em centímetros: se a distância do sensor à água for MENOR que este valor, o nível é considerado "perto do sensor".
// Para um sensor instalado no topo de um reservatório, uma distância < 10cm significa que o nível da água está BEM ALTO.
// Se a intenção é alertar para "nível baixo de água", a lógica teria que ser `distanciaCm > LIMITE_DE_NIVEL_BAIXO_CM`.
// Mantendo a lógica atual: LED acende quando a água está bem perto do sensor (nível alto).
#define CRITICAL_LEVEL_CM 10    

// --- Credenciais Wi-Fi (Para a simulação no Wokwi) ---
// Nome da rede Wi-Fi que o simulador Wokwi fornece. É uma rede aberta e gratuita para testes.
const char* ssid = "Wokwi-GUEST"; 
// Senha da rede Wi-Fi. Para "Wokwi-GUEST", a senha é vazia.
const char* password = "";        

// --- Configurações do Broker MQTT ---
// Endereço do broker MQTT público 'test.mosquitto.org'. Este servidor é amplamente usado para testes e não requer autenticação.
const char* mqtt_server = "test.mosquitto.org"; 
// Porta padrão para o protocolo MQTT não criptografado (conexão normal).
const int mqtt_port = 1883;                     
// Tópico MQTT onde os dados de nível de água serão publicados.
// Clientes externos podem assinar este tópico para receber os dados em tempo real.
const char* mqtt_topic_publish = "nivelagua/status"; 

// --- Objetos para Gerenciamento de Conexão ---
// Objeto que permite ao ESP32 atuar como um cliente Wi-Fi para se conectar à internet.
WiFiClient espClient;
// Objeto que gerencia a comunicação MQTT (publicar/assinar mensagens) usando o cliente Wi-Fi.
PubSubClient client(espClient);

// --- Variáveis para Medição de Tempo de Resposta (para o seu artigo) ---
// Armazena o tempo (em milissegundos desde o início) em que uma condição crítica foi detectada pelo sensor.
long sensorDetectionTime = 0; 
// Armazena o tempo (em milissegundos) em que o LED (atuador) foi fisicamente acionado em resposta à condição crítica.
long actuatorActionTime = 0;  
// Armazena o tempo (em milissegundos) em que o comando de publicação MQTT foi enviado pelo ESP32.
long mqttPublishSendTime = 0; 

// Variável global para rastrear o estado atual do LED (se deveria estar aceso)
bool isLedOn = false; 
// Variável global para armazenar o último tempo em que o LED foi aceso
unsigned long lastLedOnTime = 0; 
// Tempo mínimo que o LED deve ficar aceso (em milissegundos), por exemplo, 5 segundos.
// Isso evita que o LED pisque rapidamente se a leitura do sensor oscilar ligeiramente.
const unsigned long MIN_LED_ON_DURATION = 5000; // 5 segundos

// --- Função: Conectar ao Wi-Fi ---
// Tenta conectar o ESP32 à rede Wi-Fi definida.
void setup_wifi() {
  delay(10); // Pequena pausa inicial para estabilização.
  Serial.println(); // Quebra de linha para formatar a saída no Serial Monitor.
  Serial.print("Conectando-se a Wi-Fi: ");
  Serial.println(ssid); // Imprime o nome da rede que está tentando conectar.

  WiFi.begin(ssid, password); // Inicia o processo de conexão Wi-Fi com SSID e senha.

  int attempts = 0; // Contador de tentativas de conexão.
  // Loop que continua enquanto o Wi-Fi não estiver conectado.
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); // Espera 500ms entre as tentativas de conexão.
    Serial.print("."); // Imprime um ponto para indicar que está tentando.
    attempts++;
    if (attempts > 20) { // Se tentar por mais de 10 segundos (20 * 500ms), assume falha.
        Serial.println("\nFalha na conexao WiFi. Verifique sua simulacao ou hardware.");
        break; // Sai do loop para evitar travamento infinito.
    }
  }

  // Verifica o status final da conexão e imprime o resultado.
  if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWiFi conectado com sucesso!");
      Serial.print("Endereco IP do ESP32: ");
      Serial.println(WiFi.localIP()); // Mostra o endereço IP que o ESP32 recebeu na rede.
  } else {
      Serial.println("Erro: Nao foi possivel conectar ao WiFi.");
  }
}

// --- Função: Reconectar ao Broker MQTT ---
// Tenta reconectar ao broker MQTT se a conexão cair (ex: problema de rede, broker offline).
void reconnect_mqtt() {
  // Loop que continua enquanto o cliente MQTT não estiver conectado.
  while (!client.connected()) { 
    Serial.print("Tentando conexao MQTT...");
    // Cria um ID de cliente único para cada conexão. Isso é importante para o broker.
    String clientId = "ESP32Monitor-" + String(random(0xffff), HEX);

    // Tenta conectar ao broker MQTT.
    // 'c_str()' converte a String (Arduino) para um ponteiro de char (C++ padrão) que a função 'connect' espera.
    if (client.connect(clientId.c_str())) { 
      Serial.println("conectado ao MQTT!");
      // Publica uma mensagem inicial no tópico para confirmar que o dispositivo está online.
      client.publish(mqtt_topic_publish, "Sistema de monitoramento de nivel de agua conectado!");
    } else {
      Serial.print("Falha na conexao MQTT, rc="); // Imprime o código de retorno (reason code) da falha (útil para depuração).
      Serial.print(client.state()); 
      Serial.println(" -> tentando novamente em 5 segundos...");
      delay(5000); // Espera 5 segundos antes de tentar reconectar novamente.
    }
  }
}

// --- Setup: Inicialização do ESP32 (esta função é executada APENAS UMA VEZ ao ligar) ---
void setup() {
  Serial.begin(115200); // Inicia a comunicação serial para depuração e mensagens no Serial Monitor, com velocidade de 115200 bits por segundo.

  // Configura os pinos do sensor ultrassônico: TRIG como SAÍDA (para enviar pulso) e ECHO como ENTRADA (para ler o eco).
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Configura o pino do LED como SAÍDA.
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // Garante que o LED comece desligado para um estado inicial claro.

  // Inicializa o gerador de números aleatórios com base no tempo atual em microssegundos.
  // Isso ajuda a gerar IDs de cliente MQTT mais aleatórios e únicos a cada boot.
  randomSeed(micros()); 

  Serial.println("Iniciando configuracao Wi-Fi e MQTT...");
  setup_wifi(); // Chama a função para conectar ao Wi-Fi.
  client.setServer(mqtt_server, mqtt_port); // Configura o endereço e porta do broker MQTT para o cliente.
  Serial.println("Setup completo. Iniciando o loop de monitoramento...");
}

// --- Loop Principal: Esta função é executada REPETIDAMENTE e infinitamente após o 'setup()' ---
void loop() {
  // Garante que a conexão MQTT esteja sempre ativa. Se por algum motivo ela cair, tenta reconectar.
  if (!client.connected()) {
    reconnect_mqtt();
  }
  client.loop(); // Essencial: Esta linha processa mensagens MQTT pendentes e mantém a conexão ativa com o broker.

  // --- 1. Medição do Sensor Ultrassônico HC-SR04 ---
  Serial.println("\n--- Nova Leitura do Sensor ---");
  // Envia um pulso de 10 microssegundos no pino TRIG para iniciar a medição do sensor.
  // O sensor emite uma onda sonora.
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // 'pulseIn' mede a duração do pulso de retorno (eco) no pino ECHO.
  // O terceiro parâmetro (30000) é um timeout de 30 milissegundos. Se não houver eco nesse tempo, retorna 0.
  long duracao = pulseIn(ECHO_PIN, HIGH, 30000); 
  float distanciaCm; // Variável para armazenar a distância calculada em centímetros.

  Serial.print("Duracao do pulso (microssegundos): ");
  Serial.println(duracao);

  if (duracao == 0) { // Se a duração for 0, indica que o sensor não detectou um eco (timeout ou objeto fora de alcance).
      distanciaCm = -1; // Usa -1 para sinalizar uma leitura inválida (valor especial para erro).
      Serial.println("DEBUG: Sem eco detectado ou timeout. Leitura invalida.");
  } else {
      // Calcula a distância em centímetros. A velocidade do som no ar é aproximadamente 0.0343 cm/microsegundo.
      // Divide por 2 porque o som viaja até o objeto e volta.
      distanciaCm = duracao * 0.0343 / 2; 
      
      // Valida a distância dentro dos limites operacionais confiáveis do sensor.
      if (distanciaCm > MAX_SENSOR_RANGE_CM) { 
          distanciaCm = -1; // Leitura acima do alcance máximo confiável.
          Serial.print("DEBUG: Distancia ("); 
          Serial.print(distanciaCm); 
          Serial.println("cm) maior que o alcance maximo do sensor.");
      } else if (distanciaCm < MIN_SENSOR_RANGE_CM) {
          distanciaCm = -1; // Leitura abaixo do alcance mínimo confiável (pode ser imprecisa).
          Serial.print("DEBUG: Distancia ("); 
          Serial.print(distanciaCm); 
          Serial.println("cm) menor que o alcance minimo do sensor.");
      }
  }
  
  Serial.print("Distancia final (cm): ");
  Serial.println(distanciaCm);

  // --- 2. Lógica de Acionamento do Atuador (LED) e Definição do Status MQTT ---
  char payload[100]; // Buffer para armazenar a mensagem JSON que será enviada via MQTT.
  String status = "NORMAL"; // O status inicial padrão do nível de água é "NORMAL".

  // Verifica se a condição crítica atual é verdadeira: leitura válida E distância menor que o limite crítico.
  bool currentCriticalCondition = (distanciaCm != -1 && distanciaCm < CRITICAL_LEVEL_CM);

  if (currentCriticalCondition) { 
    // Se a condição crítica é verdadeira NESTE ciclo:
    if (!isLedOn) { // Se o LED ainda não estava aceso na nossa lógica, ligue-o agora.
      sensorDetectionTime = millis(); // Marca o tempo de detecção da condição crítica.
      digitalWrite(LED_PIN, HIGH); // **ACENDE O LED**.
      actuatorActionTime = millis(); // Marca o tempo em que o LED foi fisicamente acionado.
      lastLedOnTime = millis(); // Marca o tempo atual como o momento em que o LED foi ligado (para persistência).
      isLedOn = true; // Atualiza o estado da nossa variável de controle do LED.
    }
    status = "CRITICO"; // Define o status como CRITICO.
    Serial.println("DEBUG: CONDICAO CRITICA DETECTADA! LED ACIONADO.");
  } else {
    // Se a condição crítica NÃO é verdadeira NESTE ciclo:
    // Verifica se o LED deve ser mantido aceso por persistência ou se a leitura é inválida.
    // 'isLedOn' (já estava ligado) AND ('millis() - lastLedOnTime < MIN_LED_ON_DURATION' (tempo mínimo não passou) OR 'distanciaCm == -1' (leitura inválida)).
    if (isLedOn && (millis() - lastLedOnTime < MIN_LED_ON_DURATION || distanciaCm == -1)) {
      // Mantém o LED aceso. Isso é para evitar que o LED pisque se a leitura oscilar ou falhar brevemente.
      digitalWrite(LED_PIN, HIGH); // Garante que o LED permaneça aceso.
      if (distanciaCm == -1) {
          status = "SENSOR_FORA_FAIXA"; // Se a leitura ficou inválida mas o LED está persistindo, o status é SENSOR_FORA_FAIXA.
          Serial.println("DEBUG: Leitura do sensor fora da faixa. Mantendo LED ligado (aviso persistente).");
      } else {
          status = "CRITICO"; // Se a condição não é mais crítica, mas o tempo mínimo de LED ligado não passou, o status pode ser CRITICO (do aviso anterior).
          Serial.println("DEBUG: Condicao normal, mas LED ainda ligado (persistindo aviso por tempo minimo).");
      }
    } else {
      // Se nenhuma das condições acima for verdadeira, apaga o LED.
      digitalWrite(LED_PIN, LOW); 
      isLedOn = false; // Atualiza o estado da nossa variável de controle do LED.
      if (distanciaCm == -1) { 
          status = "SENSOR_FORA_FAIXA"; // Se a leitura é inválida e o LED não está persistindo.
          Serial.println("DEBUG: Leitura do sensor fora da faixa. LED DESLIGADO.");
      } else { 
          status = "NORMAL"; // Se a leitura é válida e a condição não é crítica.
          Serial.println("DEBUG: Condicao NORMAL. LED DESLIGADO.");
      }
    }
  }

  // --- 3. Preparação e Publicação da Mensagem MQTT ---
  mqttPublishSendTime = millis(); // Marca o tempo antes de enviar a mensagem MQTT.

  // Constrói a string JSON do payload com base no status e nível de água.
  if (distanciaCm == -1) {
      // Payload para leitura fora da faixa (nível 'null' porque não há um valor válido, e status SENSOR_FORA_FAIXA).
      sprintf(payload, "{\"nivel\": null, \"status\": \"%s\"}", status.c_str());
  } else if (strcmp(status.c_str(), "CRITICO") == 0) { // Se o status é "CRITICO" (usar strcmp para comparar strings C)
      // Payload para nível crítico, incluindo a distância e o tempo de resposta do atuador.
      sprintf(payload, "{\"nivel\": %.2f, \"status\": \"%s\", \"tempo_atuador_ms\": %ld}", 
              distanciaCm, status.c_str(), actuatorActionTime - sensorDetectionTime);
  } else { 
      // Payload para nível normal ou SENSOR_FORA_FAIXA, incluindo a distância e o status.
      sprintf(payload, "{\"nivel\": %.2f, \"status\": \"%s\"}", distanciaCm, status.c_str());
  }

  // Tenta publicar a mensagem no tópico MQTT. Retorna true se a publicação foi bem-sucedida.
  if (client.publish(mqtt_topic_publish, payload)) {
    Serial.print("Publicado no MQTT: ");
    Serial.println(payload);
  } else {
    Serial.println("Falha ao publicar mensagem MQTT.");
  }

  // --- 4. Medição de Tempos de Resposta para o Artigo ---
  // Calcula e imprime o tempo de resposta do sensor para o atuador (LED)
  // SOMENTE se a condição crítica for verdadeira NESTE CICLO.
  if (currentCriticalCondition) { 
      long tempoRespostaAtuador = actuatorActionTime - sensorDetectionTime;
      Serial.print("Tempo de resposta (Sensor -> Atuador LED): ");
      Serial.print(tempoRespostaAtuador);
      Serial.println(" ms");
  }

  // Calcula e imprime o tempo que a operação de publicação MQTT levou na placa.
  // NOTA IMPORTANTE: Isso mede o tempo que o ESP32 leva para enviar a mensagem.
  // Não inclui a latência da rede (internet) até o broker na internet, que pode variar.
  long tempoOperacaoMqtt = millis() - mqttPublishSendTime;
  Serial.print("Tempo da operacao MQTT (envio da placa): ");
  Serial.print(tempoOperacaoMqtt);
  Serial.println(" ms");

  delay(1000); // Pausa de 1 segundo (1000 milissegundos) antes de repetir o loop.
               // Isso controla a frequência de leitura e publicação.
               // Um delay muito curto pode sobrecarregar o broker ou o sensor.
}
