/**COPYRIGHT DELARE */
/*********
Rui Santos
Complete instructions at https://RandomNerdTutorials.com/esp32-ble-server-client/
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*********/
/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-client-server-wi-fi/

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/
/**
 * @author drinktoomuchsax@github.com
 * @date 2023-07-18
 * @copyright The copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 */

#include <Arduino.h>
// keyboard part
#include <vector>
using namespace std;

#include <keyboardMatrix.h>

// STATE indicate the state of esp32. It could be syandby, verifyPSWD_wipeJitter, undifened and etc
String STATE = "undifeined";
// face recognition part
// #include <fr1002.h>

HardwareSerial SERfr1002(2); // rename Serial2 into SERfr1002 (stand for serial for fr1002)

// keyboard part set your password in keyboardMatrix lib
vector<int> pswd;                  // initialize a password vector(array) to store the key user press
vector<int> realpswd;              // realpswd
int keyboardOutput[3] = {3, 4, 5}; // defining gpio output pin
#define key0 9
#define key1 12
#define key2 13
#define key3 8
int keyboardInput[4] = {key0, key1, key2, key3}; // defining gpio input pin
unsigned long lastTime = 0;                      // input timeout
unsigned long timerDelay = 30000;                // 30s

// face recognition part
uint8_t set_standby[6] = {0xEF, 0xAA, 0x23, 0x00, 0x00, 0x23};
uint8_t get_status[6] = {0xEF, 0xAA, 0x11, 0x00, 0x00, 0x11};
uint8_t go_recognization[8] = {0xEF, 0xAA, 0x12, 0x00, 0x02, 0x00, 0x0A, 0x1A}; // timeout 10s, parity is 0x1A
uint8_t get_usernameANDid[6] = {0xEF, 0xAA, 0x24, 0x00, 0x00, 0x24};

// led setup
bool testState = false;
bool lockState = false;

const int ledA = 10;
const int ledB = 11;

void ledBlink(int whichLed, int citcleTime, int gapTime)
{
  for (int Z = 0; Z < citcleTime; Z++)
  {
    digitalWrite(whichLed, HIGH);
    delay(gapTime);
    digitalWrite(whichLed, LOW);
    delay(gapTime);
  }
  if (lockState)
  {
    digitalWrite(ledB, HIGH);
  }
}

bool switchState(bool state)
{
  if (testState)
  {
    Serial.println("switchState is here");
  }
  if (state)
  {
    if (testState)
    {
      Serial.println("ops false");
    }
    return false;
  }
  else
  {
    if (testState)
    {
      Serial.println("ops true");
    }
    return true;
  }
}

void setup()
{
  Serial.begin(115200);

  // SERfr1002.begin(115200, SERIAL_8N1, 16, 17); // uart port for hlk-fr1002 face recogniton module with baud rate 115200 bps, 8_data_bit, No_parity, 1_stop_bit

  // led setup
  pinMode(ledA, OUTPUT);
  pinMode(ledB, OUTPUT);
}
void loop()
{
  // 0.5ms finish write for loop

  // delay(500);                          // wait fr1002 to initial
  // if (STATE != "sendRecg_waitRespons") // add lidar dsitance condition
  // {
  //   if (SERfr1002.availableForWrite())
  //   {
  //     for (int i = 0; i < 8; i++)
  //     {
  //       SERfr1002.write(go_recognization[i]);
  //       STATE = "sendRecg_waitRespons";
  //     }
  //   }
  //   // face match: 0xEF 0xAA 0x00 0x00 0x26 0x12 0x00 (36bytes) 0x23
  //   //             EF AA 00 00 26 12 00 00 01 73 61 78 00 00
  //   // face fail: 0xEF 0xAA 0x00 0x00 0x26 0x12 0x0D (36bytes) 0x23
  //   delay(1000);
  //   vector<uint16_t>
  //       DATA = {};
  //   uint16_t dat;
  //   for (int d = 0; d < 44; d++)
  //   {
  //     dat = SERfr1002.read();
  //     Serial.printf("%x ", dat);
  //     DATA.push_back(dat);
  //   }
  //   Serial.println();
  // }
  // // 0.7ms finish to receive

  // delay(10000);

  STATE = "standby";

  if (whichKeyPress(keyboardOutput, keyboardInput) != -1)
  {
    lastTime = millis(); // start record time
    STATE = "checkPSWD";
    // keyboard password part, detail look on productdesign.excalidraw
    while ((STATE == "checkPSWD"))
    {
      pswd.push_back(whichKeyPress(keyboardOutput, keyboardInput)); // save the key into an array(actully vector)
      if (pswd.size() == 100)                                       // deal with the jitter
      {
        int keyBeenPressed = wipeJitter(pswd);
        if (keyBeenPressed != -1)
        {
          realpswd.push_back(keyBeenPressed);
          ledBlink(ledA, 1, 50);
          if (testState)
          {
            Serial.printf("add to \"%d\" password\n", keyBeenPressed);
          }
        }
        vector<int> clearpswd; // clear the pswd vector to save memory
        pswd.swap(clearpswd);
      }

      if (whichKeyPress(keyboardOutput, keyboardInput) == -9) // click "#" to enter
      {
        if (testState) // print real pswd
        {
          Serial.print("realpswd is: ");
          for (int i = 0; i < realpswd.size(); i++)
          {
            Serial.printf("_%d", realpswd[i]);
          }
          Serial.println();
        }

        if (verifyPSWD(realpswd) == "TRUE") // match, open the door
        {
          uint8_t open = 0x01;
          Serial.write(open);
          ledBlink(ledA, 5, 80);
          STATE = "standby";
          delay(1000);
        }
        else if (verifyPSWD(realpswd) == "LOCK") // match, lock/unlock the door
        {
          lockState = switchState(lockState);

          if (testState)
          {
            if (lockState)
              Serial.print("\nsend lock door signal\n");
            else
              Serial.print("\nsend unlock door signal\n");
          }
          uint8_t lock = 0x03;
          Serial.write(lock);
          STATE = "standby";
          delay(1000);
        }
        else if (verifyPSWD(realpswd) == "TEST")
        {
          testState = switchState(testState);
          if (testState)
          {
            Serial.println("test mode");
          }
          else
          {
            Serial.println("exit test mode");
          }
          STATE = "undefined";
        }
        else // do nothing but red led
        {
          Serial.print("verify wrong\n");
          ledBlink(ledB, 5, 80);
          STATE = "undefined";
        };
        realpswd.clear();
      }
      else if (whichKeyPress(keyboardOutput, keyboardInput) == -6) // click "C" to clear
      {
        vector<int> clearpswd;
        pswd.swap(clearpswd);
        realpswd.swap(clearpswd);
        STATE = "clearPSWD";
        delay(500);
      }

      if (((millis() - lastTime) > timerDelay)) // timeout wipe off the pswd
      {
        vector<int> clearpswd;
        pswd.swap(clearpswd);
        realpswd.swap(clearpswd);
        STATE = "clearPSWD";
        ledBlink(ledB, 5, 200);
        ledBlink(ledA, 5, 200);
      }
    }
  }
}