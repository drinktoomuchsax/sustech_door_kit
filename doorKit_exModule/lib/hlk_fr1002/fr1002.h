#include <Arduino.h>
#include <vector>
uint8_t getParityByte(uint8_t COMMAND[]);
vector<uint16_t> receiveMsg(HardwareSerial serial, uint8_t command[], int commandLength);
void writeMsg(HardwareSerial serial, uint8_t command[], int commandLength);