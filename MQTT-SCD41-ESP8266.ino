#include <Wire.h>
#include "SparkFun_SCD4x_Arduino_Library.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Pinout for ESP8266
#define SDA_PIN 14 //D5 - GPIO14
#define SCL_PIN 12 //D6 - GPIO12

SCD4x sensor;

float temp;
float humidity;
float co2;


// Variables de conexión WiFi
const char* ssid = "SmartLab";
const char* password = "D5cy6$3y)5&$wT$7ry";

// Variables de conexión MQTT
const char* mqtt_server = "192.168.0.18"; // Dirección IP del servidor MQTT
const char* mqtt_topic_temp = "temperaturaH";
const char* mqtt_topic_hume = "humedadH";
const char* mqtt_topic_co2 = "CO2H";

WiFiClient espClient;
PubSubClient client(espClient);


void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Dirección IP:");
  Serial.println(WiFi.localIP());
}


void reconnect() {
  // Loop hasta que estemos reconectados
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Conectado");
    } else {
      // Depurar: Imprimir el estado de la conexión MQTT
      Serial.print("Falló, rc=");
      Serial.print(client.state());
      Serial.println(" Intentando de nuevo en 5 segundos");
      delay(5000);
    }
  }
}


void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!sensor.begin()) {
    Serial.println("Error al iniciar el sensor!");
    while (1);
  }
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (sensor.readMeasurement()) {
    co2 = sensor.getCO2();
    temp = sensor.getTemperature();
    humidity = sensor.getHumidity();

    Serial.print("CO2: ");
    Serial.print(co2);
    Serial.print(" ppm, Temperatura: ");
    Serial.print(temp);
    Serial.print(" °C, Humedad: ");
    Serial.print(humidity);
    Serial.println(" %");

    // Publicar datos en los temas MQTT correspondientes
    String co2String = String(co2);
    String tempString = String(temp);
    String humString = String(humidity);
    
    // Publicar CO2
    String co2Payload = "{" + co2String + "}";
    Serial.print("Publicando mensaje de CO2: ");
    Serial.println(co2Payload);
    client.publish(mqtt_topic_co2, co2Payload.c_str());

    // Publicar temperatura
    String tempPayload = "{" + tempString + "}";
    Serial.print("Publicando mensaje de temperatura: ");
    Serial.println(tempPayload);
    client.publish(mqtt_topic_temp, tempPayload.c_str());
  
    // Publicar humedad
    String humPayload = "{" + humString + "}";
    Serial.print("Publicando mensaje de humedad: ");
    Serial.println(humPayload);
    client.publish(mqtt_topic_hume, humPayload.c_str());
  } else {
    Serial.println("Error al leer el sensor!");
  }
  
  delay(5000);
}
