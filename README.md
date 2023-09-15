# Edge Computing Sprint3 
### Projeto que consiste em demonstrar a aplicação de um IoT de análise de água usando arduino em um CSP (Cloud Service Provider). 
O projeto trabalhará com um sensor LDR **simulado** no arduino através do SimulIDLE, visto que **não possuimos** o arduino físico para a realização do projeto. O sensor LDR, apesar de ser um sensor de luz, **simulara** um sensor de água **TDS**, que calcula o nivel de total de sólidos dissolvidos na água, medidos por PPM. O sensor LDR foi escolhido pois seu output de valores é semelhante com o PPM do TDS (que varia de 0 a 500+), fazendo assim, uma simulação de números parecidos com um sensor TDS. O projeto em si é um software que coletará dados de diversos sensores e fará uma análise para o cliente, com diversas outras funcionalidades. Por conta disso, o sensor TDS é apenas um dos sensores disponíveis no nosso software que será usado como exemplo.

## Requisitos e Dependências 
Para o funcionamento do projeto, é necessário: 

- Arduino IDE com bibliotecas json
- Arduino Físico OU Simulador (ex.: SimulIDLE)
- Um serial port virtual, (ex.: com0com) caso use o emulador de arduino
- Node-Red com bibliotecas de virtual ports
- Servidor MQTT (ex.: Hive MQTT Websocket Client)
- Algum Cloud Service Provider (ex.: TagoIO)

O projeto depende de serviço de internet para o bom funcionamento e comuniação IoT com servidores CSP, assim como o bom funcionamento dos sensores IoT, que são sensores no arduino UNO no exemplo. Um fluxo montado e configurado corretamente no Node-Red é essencial, visto que toda a ligação e conexão é feita através do mesmo. 

## Intruções
Primeiramente, é necessário montar o circuito arduino do sensor no SimulIDLE, que consiste em:
- 1 Arduino UNO
- 1 Sensor LDR
- 1 Resistor 100 Ω
- 3 Cabos (1 GND, 1 5V, 1 em algum pin analógico) 

Depois faça upload do seu código .ino para o programa, assim como carregar o firmware .hex do seu código. Após isso, abra o com0com, e inicie a porta virtual do SimulIDLE com algum dos pares de portas virtuais, compile o código e veja se está tudo funcionando.  
```
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
```
Em sequência, abra o node-red e coloque o "serial in" no flow do node-red com a porta virtual par a que está colocada no SimulIDLE. Conecte a porta serial em um json para a conversão de json para string, e passe em 2 funções (ou 1, se preferir), uma para o MQTT Websocket e outra para o TagoIO. A função para o TagoIO servirá para que possa ser printado de json para o CSP do Tago.
```
var currentDate = new Date();

var C = {
    payload: {
        "variable": "PPM",
        "unit": "valor",
        "value": msg.payload.ldrValue.toString()
    }
};

return C;
```
Para finalizar, ligue a função em dois nodes "mqtt in" e configure um para o Hive e outro para o TagoIO, com seu token, tópico e servidor. Dê deploy no node-red para salvar as mudanças, e ligue o arduino no SimulIDLE para ver os resultados. O Hive aparecerá o input no seu tópico de inscrição, e o TagoIO mostrará no device conectado em _Live Inspector_. É possível, também, criar um dashboard no TagoIO para que a visualização dos dados seja mais clara.  