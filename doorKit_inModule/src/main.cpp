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
#include <WiFi.h>

// WIFI part
const char *ssid = "1106-SBPool-Door_1";
const char *password = "01234567";
WiFiServer server(80);
String header;

// key and led part
const int lockRED = 6;
const int openDoor = 7;
String gpio23 = "off";
String gpio22 = "off";
String openDoorState = "off";
String lockREDState = "off";
const int lockRedInput = 12; // two input key(open and lock)
const int openDoorInput = 18;
const int ledGreen = 13;
// door operate

void setHigh(int whichPin)
{
  digitalWrite(lockRED, LOW);
  digitalWrite(openDoor, LOW);
  digitalWrite(whichPin, HIGH);
}
void operateDoor(String operate) // dircetly operate Door
{
  if (operate == "standby")
  {
    openDoorState = "off";
    lockREDState = "off";
    digitalWrite(openDoor, LOW);
    digitalWrite(lockRED, LOW);
  }
  else if (operate == "open")
  {
    openDoorState = "on";
    lockREDState = "off";
    digitalWrite(openDoor, HIGH);
    digitalWrite(lockRED, LOW);
  }
  else if (operate == "lock")
  {
    openDoorState = "off";
    lockREDState = "on";
    digitalWrite(openDoor, LOW);
    digitalWrite(lockRED, HIGH);
  }
}
void switchBetweenOpen(bool DELAY) // swith Openstate
{
  if (openDoorState == "on")
  {
    operateDoor("standby");
    Serial.println("open off");
    if (DELAY)
      delay(500);
  }
  else if (openDoorState == "off")
  {
    operateDoor("open");
    Serial.println("open on");
    if (DELAY)
      delay(500);
  }
}
void switchBetweenLock(bool DELAY)
{
  if (lockREDState == "on")
  {
    operateDoor("standby");
    Serial.println("lock off");
    if (DELAY)
      delay(500);
  }
  else if (lockREDState == "off")
  {
    operateDoor("lock");
    Serial.println("lock on");
    if (DELAY)
      delay(500);
  }
}

// BLE part
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
  if (*openChar == 'o')
  {
    switchBetweenOpen(true);
    delay(10000);
    switchBetweenOpen(true);
  }
}

static void lockNotifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic,
                               uint8_t *pData, size_t length, bool isNotify)

