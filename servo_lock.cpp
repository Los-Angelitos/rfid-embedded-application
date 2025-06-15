#include "servo_lock.h"

const Command ServoLock::UNLOCK_COMMAND = Command(2);
const Command ServoLock::LOCK_COMMAND = Command(1);

ServoLock::ServoLock(int pin, CommandHandler* handler)
  : Actuator(pin, handler), pin(pin) {
  servo.attach(pin);
  servo.write(0); // <- Cierra inmediatamente al arrancar

}

void ServoLock::handle(Command command) {
  if (command == UNLOCK_COMMAND) {
    servo.write(90); // posición para abrir
    Serial.println("[ServoLock] Cerradura abierta");
  } else if (command == LOCK_COMMAND) {
    servo.write(0); // posición para cerrar
    Serial.println("[ServoLock] Cerradura cerrada");
  }
}
