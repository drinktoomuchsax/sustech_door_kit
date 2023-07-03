/*********
  Rui Santos
  Complete instructions at https://RandomNerdTutorials.com/esp32-ble-server-client/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*********/
#include <Arduino.h>
#include <WiFi.h>

// WIFI part
const char *ssid = "1106-SBPool-Door";
const char *password = "01234567";

WiFiServer server(80);

String header;
// GPIO define
// 23 lockBLUE
// 22 nothingBUTgreen
// 21 lockRED
// 19 open
const int lockBLUE = 23;
const int nothingBUTgreen = 22;
const int lockRED = 21;
const int openDoor = 19;
String gpio23 = "off";
String gpio22 = "off";
String openDoorState = "off";
String lockREDState = "off";

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");

  // set gpio pin
  pinMode(lockBLUE, OUTPUT);
  pinMode(lockRED, OUTPUT);
  pinMode(nothingBUTgreen, OUTPUT);
  pinMode(openDoor, OUTPUT);
  digitalWrite(lockBLUE, LOW);
  digitalWrite(lockRED, LOW);
  digitalWrite(nothingBUTgreen, LOW);
  digitalWrite(openDoor, LOW);

  // WIFI part

  Serial.print("Setting AP (Access Point)…");
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
}

void loop()
{
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
            if (header.indexOf("GET /26/on") >= 0)
            {
              Serial.println("GPIO 26 on");
              openDoorState = "on";
              digitalWrite(openDoor, HIGH);
            }
            else if (header.indexOf("GET /26/off") >= 0)
            {
              Serial.println("GPIO 26 off");
              openDoorState = "off";
              digitalWrite(openDoor, LOW);
            }
            else if (header.indexOf("GET /27/on") >= 0)
            {
              Serial.println("GPIO 27 on");
              lockREDState = "on";
              digitalWrite(lockRED, HIGH);
            }
            else if (header.indexOf("GET /27/off") >= 0)
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
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");

            // Web Page Heading
            client.println("<body><h1>ESP32 Web Server</h1>");

            // Display current state, and ON/OFF buttons for GPIO 26
            client.println("<p>OpenSwitch " + openDoorState + "</p>");
            // If the openDoorState is off, it displays the ON button
            if (openDoorState == "off")
            {
              client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
            }
            else
            {
              client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
            }

            // Display current state, and ON/OFF buttons for GPIO 27
            client.println("<p>LockSwitch " + lockREDState + "</p>");
            // If the lockREDState is off, it displays the ON button
            if (lockREDState == "off")
            {
              client.println("<p><a href=\"/27/on\"><button class=\"button\">ON</button></a></p>");
            }
            else
            {
              client.println("<p><a href=\"/27/off\"><button class=\"button button2\">OFF</button></a></p>");
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