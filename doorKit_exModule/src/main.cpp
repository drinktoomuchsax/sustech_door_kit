#include <Arduino.h>

const int ledA = 10;

void ledBlink(int circle)
{
  for (int i = 0; i < circle; i++)
  {
    digitalWrite(ledA, HIGH);
    delay(80);
    digitalWrite(ledA, LOW);
    delay(80);
  }
}
void setup()
{
  Serial.begin(115200);
  pinMode(ledA, OUTPUT);
}

void loop()
{
  uint8_t c = 3;
  Serial.write(c);
  ledBlink(3);
  Serial.print(c, HEX);
  delay(3000);
  ledBlink(6);
  delay(3000);
  /*Serial.print(76, BIN) gives "0100 1100"
Serial.print(76, OCT) gives "114"
Serial.print("L", DEC) gives "76"
Serial.print(76, HEX) gives "4C" */
}