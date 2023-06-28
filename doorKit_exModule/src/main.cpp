// part of the BLE code comes from Rui Santos' instructions at https://RandomNerdTutorials.com/esp32-ble-server-client/
#include <Arduino.h>
// keyboard part
#include <vector>
using namespace std;
// STATE indicate the state of esp32. It could be syandby, verifyPSWD_wipeJitter, undifened and etc
String STATE;
// BLE part
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
// face recognition part
// #include <fr1002.h>

HardwareSerial SERfr1002(2); // rename Serial2 into SERfr1002 (stand for serial for fr1002)

// keyboard part
vector<int> setpassword = {6, 0, 6, 0};                                 // set your password here !!!
vector<int> pswd;                                                       // initialize a password vector(array) to store the key user press
vector<int> realpswd;                                                   // realpswd
int keyboardMap[4][3] = {{7, 8, 9}, {4, 5, 6}, {1, 2, 3}, {-6, 0, -9}}; // keyboardMap[i][j], it depend on you
int keyboardOutput[3] = {21, 22, 23};                                   // defining gpio output pin
int keyboardInput[4] = {33, 32, 35, 4};                                 // defining gpio input pin

// BLE part
#define bleServerName "SB_Pool" // config name of your dorm here
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;
bool deviceConnected = false;
#define SERVICE_UUID "a06dea61-ccf7-450f-b6fb-014c1e3901e0"

