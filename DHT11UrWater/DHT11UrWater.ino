#include <ArduinoJson.h>
#include <dht.h>

dht DHT;
#define DHT11_PIN 4 // Pin connected to 4


void setup() {
  Serial.begin(9600);
}

void loop() {
  // read the input on pin 5:
  int chk = DHT.read11(DHT11_PIN);
  float temperature = DHT.temperature;
  float humidity = DHT.humidity;

  StaticJsonDocument<100> json;
  json["OdValue"] = DHT.temperature; // humidity = Dissolved Oxygen sensor (example)
  json["TmpValue"] = humidity; // temperatura = Temperature (since its a WQI's parameter) 

  String jsonString;
  serializeJson(json, Serial);
  Serial.println(jsonString);
  Serial.println(temperature);

  delay(3000);
}
