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

void setup()
{
    // Start serial communication
    Serial.begin(115200);

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
    if (deviceConnected)
    {
        delay(30000);
        char hello[] = {'o'};
        openCharacteristics.setValue(hello);
        openCharacteristics.notify();
        Serial.println("sent hello");

        char world[] = {'l'};
        lockCharacteristics.setValue(world);
        lockCharacteristics.notify();
        Serial.println("world");

        delay(30000);
    }
}