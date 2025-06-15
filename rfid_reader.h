#include "Sensor.h"
#include <Arduino.h> 

class RfidReader : public Sensor {
public:
  static const Event CARD_DETECTED_EVENT;

  RfidReader(int pin, EventHandler* handler);
  void loop();
  uint8_t* getLastUID();

private:
  int pin;
  uint8_t lastUid[4] ;
  unsigned long lastReadTime = 0;
};
