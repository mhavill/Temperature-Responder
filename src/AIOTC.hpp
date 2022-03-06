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
void reconnect();
void wifiConnect();
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

  wifiConnect();

  delay(2000);
  initProperties();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  setDebugMessageLevel(4);
  ArduinoCloud.printDebugInfo();
#endif
}

/*******************************
 * Loop
 *******************************/
bool AIOTCupdate(void *)
{
#ifdef IOTCLOUD
  if (!ArduinoCloud.connected())
    reconnect();

  ArduinoCloud.update();
  // if (ArduinoCloud.connected())
  // {
  //   ArduinoCloudConnected = true;
  // }
  // else
  // {
  // // Serial.println ("lost connection to AIOTC");
  //   ArduinoCloudConnected = false;
  //     ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  // }

// Serial.println("Updating AIOTC");
// ArduinoCloud.update();
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

//=====================================
void reconnect()
{
  wifiConnect();
  // Loop until we're reconnected
  while (!ArduinoCloud.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Connect to Arduino IoT Cloud
    ArduinoCloud.update();

    // String clientId = "ESP8266Client-";   // Create a random client ID
    // clientId += String(random(0xffff), HEX);

    // Attempt to connect
    if (ArduinoCloud.connected())
    {
      Serial.println("AIOTC connected");

      //   client.subscribe(command1_topic);   // subscribe the topics here
      //   //client.subscribe(command2_topic);   // subscribe the topics here
    }
    else
    {
      Serial.print("failed, rc=");
      ArduinoCloud.printDebugInfo();
      Serial.println(" try again in 5 seconds"); // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void wifiConnect()
{
  delay(10);
  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi.setAutoReconnect(true);
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(SSID, PASS);

    Serial.print("\nConnecting to ");
    Serial.println(SSID);
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
    }
  }
  Serial.print("\nWiFi connected\nIP address: ");
  Serial.println(WiFi.localIP());
}

/*******************************
 * Finite State Machine
 *******************************/