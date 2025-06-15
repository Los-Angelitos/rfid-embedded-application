#include "rfid_reader.h"
#include <SPI.h>
#include <Arduino.h> 


const Event RfidReader::CARD_DETECTED_EVENT = Event(1);

RfidReader::RfidReader(int pin, EventHandler* handler)
  : Sensor(pin, handler), pin(pin){
  SPI.begin();
  pinMode(pin, OUTPUT);
}

void RfidReader::loop() {
  uint8_t command = 0x26, response;
  digitalWrite(pin, LOW);
  SPI.transfer(0x00);
  SPI.transfer(command);
  response = SPI.transfer(0x00);
  digitalWrite(pin, HIGH);

  if (response == 0x0A) {
    digitalWrite(pin, LOW);
    SPI.transfer(0x00);
    SPI.transfer(0x93);
    for (int i = 0; i < 4; ++i) lastUid[i] = SPI.transfer(0x00);
    digitalWrite(pin, HIGH);

    if (handler) handler->on(CARD_DETECTED_EVENT);
  }
}

uint8_t* RfidReader::getLastUID() {
  static uint8_t fakeUid[4] = {0xDE, 0xAD, 0xBE, 0xEF};
  return fakeUid;
}