{
  lockChar = (char *)pData;
  newLock = true;
  Serial.println(*lockChar);
  if (*openChar == 'l')
  {
    switchBetweenLock(true);
  }
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
  Serial.begin(115200);

  // set gpio pin
  pinMode(lockRED, OUTPUT);
  pinMode(openDoor, OUTPUT);
  digitalWrite(lockRED, LOW);
  digitalWrite(openDoor, LOW);
  pinMode(lockRedInput, INPUT);
  pinMode(openDoorInput, INPUT);
  pinMode(ledGreen, OUTPUT);
  digitalWrite(ledGreen, LOW);

  // WIFI part
  Serial.println("Setting AP (Access Point)…");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password); // launch the access point
  Serial.println("Wait 100 ms for AP_START...");
  delay(100);
  Serial.println("Setting the AP");
  IPAddress Ip(192, 168, 110, 6); // setto IP Access Point same as gateway
  IPAddress NMask(255, 255, 255, 0);
  WiFi.softAPConfig(Ip, Ip, NMask);
  IPAddress ip = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(ip);
  server.begin();

  // BLE part
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
  // BLE part
  if (doConnect == true)
  {
    if (connectToServer(*pServerAddress))
    {
      Serial.println("We are now connected to the BLE Server."); // Activate the Notify property of each Characteristic
      openCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t *)notificationOn, 2, true);
      lockCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t *)notificationOn, 2, true);
      connected = true;
    }
    else
    {
      Serial.println("We have failed to connect to the server; Restart your device to scan for nearby BLE server again.");
      digitalWrite(ledGreen, HIGH);
    }
    doConnect = false;
  }

  if (newOpen && newLock)
  {
    newOpen = false;
    newLock = false;
  }

  // press upper key set door open(clear other state), press lower key set door lock(clear other state), press both clear all
  if ((digitalRead(openDoorInput) == 1) && (digitalRead(lockRedInput) != 1))
  {
    switchBetweenOpen(true);
  }
  else if ((digitalRead(openDoorInput) != 1) && (digitalRead(lockRedInput) == 1))
  {
    switchBetweenLock(true);
  }
  else if ((digitalRead(openDoorInput) == 1) && (digitalRead(lockRedInput) == 1))
  {
    operateDoor("standby");
    delay(2000);
  }

  // blinking led
  if (openDoorState == "on")
  {
    digitalWrite(ledGreen, HIGH);
  }
  else if (openDoorState == "off")
  {
    digitalWrite(ledGreen, LOW);
  }

  if (lockREDState == "on")
  {
    digitalWrite(ledGreen, HIGH);
    delay(80);
    digitalWrite(ledGreen, LOW);
    delay(80);
  }
  else
  {
    digitalWrite(ledGreen, LOW);
  }

  // WIFI part copy from https://randomnerdtutorials.com/esp32-web-server-arduino-ide/,if you want to know how these code works, learn details there.
  WiFiClient client = server.available(); // Listen for incoming clients
  if (client)
  {                                // If a new client connects,
    Serial.println("New Client."); // print a message out in the serial port
    String currentLine = "";       // make a String to hold incoming data from the client
    int timeout = 0;
    while (client.connected())
    { // loop while the client's connected
      timeout++;
      if (client.available())
      {                         // if there's bytes to read from the client,
        char c = client.read(); // read a byte, then
        Serial.write(c);        // print it out the serial monitor
        header += c;
        if (c == '\n')
        {
          if (currentLine.length() == 0)
          {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // turns the GPIOs on and off
            if (header.indexOf("GET /openDoorState/on") >= 0)
            {
              Serial.println("/openDoorState/on");
              operateDoor("open");
            }
            else if (header.indexOf("GET /openDoorState/off") >= 0)
            {
              Serial.println("/openDoorState/off");
              operateDoor("standby");
            }
            else if (header.indexOf("GET /lockREDState/on") >= 0)
            {
              Serial.println("/lockREDState/on");
              operateDoor("lock");
            }
            else if (header.indexOf("GET /lockREDState/off") >= 0)
            {
              Serial.println("/lockREDState/off");
              operateDoor("standby");
            }

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { display: inline-block; padding: 15px 25px; font-size: 24px; cursor: pointer; text-align: center; text-decoration: none; outline: none;");
            client.println("color: #fff; background-color: #4CAF50; border: none; border-radius: 15px; box-shadow: 0 9px #999;}");
            client.println("button:hover { background-color: #3e8e41 }");
            client.println("button:active { background-color: #3e8e41; box-shadow: 0 5px #666; transform: translateY(4px); }");
            client.println(".button2 {background-color: #555555;}</style></head>");

            // Web Page Heading
            client.println("<body><h1>ESP32 Web Server</h1>");

            // Display current state, and ON/OFF buttons for GPIO 26
            client.println("<p>OpenSwitch " + openDoorState + "</p>");
            // If the openDoorState is off, it displays the ON button
            if (openDoorState == "off")
            {
              client.println("<p><a href=\"/openDoorState/on\"><button class=\"button\">ON</button></a></p>");
            }
            else
            {
              client.println("<p><a href=\"/openDoorState/off\"><button class=\"button button2\">OFF</button></a></p>");
            }

            // Display current state, and ON/OFF buttons for GPIO 27
            client.println("<p>LockSwitch " + lockREDState + "</p>");
            // If the lockREDState is off, it displays the ON button
            if (lockREDState == "off")
            {
              client.println("<p><a href=\"/lockREDState/on\"><button class=\"button\">ON</button></a></p>");
            }
            else
            {
              client.println("<p><a href=\"/lockREDState/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          }
          else
          { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        }
        else if (c != '\r')
        {                   // if you got anything else but a carriage return character,
          currentLine += c; // add it to the end of the currentLine
        }
      }
      else if (timeout > 1000 ^ 1000 ^ 1000) // 40Mhz
      {
        break;
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}