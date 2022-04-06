/*******************************
 * Header
 * Name:Temperature.cpp
 * Purpose: Setup and obtain temperature readings and pass them to the controller
 * Created Date: 02/09/2022
 *******************************/

/*******************************
 * Includes
 *******************************/
#include <OneWire.h>
#include <DallasTemperature.h>
#ifndef VANMESH
#include <vanMesh.hpp>
#define VANMESH
#endif
/*******************************
 * Protptypes
 *******************************/
void temperatureSetup();
void printAddress(DeviceAddress deviceAddress);
bool temperature(void *);
void printTemperature(DeviceAddress deviceAddress);

/*******************************
 * Definitions
 *******************************/

// Data wire is plugged into
#ifdef espressif8266
#define ONE_WIRE_BUS 2  // GPIO port 2 on the ESP8266
#else                   // esp32
#define ONE_WIRE_BUS 23 // GPIO port 23 on the ESP32
#endif

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold device address
DeviceAddress insideThermometer;

/*******************************
 * Setup
 *******************************/
void temperatureSetup()
{
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
}

/*******************************
 * Loop
 *******************************/

/*******************************
 * Utility Functions
 *******************************/
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
  // Serial.print("   Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  // Serial.print("DONE\t");

  // It responds almost immediately. Let's print out the data
  printTemperature(insideThermometer); // Use a simple function to print out the data
  // char *result;
  // sendMessage(result);
  return true;
}

// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress)
{
  float tempC = sensors.getTempC(deviceAddress);
  if (tempC == DEVICE_DISCONNECTED_C)
  {
    Serial.println("Error: Could not read temperature data");
    return;
  }
  //DEBUG 
  // Serial.print(device);
  // Serial.print("\tC: ");
  // Serial.print(tempC);
  // Serial.print("\n ");
  // DEBUG
   nodearray[node].data1 = tempC;
   nodearray[node].data2 = 0;
   nodearray[node].data3 = 0;
   nodearray[node].status = 1;
   nodearray[node].lastcall = 0;
   sendMessage();
}

/*******************************
 * Finite State Machine
 *******************************/