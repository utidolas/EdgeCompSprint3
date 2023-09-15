#include <ArduinoJson.h>

const int ldrPin = A0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(ldrPin, INPUT);
}

void loop() {
  // read the input on Analog pin 0:
  int ldrValue = analogRead(ldrPin);

  StaticJsonDocument<100> json;
  json["ldrValue"] = ldrValue;


  String jsonString;
  serializeJson(json, Serial);
  Serial.println(jsonString);

  delay(5000);
}
