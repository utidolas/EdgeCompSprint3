#include <ArduinoJson.h>
#include <dht.h>

dht DHT;

const int dhtPin = A0; // Pin connected to A0

void setup() {
  Serial.begin(9600);
  pinMode(dhtPin, INPUT);
}

void loop() {
  // read the input on Analog pin 0:
  int dhtValue = DHT.read11(dhtPin);
  float temperature = DHT.temperature;
  float humidity = DHT.humidity;

  StaticJsonDocument<100> json;
  json["OdValue"] = temperature; // humidity = Dissolved Oxygen sensor (example)
  json["TmpValue"] = humidity; // temperatura = Temperature (since its a WQI's parameter) 

  String jsonString;
  serializeJson(json, Serial);
  Serial.println(jsonString);

  delay(3000);
}
