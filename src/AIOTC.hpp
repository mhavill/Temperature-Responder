/*******************************
 * Header
 * Name:AIOTC.cpp
 * Purpose: The functions for integrating with Arduino IOT Cloud
 * Created Date: 09/02/2022
 *******************************/

/*******************************
 * Includes
 *******************************/
#ifdef IOTCLOUD
#include <Arduino_DebugUtils.h>
#include <ArduinoMqttClient.h>

#ifndef THINGPROPERTIES
#include <thingProperties.h>
#define THINGPROPERTIES
#endif

#ifndef SECRETS
#include <secrets.h>
#define SECRETS
#endif
#endif

/*******************************
 * Protptypes
 *******************************/
void aiotcSetup();
bool AIOTCupdate(void *);
void initProperties();
/*******************************
 * Definitions
 *******************************/
bool ArduinoCloudConnected;
#ifdef IOTCLOUD
WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);
#endif
/*******************************
 * Setup
 *******************************/
void aiotcSetup()
{
#ifdef IOTCLOUD
  initProperties();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
#endif
}

/*******************************
 * Loop
 *******************************/
bool AIOTCupdate(void *)
{
#ifdef IOTCLOUD
  ArduinoCloud.update();
  if (ArduinoCloud.connected())
  {
    ArduinoCloudConnected = true;
  }
  else
  {
    ArduinoCloudConnected = false;
      ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  }
#endif
  return true;
}
/*******************************
 * Utility Functions
 *******************************/

void initProperties()
{
#ifdef IOTCLOUD
  ArduinoCloudConnected = false;
  ArduinoCloud.setBoardId(DEVICE_LOGIN_NAME);
  ArduinoCloud.setSecretDeviceKey(DEVICE_KEY);
  ArduinoCloud.setThingId(THING_ID);

  ArduinoCloud.addProperty(count, READ, 10 * SECONDS, NULL);
  ArduinoCloud.addProperty(temp03, READ, 10 * SECONDS, NULL);
  ArduinoCloud.addProperty(temp04, READ, 10 * SECONDS, NULL);
  ArduinoCloud.addProperty(temp05, READ, 10 * SECONDS, NULL);
  ArduinoCloud.addProperty(temp01, READ, 10 * SECONDS, NULL);
  ArduinoCloud.addProperty(temp02, READ, 10 * SECONDS, NULL);
#endif
}

/*******************************
 * Finite State Machine
 *******************************/