#include "smartlock.h"
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>  

SmartLockDevice::SmartLockDevice()
  : rfidReader(RFID_READER_PIN, this), lock(LOCK_SERVO_PIN, this) {}

void SmartLockDevice::on(Event event) {
  if (event == RfidReader::CARD_DETECTED_EVENT) {
    uint8_t* uid = rfidReader.getLastUID();
    bool access = validateUID(uid);
    if (access) {
      lock.handle(ServoLock::UNLOCK_COMMAND);
      Serial.println("[SmartLockDevice] Acceso concedido");
    } else {
      lock.handle(ServoLock::LOCK_COMMAND);
      Serial.println("[SmartLockDevice] Acceso denegado");
    }
  }
}

void SmartLockDevice::handle(Command command) {
  if (command == ServoLock::LOCK_COMMAND || command == ServoLock::UNLOCK_COMMAND) {
    Serial.printf("[SmartLockDevice] Estado cerradura: %s\n", command == ServoLock::UNLOCK_COMMAND ? "Abierta" : "Cerrada");
  }
}

bool SmartLockDevice::validateUID(uint8_t uid[4]) {
  WiFiClient client;
  HTTPClient http;
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[SmartLockDevice] WiFi no conectado");
    return false;
  }

  char uid_hex[9];
  sprintf(uid_hex, "%02X%02X%02X%02X", uid[0], uid[1], uid[2], uid[3]);

  http.begin("https://sweet-manager-iot.free.beeceptor.com/api/validate-access");
  http.addHeader("Content-Type", "application/json");

  DynamicJsonDocument doc(256);
  doc["rfid_uid"] = uid_hex;    // string, OK
  doc["room_id"] = 101;         // entero, sin comillas

  String requestBody;
  serializeJson(doc, requestBody);


  Serial.println("[SmartLockDevice] JSON enviado: " + requestBody);

  int httpResponseCode = http.POST(requestBody);
  Serial.printf("[SmartLockDevice] Código HTTP: %d\n", httpResponseCode);

  if (httpResponseCode != 200) {
    http.end();
    return false;
  }

  String response = http.getString();
  Serial.println("[SmartLockDevice] Respuesta cruda: " + response);

  DynamicJsonDocument responseDoc(256);
  DeserializationError err = deserializeJson(responseDoc, response);
  if (err) {
    Serial.print("[SmartLockDevice] Error al parsear JSON: ");
    Serial.println(err.c_str());
    http.end();
    return false;
  }

  bool access = responseDoc["access"];
  http.end();
  return access;
}


void SmartLockDevice::triggerRfidEvent(Event event) {
  rfidReader.on(event);
}
