// this is driver for HLK-LD2420 lidar module
#include <Arduino.h>

HardwareSerial SerialPort(2); // use UART2

void TouchEvent()
{
    Serial.printf("Touch Event.\r\n");
}

void PinIntEvent()
{
}

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);
    SerialPort.begin(57600, SERIAL_8N2, 16, 17);

    pinMode(0, INPUT_PULLUP);
    attachInterrupt(0, PinIntEvent, FALLING);
}

void loop()
{
    int sig = SerialPort.read();
    Serial.printf("%x\n", sig);
    delay(2000);
}