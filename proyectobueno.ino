#include <WiFi.h>
#include "time.h"
#include <PubSubClient.h>
#include <stdint.h>

// Pines
int8_t sensorPin = 32;
int8_t ledRojo = 18;
int8_t ledVerde = 19;

// WiFi
const char* ssid = "DIGIFIBRA-TZfC";
const char* password = "3yHcu7GKQ2H3";
//const char* ssid = "ALUMNOS_CICLOS";
//const char* password = "Ciclos2025sz?";

// NTP (hora)
const char* ntpServer = "es.pool.ntp.org";
const int32_t gmtOffset_sec = 3600;
const int32_t daylightOffset_sec = 3600;
struct tm timeinfo;

// MQTT
const char* mqtt_username = "leire1";
const char* mqtt_password = "1357";
const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;
WiFiClient espClient;
PubSubClient mqtt_client(espClient);

// Botón MQTT
int32_t button = 0;

// Función cuando llega mensaje MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Mensaje en tópico ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(message);

  if (String(topic) == "sensor/boton") {
    if (message == "1") {
      digitalWrite(LED_BUILTIN, HIGH); // Encender LED_BUILTIN
      button = 1;
    } else {
      digitalWrite(LED_BUILTIN, LOW); // Apagar LED_BUILTIN
      button = 0;
    }
  }
}

// Conexión al broker MQTT
void reconnect() {
  while (!mqtt_client.connected()) {
    Serial.print("Conectando a MQTT...");
    String clientId = "esp32-" + String(WiFi.macAddress());
    if (mqtt_client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Conectado");
      mqtt_client.subscribe("sensor/boton");
    } else {
      Serial.print("Fallo (estado: ");
      Serial.print(mqtt_client.state());
      Serial.println("). Reintentando...");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Pines
  pinMode(sensorPin, INPUT);
  pinMode(ledRojo, OUTPUT);
  pinMode(ledVerde, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(ledRojo, HIGH);
  digitalWrite(ledVerde, LOW);
  digitalWrite(LED_BUILTIN, LOW);

  // Conectar a WiFi
  WiFi.begin(ssid, password);
  Serial.println("Conectando a WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado. Dirección IP:");
  Serial.println(WiFi.localIP());

  // Configurar hora NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // MQTT
  mqtt_client.setServer(mqtt_server, mqtt_port);
  mqtt_client.setCallback(callback);
}

void loop() {
  if (!mqtt_client.connected()) {
    reconnect();
  }
  mqtt_client.loop();

  int32_t valor = digitalRead(sensorPin);

  if (button == 1) {
    if (valor == HIGH) {
      Serial.println("Obstáculo detectado");
      digitalWrite(ledRojo, LOW);
      digitalWrite(ledVerde, HIGH);

      if (getLocalTime(&timeinfo)) {
        char buffer[64];
        strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", &timeinfo);
        mqtt_client.publish("sensor/hora", buffer);
        Serial.print("Hora enviada: ");
        Serial.println(buffer);
      } else {
        Serial.println("No se pudo obtener la hora.");
      }
    } else {
      Serial.println("No detecta nada");
      digitalWrite(ledRojo, HIGH);
      digitalWrite(ledVerde, LOW);
    }
  } else {
    Serial.println("Esperando botón MQTT...");
    digitalWrite(ledRojo, HIGH);
    digitalWrite(ledVerde, LOW);
  }

  delay(1000);
}