

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

#include "BLEDevice.h"
#define bleServerName "1106_door_kit"                                          // BLE Server name (the other ESP32 name running the server sketch)
static BLEUUID doorServiceUUID("91bad492-b950-4226-aa2b-4ede9fa42f59");        /* UUID's of the service, characteristic that we want to read*/
static BLEUUID openCharacteristicUUID("cba1d466-344c-4be3-ab3f-189f80dd7518"); // open Characteristics
static BLEUUID lockCharacteristicUUID("ca73b3ba-39f6-4ab3-91ae-186dc9577d99"); // lock Characteristic
static boolean doConnect = false;                                              // Flags stating if should begin connecting and if the connection is up
static boolean connected = false;
static BLEAddress *pServerAddress;                  // Address of the peripheral device. Address will be found during scanning...
static BLERemoteCharacteristic *openCharacteristic; // Characteristicd that we want to read
static BLERemoteCharacteristic *lockCharacteristic;
const uint8_t notificationOn[] = {0x1, 0x0}; // Activate notify
const uint8_t notificationOff[] = {0x0, 0x0};
char *openChar; // Variables to store temperature and humidity
char *lockChar;
boolean newOpen = false; // Flags to check whether new temperature and humidity readings are available
boolean newLock = false;

static void openNotifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic,
                               uint8_t *pData, size_t length, bool isNotify) // When the BLE Server sends a new open reading with the notify property

{
    openChar = (char *)pData;
    newOpen = true;
    Serial.println(*openChar);
}

static void lockNotifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic,
                               uint8_t *pData, size_t length, bool isNotify)

{
    lockChar = (char *)pData;
    newLock = true;
    Serial.println(*lockChar);
}

bool connectToServer(BLEAddress pAddress) // Connect to the BLE Server that has the name, Service, and Characteristics
{
    BLEClient *pClient = BLEDevice::createClient();
    pClient->connect(pAddress); // Connect to the remove BLE Server.
    Serial.println(" - Connected to server");
    BLERemoteService *pRemoteService = pClient->getService(doorServiceUUID); // Obtain a reference to the service we are after in the remote BLE server.
    if (pRemoteService == nullptr)
    {
        Serial.print("Failed to find our service UUID: ");
        Serial.println(doorServiceUUID.toString().c_str());
        return (false);
    }
    openCharacteristic = pRemoteService->getCharacteristic(openCharacteristicUUID); // Obtain a reference to the characteristics in the service of the remote BLE server.
    lockCharacteristic = pRemoteService->getCharacteristic(lockCharacteristicUUID);
    if (openCharacteristic == nullptr || lockCharacteristic == nullptr)
    {
        Serial.print("Failed to find our characteristic UUID");
        return false;
    }
    Serial.println(" - Found our characteristics");
    openCharacteristic->registerForNotify(openNotifyCallback); // Assign callback functions for the Characteristics
    lockCharacteristic->registerForNotify(lockNotifyCallback);
    return true;
}
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks // Callback function that gets called, when another device's advertisement has been received
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        if (advertisedDevice.getName() == bleServerName) // Check if the name of the advertiser matches
        {
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

    BLEDevice::init(""); // Init BLE device
    // Retrieve a Scanner and set the callback we want to use to be informed when we have detected a new device.  Specify that we want active scanning and start the scan to run for 30 seconds.
    BLEScan *pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->start(30);
}

void loop()
{
    // If the flag "doConnect" is true then we have scanned for and found the desired BLE Server with which we wish to connect.  Now we connect to it.  Once we are connected we set the connected flag to be true.
    if (doConnect == true)
    {
        if (connectToServer(*pServerAddress))
        {
            Serial.println("We are now connected to the BLE Server.");
            // Activate the Notify property of each Characteristic
            openCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t *)notificationOn, 2, true);
            lockCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t *)notificationOn, 2, true);
            connected = true;
        }
        else
        {
            Serial.println("We have failed to connect to the server; Restart your device to scan for nearby BLE server again.");
        }
        doConnect = false;
    }

    if (newOpen && newLock)
    {
        newOpen = false;
        newLock = false;
    }
    delay(1000); // Delay a second between loops.
}