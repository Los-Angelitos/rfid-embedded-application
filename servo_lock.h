
#include "Actuator.h"
#include <ESP32Servo.h>

class ServoLock : public Actuator {
public:
  static const Command UNLOCK_COMMAND;
  static const Command LOCK_COMMAND;

  ServoLock(int pin, CommandHandler* handler);
  void handle(Command command) override;

private:
  Servo servo;
  int pin;
};
