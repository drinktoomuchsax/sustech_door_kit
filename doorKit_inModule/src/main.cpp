#include <Arduino.h>

const int lockRED = 13;
/*********
  Rui Santos
  Complete instructions at https://RandomNerdTutorials.com/esp32-ble-server-client/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*********/

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Wire.h>

// Default Temperature is in Celsius
// Comment the next line for Temperature in Fahrenheit
#define temperatureCelsius

// BLE server name
#define bleServerName "BME280_ESP32"

float temp;
float tempF;
float hum;

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;

bool deviceConnected = false;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define SERVICE_UUID "91bad492-b950-4226-aa2b-4ede9fa42f59"

// Temperature Characteristic and Descriptor
#ifdef temperatureCelsius
BLECharacteristic bmeTemperatureCelsiusCharacteristics("cba1d466-344c-4be3-ab3f-189f80dd7518", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor bmeTemperatureCelsiusDescriptor(BLEUUID((uint16_t)0x2902));
#else
BLECharacteristic bmeTemperatureFahrenheitCharacteristics("f78ebbff-c8b7-4107-93de-889a6a06d408", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor bmeTemperatureFahrenheitDescriptor(BLEUUID((uint16_t)0x2902));
#endif

// Humidity Characteristic and Descriptor
BLECharacteristic bmeHumidityCharacteristics("ca73b3ba-39f6-4ab3-91ae-186dc9577d99", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor bmeHumidityDescriptor(BLEUUID((uint16_t)0x2903));

// Setup callbacks onConnect and onDisconnect
class MyServerCallbacks : public BLEServerCallbacks
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

  // Create the BLE Device
  BLEDevice::init(bleServerName);

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *bmeService = pServer->createService(SERVICE_UUID);

  // Create BLE Characteristics and Create a BLE Descriptor
  // Temperature
  bmeService->addCharacteristic(&bmeTemperatureCelsiusCharacteristics);
  bmeTemperatureCelsiusDescriptor.setValue("BME temperature Celsius");
  bmeTemperatureCelsiusCharacteristics.addDescriptor(&bmeTemperatureCelsiusDescriptor);
  // Humidity
  bmeService->addCharacteristic(&bmeHumidityCharacteristics);
  bmeHumidityDescriptor.setValue("BME humidity");
  bmeHumidityCharacteristics.addDescriptor(new BLE2902());

  // Start the service
  bmeService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

void loop()
{
  if (deviceConnected)
  {
    if ((millis() - lastTime) > timerDelay)
    {
      char hello[5] = {'h', 'e', 'l', 'l', 'o'};
      // Set temperature Characteristic value and notify connected client
      bmeTemperatureCelsiusCharacteristics.setValue(hello);
      bmeTemperatureCelsiusCharacteristics.notify();
      Serial.print("sent hello");

      // Set humidity Characteristic value and notify connected client
      char world[5] = {'w', 'o', 'r', 'l', 'd'};
      bmeHumidityCharacteristics.setValue(world);
      bmeHumidityCharacteristics.notify();
      Serial.print("world");
      lastTime = millis();
    }
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
    digitalWrite(openDoor, LOW);
    digitalWrite(lockRED, LOW);
    openDoorState = "off";
    lockREDState = "off";
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
              Serial.println("GPIO 26 on");
              openDoorState = "on";
              digitalWrite(openDoor, HIGH);
            }
            else if (header.indexOf("GET /openDoorState/off") >= 0)
            {
              Serial.println("GPIO 26 off");
              openDoorState = "off";
              digitalWrite(openDoor, LOW);
            }
            else if (header.indexOf("GET /lockREDState/on") >= 0)
            {
              Serial.println("GPIO 27 on");
              lockREDState = "on";
              digitalWrite(lockRED, HIGH);
            }
            else if (header.indexOf("GET /lockREDState/off") >= 0)
            {
              Serial.println("GPIO 27 off");
              lockREDState = "off";
              digitalWrite(lockRED, LOW);
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