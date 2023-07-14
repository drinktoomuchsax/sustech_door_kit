#include <Arduino.h>

const int lockRED = 13;

void setup()
{
  Serial.begin(115200);
  pinMode(lockRED, OUTPUT);
  digitalWrite(lockRED, LOW);
}

void loop()
{
  if (Serial.available())
  {
    uint8_t c = Serial.read();
    if (c == 1)
    {
      digitalWrite(lockRED, HIGH);
      delay(200);
      digitalWrite(lockRED, LOW);
    }
    else if (c == 3)
    {
      digitalWrite(lockRED, HIGH);
      delay(200);
      digitalWrite(lockRED, LOW);
    }
  }
}