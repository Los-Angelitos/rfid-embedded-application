#include <WiFi.h>
#include "smartlock.h"
#include <Arduino.h> 

// Configuración WiFi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

SmartLockDevice device;

void setup() {
  Serial.begin(115200);

  // Conexión WiFi
  WiFi.begin(ssid, password);
  Serial.print("[Setup] Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("[Setup] WiFi conectado con IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Se realiza polling activo del lector RFID
  device.triggerRfidEvent(RfidReader::CARD_DETECTED_EVENT);
  delay(1000); // Ajusta esto según lo responsivo que quieras el lector
}