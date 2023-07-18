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
// STATE indicate the state of esp32. It could be syandby, verifyPSWD_wipeJitter, undifened and etc
String STATE = "undifeined";

// BLE part
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#define bleServerName "1106_door_kit" // BLE server name
bool deviceConnected = false;
#define SERVICE_UUID "91bad492-b950-4226-aa2b-4ede9fa42f59"                                                        // See the following for generating UUIDs: https://www.uuidgenerator.net/
BLECharacteristic openCharacteristics("cba1d466-344c-4be3-ab3f-189f80dd7518", BLECharacteristic::PROPERTY_NOTIFY); // open Characteristic and Descriptor
BLEDescriptor openDescriptor(BLEUUID((uint16_t)0x2902));
BLECharacteristic lockCharacteristics("ca73b3ba-39f6-4ab3-91ae-186dc9577d99", BLECharacteristic::PROPERTY_NOTIFY); // lock Characteristic and Descriptor
BLEDescriptor lockDescriptor(BLEUUID((uint16_t)0x2903));
class MyServerCallbacks : public BLEServerCallbacks // Setup callbacks onConnect and onDisconnect
{
  void onConnect(BLEServer *pServer)
  {
    deviceConnected = true;
  };
  void onDisconnect(BLEServer *pServer)
  {
    deviceConnected = false;
  }
};
/*
 | "o"   | open the door |
 | "l"   | lock the door |
 | "u"   | unlock       |*/
char oPen = 'o';
char lock = 'l';
char unlock = 'u';
bool OPEN = true;
bool LOCK = false;
void sendSignal(char signal, bool openORlock)
{
  if (openORlock)
  {
    char SIGNAL[] = {signal};
    openCharacteristics.setValue(SIGNAL);
    openCharacteristics.notify();
  }
  else if (!openORlock)
  {
    char SIGNAL[] = {signal};
    lockCharacteristics.setValue(SIGNAL);
    lockCharacteristics.notify();
  }
}

// keyboard part
#include <keyboardMatrix.h>
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
// face recognition part
// #include <fr1002.h>
HardwareSerial SERfr1002(2); // rename Serial2 into SERfr1002 (stand for serial for fr1002)
uint8_t set_standby[6] = {0xEF, 0xAA, 0x23, 0x00, 0x00, 0x23};
uint8_t get_status[6] = {0xEF, 0xAA, 0x11, 0x00, 0x00, 0x11};
uint8_t go_recognization[8] = {0xEF, 0xAA, 0x12, 0x00, 0x02, 0x00, 0x0A, 0x1A}; // timeout 10s, parity is 0x1A
uint8_t get_usernameANDid[6] = {0xEF, 0xAA, 0x24, 0x00, 0x00, 0x24};

// led setup
const int ledA = 10;
const int ledB = 11;
bool testState = false;
bool lockState = false;

void ledBlink(int whichLed, int circleTime, int gapTime)
{
  if (circleTime > 0)
  {
    for (int Z = 0; Z < circleTime; Z++)
    {
      digitalWrite(whichLed, HIGH);
      delay(gapTime);
      digitalWrite(whichLed, LOW);
      delay(gapTime);
    }
  }
  if (lockState) // long light for lockLED red
    digitalWrite(ledB, HIGH);
  else
    digitalWrite(ledB, LOW);
}

bool switchState(bool state) // switch testState or lockState
{
  if (testState)
  {
    Serial.println("switchState is here");
  }
  if (state)
  {
    if (testState)
      Serial.println("ops false");
    return false;
  }
  else
  {
    if (testState)
      Serial.println("ops true");
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

  // BLE setup
  BLEDevice::init(bleServerName);                 // Create the BLE Device
  BLEServer *pServer = BLEDevice::createServer(); // Create the BLE Server
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *doorService = pServer->createService(SERVICE_UUID); // Create the BLE Service
  doorService->addCharacteristic(&openCharacteristics);           // Create BLE Characteristics and Create a BLE Descriptor
  openDescriptor.setValue("open door signal");
  openCharacteristics.addDescriptor(&openDescriptor);
  doorService->addCharacteristic(&lockCharacteristics);
  lockDescriptor.setValue("lock door signal");
  lockCharacteristics.addDescriptor(new BLE2902());
  doorService->start();                                       // Start the service
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising(); // Start advertising
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}
void loop()
{
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
          sendSignal(oPen, OPEN);
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
          if (lockState)
            sendSignal(lock, LOCK);
          else
            sendSignal(unlock, LOCK);
          STATE = "afterLock";
          delay(1000);
        }
        else if (verifyPSWD(realpswd) == "TEST") // trun on/off test mode
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

      if (((millis() - lastTime) > timerDelay)) // timeout wipe off the pswd and led blink
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
  ledBlink(ledB, 0, 0); // refrsh the led for lockLED red
}