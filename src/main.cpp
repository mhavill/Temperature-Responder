/*******************************
 * Header
 * Name:main.cpp
 * Purpose: Calls components in overall solution
 * Created Date: 09/02/2022
 *******************************/

/*******************************
 * Includes
 *******************************/
#include <Arduino.h>
#include <arduino-timer.h>

#ifndef THINGPROPERTIES
#include <thingProperties.h>
#define THINGPROPERTIES
#endif

#ifndef SECRETS
#include <secrets.h>
#define SECRETS
#endif

#include <iostream>
#include <string>
#ifndef TEMPERATURE
#define TEMPERATURE
#include <Temperature.hpp>
#endif

#include <AIOTC.hpp>

#ifndef MESH
#include <Mesh.hpp>
#define MESH
#endif

#include <PushNotification.hpp>

#ifdef ESP32
// #include <WiFiClientSecure.h>
#else
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#endif

/*******************************
 * Protptypes
 *******************************/
void setup();
void loop();
/*******************************
 * Definitions
 *******************************/
auto timer = timer_create_default(); // create a timer with default settings
const int led = 2;
IPAddress myIP(0, 0, 0, 0);
WiFiClient wifiClient;
bool AIOTCconnected = false;

/*******************************
 * Setup
 *******************************/
void setup()
{
  Serial.begin(115200);

  Serial.println("\nHome IOT Control Mesh");
  Serial.print("compiled: ");
  Serial.print(__DATE__);
  Serial.print("\t");
  Serial.println(__TIME__);

  pinMode(led, OUTPUT);
  digitalWrite(led, 0);

  timer.every(10000, temperature);  // call the temperature function every 10000 millis (10 seconds)
  timer.every(10000, bumpLastCall); // A mesh function
  timer.every(10000, AIOTCupdate);    // Cloud update
  // timer.every(5000, sendMessage); //Mesh Broadcast

  // Temperature
  temperatureSetup();
  // AIOTC
  aiotcSetup();
  // Mesh
  // if (ArduinoCloudConnected)
  // {
  //   AIOTCconnected = true;
  //   meshSetup();
  // }
    meshSetup();
  // Push Notifications
  prowlSetup();
}

/*******************************
 * Loop
 *******************************/
void loop()
{
  timer.tick(); // tick the timer - looks after temperature, mesh and AIOTC
  // if (ArduinoCloudConnected)
  // {
  //   if (AIOTCconnected != true)
  //   {
  //     AIOTCconnected = true;
  //     meshSetup();
  //   }
  //   meshLoop();
  // }
  meshLoop();
  if (!prowlsent)
    sendProwl();

  if (myIP != getlocalIP())
  {
    myIP = getlocalIP();
    Serial.println("My IP is " + myIP.toString());
  }
}

/*******************************
 * Utility Functions
 *******************************/

/*******************************
 * Finite State Machine
 *******************************/