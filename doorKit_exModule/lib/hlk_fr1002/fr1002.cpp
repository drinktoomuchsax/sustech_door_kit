#include <Arduino.h>
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