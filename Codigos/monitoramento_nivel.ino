long tempoInicio;
long tempoFim;
long tempoResposta;

void setup() {
  Serial.begin(9600);

  pinMode(9, OUTPUT);   // TRIG
  pinMode(10, INPUT);   // ECHO
  pinMode(13, OUTPUT);  // LED

  Serial.println("Conectado ao Wi-Fi...");
  Serial.println("Conectado ao broker MQTT: test.mosquitto.org");
}

void loop() {
  // Envia pulso ao sensor
  digitalWrite(9, LOW);
  delayMicroseconds(2);
  digitalWrite(9, HIGH);
  delayMicroseconds(10);
  digitalWrite(9, LOW);

  long duracao = pulseIn(10, HIGH);
  float distancia = duracao * 0.034 / 2;

  // Simulação de tempo de resposta e envio MQTT
  if (distancia < 10) {
    tempoInicio = millis();

    digitalWrite(13, HIGH); // Acende o LED

    tempoFim = millis();
    tempoResposta = tempoFim - tempoInicio;

    Serial.print("Distância crítica detectada: ");
    Serial.print(distancia);
    Serial.print(" cm | Tempo de resposta: ");
    Serial.print(tempoResposta);
    Serial.println(" ms");

    // Simula envio via MQTT
    Serial.print("Publicando no tópico MQTT 'nivelagua/status': ");
    Serial.print("{ \"nivel\": ");
    Serial.print(distancia);
    Serial.println(" }");

    delay(2000);
  } else {
    digitalWrite(13, LOW);

    // Simula envio periódico de status
    Serial.print("Publicando no tópico MQTT 'nivelagua/status': ");
    Serial.print("{ \"nivel\": ");
    Serial.print(distancia);
    Serial.println(" }");
  }

  delay(1000);
}
