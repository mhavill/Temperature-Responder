#include <Arduino.h>
#include <arduino-timer.h>

// Include the libraries we need
#include <OneWire.h>
#include <DallasTemperature.h>

#include <secrets.h>
#include "thingProperties.h"

void printAddress(DeviceAddress deviceAddress);
bool temperature(void *);
void printTemperature(DeviceAddress deviceAddress);

auto timer = timer_create_default(); // create a timer with default settings

const int led = 2;

// Data wire is plugged into GPIO port 2 on the ESP8266
#define ONE_WIRE_BUS 2

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

  Serial.begin(9600);
  Serial.println("\nDallas Temperature IC Control Library Demo");
  Serial.print("compiled: ");
  Serial.print(__DATE__);
  Serial.println(__TIME__);

  // Set up WiFi STA
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);

  // Defined in thingProperties.h
  initProperties();

  WiFi.mode(WIFI_STA);
  WiFi.hostname(device);

  delay(10);
  Serial.println('\n');
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Device Name: ");
  Serial.println(device);
  Serial.println();

    // call the temperature function every 10000 millis (10 seconds)
  timer.every(10000, temperature);

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  /*
     The following function allows you to obtain more information
     related to the state of network and IoT Cloud connection and errors
     the higher number the more granular information you’ll get.
     The default is 0 (only errors).
     Maximum is 4
  */
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

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
  sensors.setResolution(insideThermometer, RESOLUTION );

  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(insideThermometer), DEC);
  Serial.println();


}

/*
 * Main function. 
 */
void loop(void)
{
  timer.tick(); // tick the timer
  ArduinoCloud.update();
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
  Serial.println("DONE");
  Serial.println("");

  // It responds almost immediately. Let's print out the data
  printTemperature(insideThermometer); // Use a simple function to print out the data
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
  Serial.print("Temp C: ");
  Serial.print(tempC);
  //Send to IOT Cloud
  setTemp(tempC);
}
