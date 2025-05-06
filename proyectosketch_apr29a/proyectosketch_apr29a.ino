#include <stdint.h>

#include <WiFi.h>
#include <PubSubClient.h>
#include "time.h"

//WiFi
//const char* ssid = "iPhone";
//const char* password = "biterkas";
const char* ssid = "ALUMNOS_CICLOS";
const char* password = "Ciclos2025sz?";

//hora española
const char* ntpServer = "es.pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 0;

//datos de mi aplicación (usuario, clave, servidor y puerto)
const char* mqtt_username = "sbarrao";
const char* mqtt_password = "1234";
const char* mqtt_server = "broker.hivemq.com";
const int32_t mqtt_port = 1883;

//creacción de objetos
WiFiClient espClient;
PubSubClient mqtt_client(espClient);


int32_t sensorPin = 32; //pin del sensor infrarrojos
int32_t ledRojo = 18;  //pin del LED rojo
int32_t ledVerde = 19; //pin del LED verde

void setup() {

  //Conexión WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }
  Serial.println("WiFi conectado");

//Configuración de hora
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

//Configuración MQTT
  mqtt_client.setServer(mqtt_server, mqtt_port);

  Serial.begin(115200); //comunicación en baudios
  pinMode(sensorPin, INPUT); 
  pinMode(ledRojo, OUTPUT); 
  pinMode(ledVerde, OUTPUT);       
}

void loop() {
  int32_t value = 0;
  value = digitalRead(sensorPin); //lectura del sensor

  if (value == LOW) {
    Serial.print("Objeto detectado con fecha y hora a las:");
    digitalWrite(ledVerde, HIGH); //Se enciende LED verde si se detecta algo
    digitalWrite(ledRojo, LOW); //LED rojo apagado
    //Obtener y enviar hora actual
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char hora[30];//se declara array ``hora´´ con 30 caracteres
    strftime(hora, sizeof(hora), "%H:%M:%S - %d/%m/%Y", &timeinfo);//sizeof(hora) devuelve 30 espacios disponibles para guardar caracteres
    
    Serial.println(hora);
  //Publicar hora al broker MQTT
    mqtt_client.publish("esp32/hora", hora); //enviar la hora
  } else {
    Serial.println("Error al obtener la hora");
  }
  } else {
    Serial.println("No detecta");
    digitalWrite(ledVerde, LOW);//LED verde apagado
    digitalWrite(ledRojo, HIGH);//Se nciende LED rojo en posición de reposo sin detectar
  }

  delay(1000); //1 segundo
}