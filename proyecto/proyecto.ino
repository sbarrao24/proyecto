#include <WiFi.h>
#include "time.h"
#include <stdint.h>
#include <PubSubClient.h>

//sensor
int32_t sensorPin = 32;
#define led 18
#define led1 19

int32_t button = 0; 

//WIFI
//const char* ssid = "iPhone";
//const char* password = "biterkas";
const char* ssid = "DIGIFIBRA-PLUS-TZfC";
const char* password = "3yHcu7GKQ2H3";
//const char* ssid = "ALUMNOS_CICLOS";
//const char* password = "Ciclos2025sz?";

//servidor para fecha y hora
const char* ntpServer = "es.pool.ntp.org";
 
//zona horaria 
const int32_t gmtOffset_sec = 3600;

//horario de verano
const int32_t daylightOffset_sec = 3600;

//Estructura para trabajar con ntp
struct tm timeinfo;

//MQTT
const char* mqtt_username = "leire1";
const char* mqtt_password = "1357";
const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient mqtt_client(espClient);
void callback(char* topic, byte* payload, unsigned int lenght){
  Serial.print("Mensaje recibido en el tópico:");
  Serial.println(topic);

  String message;
  for(unsigned int i = 0; i < lenght; i++){
    message += (char)payload[i];
  }

  Serial.print("Mensaje: ");
  Serial.println(message);

  if(message == "1"){
    Serial.println("ON");
    button = 1;
  }

  else if(message == "0"){
    Serial.println("OFF");
    button = 1;
  }
}

void reconnect(){

while(!mqtt_client.connected()){
  Serial.print("Conectando a MQTT...");
  String client_id ="esp32-client-" + String(WiFi.macAddress());
  //if(mqtt_client.connect(client_id.c_str(), mqtt_username, mqtt_password)){
    if (mqtt_client.connect(client_id.c_str())) {
    Serial.println("Conectado");
    mqtt_client.subscribe("sensor/boton");
  }
  else{
    Serial.print("Error, código: ");
    Serial.println(mqtt_client.state());
    delay(1000);
  }
}
}

void setup() {
  Serial.begin(115200);
  // sensor y leds
    pinMode(sensorPin, INPUT);
  delay(1000);
  pinMode(led, OUTPUT);
  pinMode(led1, OUTPUT);
  digitalWrite(led, HIGH);
  digitalWrite(led1, LOW);
 
  // Conexión wifi
  Serial.println("Conectando a Wifi...");
  WiFi.begin(ssid, password);

//comprobar que el tiempo de conexion es menor que 15s para ver si conecta el wifi
uint32_t conectingTime = millis();

while (WiFi.status() != WL_CONNECTED && millis() - conectingTime < 15000) {
    Serial.print(".");
    delay(1000);
}

if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nERROR: No se pudo conectar al WiFi.");
} else {
    Serial.println("\nConexión WiFi establecida.");
    Serial.print("Dirección IP: ");
    Serial.println(WiFi.localIP());
}
 

  // Fecha y hora
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Configurar MQTT
  mqtt_client.setServer(mqtt_server, mqtt_port);
  mqtt_client.setCallback(callback);

  
}

void loop() {
  mqtt_client.loop();
  int32_t value = 0;
  value = digitalRead(sensorPin);

  if (!mqtt_client.connected()) {
    reconnect();
  }
  if(button == 1){
   if(value == LOW){
     if (getLocalTime(&timeinfo)) {
       Serial.println(&timeinfo, "%d/%m/%Y %H:%M:%S");
       String currentTime = 
        String(timeinfo.tm_mday) + "/" + String(timeinfo.tm_mon + 1) + "/" + String(timeinfo.tm_year + 1900) + " " + String(timeinfo.tm_hour) 
        + ":" +  String(timeinfo.tm_min) + ":" + String(timeinfo.tm_sec);
      //  mqtt_client.publish("sensor/hora", currentTime.c_str());
       Serial.println("Obstaculo detectado");
        //display.println("Obstaculo detectado");
        digitalWrite(led, LOW);
        digitalWrite(led1, HIGH);
        mqtt_client.loop(); //procesa mensajes MQTT
       if(mqtt_client.connected()){
           mqtt_client.publish("sensor/hora", currentTime.c_str());
       }
   
      }
    }else{
     Serial.println("Obstaculo NO detectado");
     digitalWrite(led1, LOW);
     digitalWrite(led, HIGH);
    } 
  } else{
    Serial.println("Esperando orden de detectar");
    delay(1000);
  }

  delay(500);
}
