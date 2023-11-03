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
