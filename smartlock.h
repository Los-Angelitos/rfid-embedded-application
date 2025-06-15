#define SMART_LOCK_DEVICE_H

#include "EventHandler.h"
#include "CommandHandler.h"
#include "rfid_reader.h"
#include "servo_lock.h"

#define RFID_READER_PIN 5
#define LOCK_SERVO_PIN 14


class SmartLockDevice : public EventHandler, public CommandHandler {
public:
  SmartLockDevice();
  void on(Event event) override;
  void handle(Command command) override;
  void triggerRfidEvent(Event event);

private:
  RfidReader rfidReader;
  ServoLock lock;
  bool validateUID(uint8_t uid[4]);
};
