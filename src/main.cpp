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
#ifndef TIMER
#define TIMER
#include <arduino-timer.h>
#endif

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

#ifndef VANMESH
#include <vanMesh.hpp>
#define VANMESH
#endif // VANMESH

// #include <PushNotification.hpp>

#ifdef ESP32
#include <WiFiClientSecure.h>
#else
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#endif // ESP32

#include <time.h>
#include <sys/time.h>



/*******************************
 * Protptypes
 *******************************/
void setup();
void loop();
/*******************************
 * Definitions
 *******************************/
auto timer_temp = timer_create_default();  // create a timer with default settings
auto timer_AIOTC = timer_create_default(); // create a timer with default settings

auto timer_bump = timer_create_default(); // create a timer with defalt settings
auto timer_send = timer_create_default(); // create a timer with default settings

const int led = 2;

WiFiClient wifiClient;
bool AIOTCconnected = false;

#undef max
#undef min

extern "C"
{
  int _write(int fd, char *ptr, int len)
  {
    (void)fd;
    return Serial.write(ptr, len);
  }
}

/*******************************
 * Setup
 *******************************/
void setup()
{
  delay(5000); // time to start monitor
  Serial.begin(115200);

  Serial.println("\nHome IOT Control Mesh");
  Serial.print("compiled: ");
  Serial.print(__DATE__);
  Serial.print("\t");
  Serial.println(__TIME__);

  pinMode(led, OUTPUT);
  digitalWrite(led, 0);

  // Device Properties
  initProperties();

  // Temperature
  temperatureSetup();
  // delay(1000);

  // AIOTC
  aiotcSetup();

  // Push Notifications
  // prowlSetup();

  delay(5000);

  timer_AIOTC.every(10000, AIOTCupdate); // Cloud update
  timer_temp.every(5000, temperature);   // temperature function
  timer_bump.every(10000,bumpLastCall);   // look for dead nodes

  // mesh
  vanMeshSetup();
  }

/*******************************
 * Loop
 *******************************/
void loop()
{
  delay(1);
  vanMeshLoop();
  timer_AIOTC.tick(); // tick the timer - looks after temperature, mesh and AIOTC
#ifdef FINAL
  timer_mesh.tick();
#endif // FINAL
  timer_temp.tick();
  timer_bump.tick();

  // if (!prowlsent)
  //   sendProwl();
}

/*******************************
 * Utility Functions
 *******************************/

/*******************************
 * Finite State Machine
 *******************************/