// Characteristic and Descriptor
BLECharacteristic doorOperateCharacteristics("03f71a0d-1a07-4882-99d4-0e9c31b69284", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor doorOperateDescriptor(BLEUUID((uint16_t)0x2902)); // 0x2902 for Client Characteristic Configuration
BLECharacteristic touchOperateCharacteristics("38c11033-ac2a-4ed3-b2b2-29562f72a44b", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor touchDescriptor(BLEUUID((uint16_t)0x2902));
BLECharacteristic otaCharacteristics("83d77575-d864-48ab-800f-13327c51cbd1", BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor otaDescriptor(BLEUUID((uint16_t)0x2902));
BLECharacteristic rebootOperateCharacteristics("d88e016a-0aa6-4fad-97db-202a042f29d5", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor rebootDescriptor(BLEUUID((uint16_t)0x2902));

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

// face recognition part
uint8_t set_standby[6] = {0xEF, 0xAA, 0x23, 0x00, 0x00, 0x23};
uint8_t get_status[6] = {0xEF, 0xAA, 0x11, 0x00, 0x00, 0x11};
uint8_t go_recognization[8] = {0xEF, 0xAA, 0x12, 0x00, 0x02, 0x00, 0x0A, 0x1A}; // timeout 10s, parity is 0x1A
uint8_t get_usernameANDid[6] = {0xEF, 0xAA, 0x24, 0x00, 0x00, 0x24};

/**
 * @brief detect which key been pressed
 * @note keyboard map
 *           0 1 2     j    i for raw, i<=4
 *       0 | 7 8 9 |        j for column, j<=3
 *       1 | 4 5 6 |        "C" is -6, which is clear
 *       2 | 1 2 3 |        "#" is -9, which is enter
 *       3 | C 0 # |
 *       i
 * @date 2023.6.20
 * @author weiyoudong，drinktoomuchsax
 * @version 0.0.8
 */
int whichKeyPress(int GPIO_output[3], int GPIO_input[4])
{
  for (int j = 0; j < 3; j++) // this is output loop
  {
    pinMode(GPIO_output[j], OUTPUT);
    digitalWrite(GPIO_output[j], HIGH);
    for (int i = 0; i < 4; i++) // this is input loop
    {
      pinMode(GPIO_input[i], INPUT);
      if (digitalRead(GPIO_input[i]) == 1)
      {
        // Serial.print(keyboardMap[i][j]);
        digitalWrite(GPIO_output[j], LOW);
        return keyboardMap[i][j];
      }
    }
    digitalWrite(GPIO_output[j], LOW);
  }
  // Serial.printf("no key been pressed\n");
  return -1; // return -1 for find nothing
}

/**
 * @brief find a way to wipe off the jitter of key pressing, like 6666666666666666666666666666-16-166-166-16-1-1-1-1-1-1-1-13-13-133-1333333333{more than 1k 3 in 1ms}333333333333-1333-13-13-1-1-1-1-1{more than 1k -1 in 1ms}-1-1-1-1-13-1-1-1-133-13333-13333333333333-1-1-1-1-1-1-9-9-9
 * this function devide the raw password into interval, counting the most one as real password. If you want to do some develop, don't try to understand this one, rewrite your onw to save time.
 * @date 2023.6.23
 * @author weiyoudong, drinktoomuchsax
 * @version 0.2.0
 */
int last;
bool gap = true;
int wipeJitter(vector<int> rawPSWD)
{
  vector<int> realPSWD;
  int checkLength = rawPSWD.size(); // this can be changed
  int number[5] = {-1, -1, -1, -1, -1};
  int num = 0;
  int countNum[5] = {0, 0, 0, 0, 0};
  for (int j = 0; j < checkLength; j++)
  {
    int equal = 0;
    for (int x = 0; x < 5; x++)
    {
      if (number[x] == rawPSWD[j])
      {
        countNum[x] = countNum[x] + 1;
        equal = 1;
        break;
      }
    }
    if (equal == 0)
    {
      number[num] = rawPSWD[j];
      countNum[num] = countNum[num] + 1;
      num = num + 1;
    }
  }
  for (int z = 0; z < 5; z++)
  {
    bool needToBeMost = (countNum[z] > 0.9 * checkLength) && (number[z] >= 0) && (gap == true);
    bool dontRepeatSameOne = (countNum[z] > 0.9 * checkLength) && (number[z] >= 0) && (number[z] != last);
    if (needToBeMost || dontRepeatSameOne)
    {
      last = number[z];
      gap = false;
      return number[z]; // coefficient1 can be changed
    }
    bool areYouGap = (countNum[z] > checkLength * 0.5) && (number[z] == -1) && (gap == false);
    if (areYouGap)
      gap = true;
  }
  return -1;
}

/**
 * @brief passwords verifing, using "xxx" represent wahtever key, the real password is hiden in the key.
 *        so, the actual password woulb be defined as xxx20231xxxxxxxxx , "x" is whatever number except "#", "2023" is the real password and following 1 is user identify bit to record who open the door.
 *        The user could keep typing and typing but once them press "enter", the password would be verifyed.
 * @date 2023.6.20
 * @author drinktoomuchsax
 * @version 0.0.3
 */
bool verifyPSWD(vector<int> PSWD)
{
  if (PSWD.size() >= 6)
  {
    bool one, two, three, four;
    one = (setpassword[0] == PSWD[3]);
    two = (setpassword[1] == PSWD[4]);
    three = (setpassword[2] == PSWD[5]);
    four = (setpassword[3] == PSWD[6]);
    if (one && two && three && four) // all four bit is right, the
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}

void setup()
{
  Serial.begin(115200);
  SERfr1002.begin(115200, SERIAL_8N1, 16, 17); // uart port for hlk-fr1002 face recogniton module with baud rate 115200 bps, 8_data_bit, No_parity, 1_stop_bit

  BLEDevice::init(bleServerName);
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  // Create the BLE Service
  BLEService *doorKitService = pServer->createService(SERVICE_UUID);
  // Create BLE Characteristics and Descriptor
  // doorOperate
  doorKitService->addCharacteristic(&doorOperateCharacteristics);
  doorOperateDescriptor.setValue("OPEN or LOCK");
  doorOperateCharacteristics.addDescriptor(&doorOperateDescriptor);
  // handle touch
  doorKitService->addCharacteristic(&touchOperateCharacteristics);
  touchDescriptor.setValue("handle been touched");
  touchOperateCharacteristics.addDescriptor(&touchDescriptor);
  //  ota
  doorKitService->addCharacteristic(&otaCharacteristics);
  otaDescriptor.setValue("over-the-air update");
  otaCharacteristics.addDescriptor(&otaDescriptor);
  // reboot
  doorKitService->addCharacteristic(&rebootOperateCharacteristics);
  rebootDescriptor.setValue("reboot signal from exModule");
  rebootOperateCharacteristics.addDescriptor(&rebootDescriptor);
  // Start the service
  doorKitService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}
void loop()
{
  // 0.5ms finish write for loop

  delay(500);                          // wait fr1002 to initial
  if (STATE != "sendRecg_waitRespons") // add lidar dsitance condition
  {
    if (SERfr1002.availableForWrite())
    {
      for (int i = 0; i < 8; i++)
      {
        SERfr1002.write(go_recognization[i]);
        STATE = "sendRecg_waitRespons";
      }
    }
    // face match: 0xEF 0xAA 0x00 0x00 0x26 0x12 0x00 (36bytes) 0x23
    //             EF AA 00 00 26 12 00 00 01 73 61 78 00 00
    // face fail: 0xEF 0xAA 0x00 0x00 0x26 0x12 0x0D (36bytes) 0x23
    delay(1000);
    vector<uint16_t>
        DATA = {};
    uint16_t dat;
    for (int d = 0; d < 44; d++)
    {
      dat = SERfr1002.read();
      Serial.printf("%x ", dat);
      DATA.push_back(dat);
    }
    Serial.println();
  }
  // 0.7ms finish to receive

  delay(10000);

  if (deviceConnected)
  {
    STATE = "standby";

    if (whichKeyPress(keyboardOutput, keyboardInput) != -1)
    {
      STATE = "checkPSWD";
      // keyboard password part, detail look on productdesign.excalidraw
      while (STATE == "checkPSWD")
      {
        pswd.push_back(whichKeyPress(keyboardOutput, keyboardInput)); // save the key into an array(actully vector)
        if (pswd.size() == 100)
        {
          int keyBeenPressed = wipeJitter(pswd);
          if (keyBeenPressed != -1)
          {
            realpswd.push_back(keyBeenPressed);
            Serial.printf("add to \"%d\" password\n", keyBeenPressed);
          }
          vector<int> clearpswd;
          pswd.swap(clearpswd);
        }

        if (whichKeyPress(keyboardOutput, keyboardInput) == -9)
        {
          Serial.print("realpswd is\n");
          for (int i = 0; i < realpswd.size(); i++)
          {
            Serial.printf("_%d", realpswd[i]);
          }
          STATE = "verifyPSWD_wipeJitter";

          if (verifyPSWD(realpswd) == 1) // match, open the door
          {
            doorOperateCharacteristics.setValue("1");
            doorOperateCharacteristics.notify();
            Serial.print("\nsend open door signal\n");
            STATE = "standby";
            delay(1000);
          }
          else if (verifyPSWD(realpswd) == -1) // match, lock/unlock the door
          {
            doorOperateCharacteristics.setValue("-1");
            doorOperateCharacteristics.notify();
            Serial.print("\nsend lock door signal\n");
            STATE = "standby";
            delay(1000);
          }
          else // do nothing but red led
          {
            Serial.print("verify wrong\n");
            STATE = "undefined";
            delay(500);
          };
          realpswd.clear();
        }
        else if (whichKeyPress(keyboardOutput, keyboardInput) == -6) // click "C" to clear
        {
          vector<int> clearpswd;
          pswd.swap(clearpswd);
          realpswd.swap(clearpswd);
          STATE = "verifyPSWD_wipeJitter";
          delay(500);
        }
      }
    }
  }
}