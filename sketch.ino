#include <SPI.h>
#include <ESP32Servo.h>
#include <ArduinoJson.h>
#include <WiFi.h>           // Para conectarse a una red WiFi
#include <HTTPClient.h>     // Para hacer solicitudes HTTP (GET, POST, etc.)
#include <ArduinoJson.h>    // Para construir y parsear JSON


//primero definimos lo basico.
#define CS 5
const int servoPin = 2;
const int unlockPosition = 0;
const int lockPosition = 180;
bool tarjeta_presente = false;


Servo lockServo;


//se setea todo lo esencial
void setup() {
  Serial.begin(115200);
  SPI.begin(18, 19, 23, CS);
  pinMode(CS, OUTPUT);
  lockServo.attach(servoPin);
  WiFi.begin("Wokwi-GUEST", "", 6);  // canal 6 si estás en Wokwi

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado con IP: ");
  Serial.println(WiFi.localIP());
  closeLock(); // Puerta cerrada por defecto
}

bool readRFID(uint8_t uid[4]) {
  uint8_t command, response;

  // Enviar comando REQA
  digitalWrite(CS, LOW);
  SPI.transfer(0x00); // dummy
  command = 0x26;
  SPI.transfer(command);      // esta respuesta no importa
  response = SPI.transfer(0x00); // respuesta real
  digitalWrite(CS, HIGH);

  if (response != 0x0A) {
    return false; // No hay tarjeta presente
  }

  // Enviar comando anticollision
  digitalWrite(CS, LOW);
  SPI.transfer(0x00); // dummy
  command = 0x93;
  SPI.transfer(command); // respuesta no importa

  for (int i = 0; i < 4; i++) {
    uid[i] = SPI.transfer(0x00);
  }
  digitalWrite(CS, HIGH);

  return true;
}


bool validateUID(uint8_t uid[]) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi no conectado");
    return false;
  }

  HTTPClient http;
  http.begin("https://sweet-manager-iot.free.beeceptor.com/api/validate-access");
  http.addHeader("Content-Type", "application/json");

  char uid_hex[9];
  sprintf(uid_hex, "%02X%02X%02X%02X", uid[0], uid[1], uid[2], uid[3]);

  DynamicJsonDocument doc(256);
  doc["rfid_uid"] = uid_hex;
  doc["room_id"] = 101;

  String requestBody;
  serializeJson(doc, requestBody);

  int httpResponseCode = http.POST(requestBody);
  bool access = false;

  if (httpResponseCode == 200) {
  String response = http.getString();
  Serial.print("Respuesta cruda: ");
  Serial.println(response);

  DynamicJsonDocument responseDoc(256);
  DeserializationError error = deserializeJson(responseDoc, response);
  if (error) {
    Serial.print("Error al parsear JSON: ");
    Serial.println(error.c_str());
  } else {
    access = responseDoc["access"];
  }
}


  http.end();
  return access;
  }


void openLock() {
  lockServo.write(unlockPosition);
}

void closeLock() {
  lockServo.write(lockPosition);
}

uint8_t last_uid[4] = {0x00, 0x00, 0x00, 0x00};

void loop() {
  uint8_t current_uid[4];

  if (readRFID(current_uid)) {
    if (!tarjeta_presente || memcmp(current_uid, last_uid, 4) != 0) {
      tarjeta_presente = true;
      memcpy(last_uid, current_uid, 4);

      Serial.print("UID leído: ");
      for (int i = 0; i < 4; i++) {
        Serial.print("0x");
        Serial.print(current_uid[i], HEX);
        if (i < 3) Serial.print(" ");
      }
      Serial.println();

      if (validateUID(current_uid)) {
        Serial.println("Acceso concedido");
        openLock();
      } else {
        Serial.println("Acceso denegado");
        closeLock();
      }
    }
  } else {
    tarjeta_presente = false; // se fue la tarjeta
  }

  delay(500);
}
