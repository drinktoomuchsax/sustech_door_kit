/*********
  Rui Santos
  Complete instructions at https://RandomNerdTutorials.com/esp32-ble-server-client/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*********/
#include <Arduino.h>
#include "BLEDevice.h"

// Default Temperature is in Celsius
// Comment the next line for Temperature in Fahrenheit

// BLE Server name (the other ESP32 name running the server sketch)
#define bleServerName "SB_Pool"

/* UUID's of the service, characteristic that we want to read*/
// BLE Service
static BLEUUID doorKitServiceUUID("a06dea61-ccf7-450f-b6fb-014c1e3901e0");

// BLE Characteristics
static BLEUUID doorOperateCharacteristicsUUID("03f71a0d-1a07-4882-99d4-0e9c31b69284");
static BLEUUID touchOperateCharacteristicsUUID("38c11033-ac2a-4ed3-b2b2-29562f72a44b");
static BLEUUID otaCharacteristicsUUID("83d77575-d864-48ab-800f-13327c51cbd1");
static BLEUUID hrebootOperateCharacteristicsUUID("d88e016a-0aa6-4fad-97db-202a042f29d5");

// Flags stating if should begin connecting and if the connection is up
static bool doConnect = false;
static bool connected = false;

// Address of the peripheral device. Address will be found during scanning...
static BLEAddress *pServerAddress;

// Characteristicd that we want to read
static BLERemoteCharacteristic *doorOperateCharacteristics;
static BLERemoteCharacteristic *touchOperateCharacteristics;

// Activate notify
const uint8_t notificationOn[] = {0x1, 0x0};
const uint8_t notificationOff[] = {0x0, 0x0};

// Variables to store temperature and humidity
bool doorOperateChar = false;

// Flags to check whether new temperature and humidity readings are available
bool newdoorOperateChar = false;

// When the BLE Server sends a new temperature reading with the notify property
static void doorOperateNotifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic,
                                      uint8_t *pData, size_t length, bool isNotify)
{
  // store temperature value
  doorOperateChar = (bool *)pData;
  newdoorOperateChar = true;
}

// Connect to the BLE Server that has the name, Service, and Characteristics
bool connectToServer(BLEAddress pAddress)
{
  BLEClient *pClient = BLEDevice::createClient();

  // Connect to the remove BLE Server.
  pClient->connect(pAddress);
  Serial.println(" - Connected to server");

  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService *pRemoteService = pClient->getService(doorKitServiceUUID);
  if (pRemoteService == nullptr)
  {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(doorKitServiceUUID.toString().c_str());
    return (false);
  }

  // Obtain a reference to the characteristics in the service of the remote BLE server.
  doorOperateCharacteristics = pRemoteService->getCharacteristic(doorOperateCharacteristicsUUID);

  if (doorOperateCharacteristics == nullptr)
  {
    Serial.print("Failed to find our characteristic UUID");
    return false;
  }
  Serial.println(" - Found our characteristics");

  // Assign callback functions for the Characteristics
  doorOperateCharacteristics->registerForNotify(doorOperateNotifyCallback);
  return true;
}

// Callback function that gets called, when another device's advertisement has been received
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    if (advertisedDevice.getName() == bleServerName)
    {                                                                 // Check if the name of the advertiser matches
      advertisedDevice.getScan()->stop();                             // Scan can be stopped, we found what we are looking for
      pServerAddress = new BLEAddress(advertisedDevice.getAddress()); // Address of advertiser is the one we need
      doConnect = true;                                               // Set indicator, stating that we are ready to connect
      Serial.println("Device found. Connecting!");
    }
  }
};

void setup()
{
  // Start serial communication
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");

  // Init BLE device
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 30 seconds.
  BLEScan *pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(30);
}

void loop()
{
  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
  // connected we set the connected flag to be true.
  if (doConnect == true)
  {
    if (connectToServer(*pServerAddress))
    {
      Serial.println("We are now connected to the BLE Server.");
      // Activate the Notify property of each Characteristic
      doorOperateCharacteristics->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t *)notificationOn, 2, true);
      connected = true;
    }
    else
    {
      Serial.println("We have failed to connect to the server; Restart your device to scan for nearby BLE server again.");
    }
    doConnect = false;
  }
  // if new temperature readings are available, print in the OLED
  if (newdoorOperateChar)
  {
    newdoorOperateChar = false;
    if (doorOperateChar == 1)
    {
      Serial.println("open the door");
    }
    else if (doorOperateChar == -1)
    { // lock the door
    }
  }
  delay(1000); // Delay a second between loops.
}