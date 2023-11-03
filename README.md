# Edge Computing Sprint4 
### Projeto que consiste em demonstrar a aplicação de um IoT de análise de água utilizando arduino ou ESP32 e protocolo MQTT, com PoC (proof of concept). 
O projeto trabalhará com um sensor **DHT11** ou DHT22 no ESP32 (sera usado como exemplo no projeto pela conexão com wifi, porém funcionaria localmente com o arduino) através da plataforma física arduino uno ou ESP32. O sensor DHT11, apesar de ser um sensor de temperatura e umidade, **simulara** um sensor de água para parâmetros do IQA (índice de qualidade da água) de **Oxigênio dissolvido** e **Variação da temperatura**, medidos por % e graus, respectivamente. O sensor DHT11 foi escolhido pois seu output de valores é semelhante com os parâmetros escolhidos, fazendo assim, uma simulação de números parecidos com um sensor de análise de água. O projeto em si é um software que coletará dados de diversos sensores e fará uma análise para o cliente, com diversas outras funcionalidades. Por conta disso, o sensor de OD (Oxigênio dissolvido) e temperatura são apenas alguns dos sensores disponíveis no nosso software que será usado como exemplo.

## Requisitos e Dependências 
Para o funcionamento do projeto, é necessário: 

- Arduino IDE com bibliotecas json e dht11
- Placa Arduino físico
- Node-Red 
- Servidor MQTT (ex.: Hive MQTT Websocket Client)
- Algum Cloud Service Provider (ex.: TagoIO)

O projeto depende de serviço de internet para o bom funcionamento e comuniação IoT com servidores CSP, assim como o bom funcionamento dos sensores IoT, que são sensores no arduino UNO no exemplo. Um fluxo montado e configurado corretamente no Node-Red é essencial, visto que toda a ligação e conexão é feita através do mesmo. 

## Intruções
Primeiramente, é necessário montar o circuito arduino do sensor na sua placa, que consiste em:
- 1 Arduino UNO
- 1 Sensor DHT11
- 2 Resistores 220 Ω
- 2 Leds verdes
- 5 Cabos (1 GND, 1 5V, 1 pin analógico para o DHT, 2 pins digitais para LED) 

