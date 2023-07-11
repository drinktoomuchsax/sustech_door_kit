#include <Arduino.h>
#include <vector>
using namespace std;

uint8_t getParityByte(uint8_t COMMAND[], int length)
{
    uint8_t parityByte = COMMAND[2];
    for (uint32_t i = 3; i < length; i++) // 校验逆过程
    {
        parityByte ^= COMMAND[i];
    }
    return parityByte;
}
vector<uint16_t> receiveMsg(HardwareSerial serial, uint8_t command[], int commandLength)
{
    vector<uint16_t> DATA = {};
    uint16_t dat;
    for (int d = 0; d < commandLength; d++)
    {
        dat = serial.read();
        DATA.push_back(dat);
    }
    return DATA;
}

void writeMsg(HardwareSerial serial, uint8_t command[], int commandLength)
{
    for (int i = 0; i < commandLength; i++)
    {
        serial.write(command[i]);
    }
}
