#include <Arduino.h>
#include <arduino-timer.h>

// Include the libraries we need
#include <OneWire.h>
#include <DallasTemperature.h>

#include <secrets.h>
#include "thingProperties.h"

#include "mesh.h"

#ifdef ESP32

#else
#include "user_interface.h"
#endif

void printAddress(DeviceAddress deviceAddress);
bool temperature(void *);
void printTemperature(DeviceAddress deviceAddress);
void sendMessage(); // Prototype so PlatformIO doesn't complain
void bumpLastCall();

auto timer = timer_create_default(); // create a timer with default settings

const int led = 2;

// Data wire is plugged into GPIO port 2 on the ESP8266
#ifdef espressif8266
#define ONE_WIRE_BUS 2
#else //esp32
#define ONE_WIRE_BUS 23
#endif

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold device address
DeviceAddress insideThermometer;

/*
 * Setup function. Here we do the basics
 */
void setup(void)
{
  // start serial port

  Serial.begin(115200);
  Serial.println("\nDallas Temperature IC Control Library Demo");
  Serial.print("compiled: ");
  Serial.print(__DATE__);
  Serial.println(__TIME__);

  // Set up WiFi STA
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);

  // call the temperature function every 10000 millis (10 seconds)
  timer.every(10000, temperature);

  // locate devices on the bus
  Serial.print("Locating devices...");
  sensors.begin();
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  // report parasite power requirements
  Serial.print("Parasite power is: ");
  if (sensors.isParasitePowerMode())
    Serial.println("ON");
  else
    Serial.println("OFF");

  if (!sensors.getAddress(insideThermometer, 0))
    Serial.println("Unable to find address for Device 0");

  // show the addresses we found on the bus
  Serial.print("Device 0 Address: ");
  printAddress(insideThermometer);
  Serial.println();

  // set the resolution to 12 bit (Each Dallas/Maxim device is capable of several different resolutions)
  const int RESOLUTION = 12;
  sensors.setResolution(insideThermometer, RESOLUTION);

  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(insideThermometer), DEC);
  Serial.println();

  meshSetup();
}

/*
 * Main function. 
 */
void loop(void)
{
  timer.tick(); // tick the timer
  meshLoop();
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16)
      Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

bool temperature(void *)
{
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  Serial.print("   Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.print("DONE\t");

  // It responds almost immediately. Let's print out the data
  printTemperature(insideThermometer); // Use a simple function to print out the data
  return true;
}

// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress)
{
  tempC = sensors.getTempC(deviceAddress);
  if (tempC == DEVICE_DISCONNECTED_C)
  {
    Serial.println("Error: Could not read temperature data");
    return;
  }
  Serial.print(device);
  Serial.print("\tC: ");
  Serial.print(tempC);

  Serial.print("\tBroadcastCount: ");
  Serial.println(++broadcastCount);
  bumpLastCall();
}

void bumpLastCall()
{
  int lcTotal = 0;
  for (int i = 0; i <= 4; ++i)
  {
    nodearray[i].lastcall++;
    lcTotal = lcTotal + nodearray[i].lastcall;
  }
  if (lcTotal >= 60)
  {
    Serial.println("Oh No!!  -- last call total over 60! - RESETTING");
#ifdef ESP32
    ESP.restart();
#else
    ESP.reset();
#endif
  }
}