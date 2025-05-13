#include <WiFi.h>
#include "time.h"
#include <PubSubClient.h>
#include <stdint.h> 
//librerías necesarias para conectarse por wifi, para trabajar con fecha y hora, y comunicación mqtt

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
const char* ntpServer = "es.pool.ntp.org";//servidor NTP ubicado en España
const int32_t gmtOffset_sec = 3600;// horario estándar
const int32_t daylightOffset_sec = 3600;//horario de verano
struct tm timeinfo; //almacenará la fecha y hora actual

// MQTT
const char* mqtt_username = "leire1";
const char* mqtt_password = "1357";
const char* mqtt_server = "broker.emqx.io";//dirección de un broker MQTT
const int mqtt_port = 1883;
WiFiClient espClient; //maneja la conexión
PubSubClient mqtt_client(espClient);

// Botón MQTT
int32_t button = 0;

// Función cuando llega mensaje MQTT
void callback(char* topic, byte* payload, unsigned int length) {//nombre del tópico, contenido del mensaje, nº de bytes
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }//convierte cada byte del mensaje (payload[i]) a carácter (char) y lo va añadiendo a la cadena message
  Serial.print("Mensaje en tópico ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(message);

//Si el valor es 1 activa LED_BUILTIN y pone button = 1 si no lo apaga (button = 0)
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

// Conexión al broker MQTT para interacción con el botón de la app
void reconnect() {
  while (!mqtt_client.connected()) { //bucle por si no se conecta a la primera
    Serial.print("Conectando a MQTT...");
    String clientId = "esp32-" + String(WiFi.macAddress());//Crea un ID único para evitar conflictos con otros dispositivos
    if (mqtt_client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {//ID y credenciales si son necesarias
      Serial.println("Conectado");
      mqtt_client.subscribe("sensor/boton");//para recibir los mensajes 
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
  pinMode(sensorPin, INPUT); //sensor infrarrojo entrada
  pinMode(ledRojo, OUTPUT); // led rojo salida
  pinMode(ledVerde, OUTPUT); //led verde salida
  pinMode(LED_BUILTIN, OUTPUT); //led integrado del ESP32 como salida se activa con el botón
  digitalWrite(ledRojo, HIGH);//led rojo encendido por espera o error
  digitalWrite(ledVerde, LOW);//led verde apagado se activa con el botón y cuando no haya corte de señal
  digitalWrite(LED_BUILTIN, LOW);//led integrado del ESP32 apagado se activa con el botón de la app

  // Conectar a WiFi
  WiFi.begin(ssid, password);
  Serial.println("Conectando a WiFi...");
  while (WiFi.status() != WL_CONNECTED) {//evalua hasta que hay conexión
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado. Dirección IP:");
  Serial.println(WiFi.localIP());

  // Configurar hora NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // configuración MQTT y llamada a la función
  mqtt_client.setServer(mqtt_server, mqtt_port);
  mqtt_client.setCallback(callback);
}

void loop() {
  if (!mqtt_client.connected()) {
    reconnect();
  }
  mqtt_client.loop(); 
//Verificación de la conexión
  int32_t valor = digitalRead(sensorPin);

//Si el botón MQTT está activado y Si detecta obstáculo se enciende led rojo y publica la hora actual por la app
  if (button == 1) {
    if (valor == HIGH) {
      Serial.println("Obstáculo detectado");
      digitalWrite(ledRojo, LOW);
      digitalWrite(ledVerde, HIGH);

      if (getLocalTime(&timeinfo)) {
        char buffer[64];// espacio para almacenar la cadena de fecha y hora
        strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", &timeinfo);//cadena con formato día/mes/año hora:minuto:segundo
        mqtt_client.publish("sensor/hora", buffer);//envía la hora actualizada del MQTT
        Serial.print("Hora enviada: ");
        Serial.println(buffer);
      } else {
        Serial.println("No se pudo obtener la hora.");
      }
      //Si no hay obstáculo led rojo off, led verde on
    } else {
      Serial.println("Corte de señal");
      digitalWrite(ledRojo, HIGH);
      digitalWrite(ledVerde, LOW);
    }
    //Si el botón está desactivado espera y mantiene solo el led rojo encendido
  } else {
    Serial.println("Esperando botón MQTT...");
    digitalWrite(ledRojo, HIGH);
    digitalWrite(ledVerde, LOW);
  }

  delay(1000);
}