Depois conecte o arduino em algum PORT da máquina e importe seu código .ino para o programa, selecionando a devida PORT que o arduino está conectado. Após isso, compile e faça o upload do código e veja se está tudo funcionando. Note que o código abaixo é um código feito para ESP32 pelo fato de necessitarmos da conexão wifi para a comunicação com os sensores e protocolos, mas como já mencionando anteriormente, um arduino UNO trabalhado localmente funcionaria também.
```
#include <WiFi.h>
#include <DHTesp.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

// Configurações de MQTT
const char *BROKER_MQTT = "broker.hivemq.com";
const int BROKER_PORT = 1883;
const char *ID_MQTT = "esp32_mqtt";
const char *TOPIC_PUBLISH_OXIG = "fiap/Serelepes/oxigenio";
const char *TOPIC_PUBLISH_TEMP = "fiap/Serelepes/temp";
const char *TOPIC_PUBLISH_LED = "fiap/Serelepes/led";

// WiFi
const char *SSID = "Wokwi-GUEST"; // SSID / nome da rede WI-FI que deseja se conectar
const char *PASSWORD = "";        // Senha da rede WI-FI que deseja se conectar


// Variáveis globais
DHTesp dht;
TempAndHumidity sensorValues;
WiFiClient espClient;
PubSubClient MQTT(espClient);
unsigned long publishUpdate = 0;
int temperature;
int oxygen;
const int TAMANHO = 100;

// Protótipos de funções
void updateSensorValues();
void initMQTT();

// Configurações de Hardware
#define PIN_DHT 12
#define PIN_GREENLED 4
#define PIN_GREENLED2 5
#define PUBLISH_DELAY 2000

//INIT WIFI
void initWiFi() {
  Serial.print("Conectando com a rede: ");
  Serial.println(SSID);
  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Conectado com sucesso: ");
  Serial.println(SSID);
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

}

//RECONNECT WIFI
void reconnectWiFi(void) {
  if (WiFi.status() == WL_CONNECTED)
    return;

  WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Wifi conectado com sucesso");
  Serial.print(SSID);
  Serial.println("IP: ");
  Serial.println(WiFi.localIP());
}

// CHECK WIFI AND MQTT 
void checkWiFIAndMQTT() {
  if (WiFi.status() != WL_CONNECTED) reconnectWiFi();
  if (!MQTT.connected()) reconnectMQTT();
}

//INIT MQTT
void initMQTT() {
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);
  MQTT.setCallback(callbackMQTT);
}

//RECONNECT MQTT
void reconnectMQTT() {
  while (!MQTT.connected()) {
    Serial.print("Tentando conectar com o Broker MQTT: ");
    Serial.println(BROKER_MQTT);

    if (MQTT.connect(ID_MQTT)) {
      Serial.println("Conectado ao broker MQTT!");
      MQTT.subscribe(TOPIC_PUBLISH_OXIG);
      MQTT.subscribe(TOPIC_PUBLISH_TEMP);
      MQTT.subscribe(TOPIC_PUBLISH_LED);
    } else {
      Serial.println("Falha na conexão com MQTT. Tentando novamente em 2 segundos.");
      delay(2000);
    }
  }
}


//CALLBACK
void callbackMQTT(char *topic, byte *payload, unsigned int length) {
  String msg = String((char*)payload).substring(0, length);
  
  Serial.printf("Mensagem recebida via MQTT: %s do tópico: %s\n", msg.c_str(), topic);

  if (strcmp(topic, TOPIC_PUBLISH_LED) == 0) {
    int valor = atoi(msg.c_str());

    if (valor == 1) {
      digitalWrite(PIN_GREENLED, HIGH);
      Serial.println("LED ligado via comando MQTT");
    } else if (valor == 0) {
      digitalWrite(PIN_GREENLED, LOW);
      Serial.println("LED desligado via comando MQTT");
    }
  }
}


void updateSensorValues() {
  sensorValues = dht.getTempAndHumidity();
}

void setup() {
  Serial.begin(115200);

  //LED
  pinMode(PIN_GREENLED, OUTPUT);
  digitalWrite(PIN_GREENLED, LOW);
  pinMode(PIN_GREENLED2, OUTPUT);
  digitalWrite(PIN_GREENLED2, LOW);
  //DHT
  dht.setup(PIN_DHT, DHTesp::DHT22);

  initWiFi();
  initMQTT();

  Serial.println("Iniciando checagem dos níveis de Oxigênio Dissolvido e Temperatura");
}

void loop() {

  updateSensorValues();
  checkWiFIAndMQTT();
  MQTT.loop();

  //Get temp and humi (Dissolved Oxig for simulation)
  temperature = sensorValues.temperature;
  oxygen = sensorValues.humidity;

  if (oxygen < 80 || oxygen > 120){
    digitalWrite(PIN_GREENLED, LOW);
  }else{
    digitalWrite(PIN_GREENLED, HIGH);
  }
  
  if(temperature < 10 || temperature > 25){
    digitalWrite(PIN_GREENLED2, LOW);
  }else{
    digitalWrite(PIN_GREENLED2, HIGH);
  }

  //PRINTANDO NO MQTT
  if ((millis() - publishUpdate) >= PUBLISH_DELAY) {
  publishUpdate = millis();
  updateSensorValues();

  if (!isnan(sensorValues.temperature) && !isnan(sensorValues.humidity)) {
    StaticJsonDocument<TAMANHO> doc;
    doc["temperatura"] = temperature;

    char buffer[TAMANHO];
    serializeJson(doc, buffer);
    MQTT.publish(TOPIC_PUBLISH_TEMP, buffer);
    Serial.println(buffer);
  }

  if (!isnan(sensorValues.temperature) && !isnan(sensorValues.humidity)) {
    StaticJsonDocument<TAMANHO> doc2;
    doc2["oxigenioDissolvido"] = oxygen;

    char buffer[TAMANHO];
    serializeJson(doc2, buffer);
    MQTT.publish(TOPIC_PUBLISH_OXIG, buffer);
    Serial.println(buffer);
  }

}
  delay(5000); // this speeds up the simulation
}
```
Em sequência, abra o node-red e adapte como necessário. No exemplo do projeto, utilizando o ESP32, como ele fará essa conexão com o broker MQTT, começaremos com um node "mqtt out" para pegar todas as informações e então um node "switch" para filtra-las com o payload. Para sabermos que informação pegar, basta olharmos no bloco do código onde transformamos as informações em json. 
```
  if (!isnan(sensorValues.temperature) && !isnan(sensorValues.humidity)) {
    StaticJsonDocument<TAMANHO> doc;
    doc["temperatura"] = temperature;

    char buffer[TAMANHO];
    serializeJson(doc, buffer);
    MQTT.publish(TOPIC_PUBLISH_TEMP, buffer);
    Serial.println(buffer);
  }

  if (!isnan(sensorValues.temperature) && !isnan(sensorValues.humidity)) {
    StaticJsonDocument<TAMANHO> doc2;
    doc2["oxigenioDissolvido"] = oxygen;

    char buffer[TAMANHO];
    serializeJson(doc2, buffer);
    MQTT.publish(TOPIC_PUBLISH_OXIG, buffer);
    Serial.println(buffer);
  }
```
Para finalizar, ligue o switch em um node "mqtt out" ou algum node de dashboard e configure para o seu CSP (ex.:TagoIO) ou um dashboard local, como no exemplo. Utilize o seu token, tópico e servidor. Dê deploy no node-red para salvar as mudanças, e compile o arduino ou ESP32 com ele conectado na sua máquina para ver os resultados. O Hive aparecerá o input no seu tópico de inscrição, e o dashboard mostrará os valores como você configurar. É possível, também, ver os dispositivos conectados no TagoIO em _Live Inspector_ caso queira algo mais detalhado.  