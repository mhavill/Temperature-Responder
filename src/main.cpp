#include <Arduino.h>
#include <ESP8266WiFi.h>      // Include the Wi-Fi library
#include <ESP8266WiFiMulti.h> // Include the Wi-Fi-Multi library
#include <ESP8266mDNS.h>      // Include the mDNS library
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <arduino-timer.h>

// Include the libraries we need
#include <OneWire.h>
#include <DallasTemperature.h>

#include <secrets.h>

void printAddress(DeviceAddress deviceAddress);
bool temperature(void *);
void handleRoot();
void handleNotFound();
void sendTemperature(DeviceAddress deviceAddress);
void insideTemp();
void handleLedOn();
void handleLedOff();

auto timer = timer_create_default(); // create a timer with default settings

ESP8266WebServer server(80);
ESP8266WiFiMulti wifiMulti; // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'

const int led = 2;
bool ledON = true;

// Data wire is plugged into GPIO port 2 on the Arduino - which is D4 on our pinout
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
  Serial.println("Dallas Temperature IC Control Library Demo");
  Serial.print("compiled: ");
  Serial.print(__DATE__);
  Serial.println(__TIME__);

  // Set up WiFi STA
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);

  WiFi.mode(WIFI_STA);
  WiFi.hostname(device);

  delay(10);
  Serial.println('\n');

  wifiMulti.addAP(ssid1, password); // add Wi-Fi networks you want to connect to
  // wifiMulti.addAP(ssid2, password);
  // wifiMulti.addAP(ssid3, password);

  // Wait for connection
  Serial.println("Connecting ...");
  int i = 0;
  while (wifiMulti.run() != WL_CONNECTED)
  { // Wait for the Wi-Fi to connect: scan for Wi-Fi networks, and connect to the strongest of the networks above
    delay(1000);
    Serial.print(++i);
    Serial.print(' ');
  }
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

  if (MDNS.begin(device))
  {
    Serial.println("MDNS responder started");
  }
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
  sensors.setResolution(insideThermometer, 12);

  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(insideThermometer), DEC);
  Serial.println();

  // call the temperature function every 2000 millis (2 seconds)
  timer.every(2000, temperature);

  // Handle server communications
  server.on("/", handleRoot);

  server.on("/ledON", handleLedOn);
  server.on("/ledOFF", handleLedOff);

  server.on("/temp", insideTemp);

  server.on("/inline", []()
            { server.send(200, "text/plain", "this works as well"); });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);
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
}

// function to send the temperature for a device
void sendTemperature(DeviceAddress deviceAddress)
{
  float tempC = sensors.getTempC(deviceAddress);
  if (tempC == DEVICE_DISCONNECTED_C)
  {
    server.send(200, "text/plain", "Error: Could not read temperature data");
    return;
  }
  //temporarily holds data from vals
  char charVal[10];

  //4 is mininum width, 3 is precision; float value is copied onto buff
  dtostrf(tempC, 4, 2, charVal);

  server.send(200, "text/plain", charVal);
}

/*
 * Main function. 
 */
void loop(void)
{
  timer.tick();          // tick the timer
  server.handleClient(); //deal with communications
  MDNS.update();
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
  //Serial.print("   Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  //Serial.println("DONE");
  Serial.println("");

  // It responds almost immediately. Let's print out the data
  printTemperature(insideThermometer); // Use a simple function to print out the data
  return true;
}

void insideTemp()
{
  sendTemperature(insideThermometer);
}

void handleRoot()
{
  if(ledON) {digitalWrite(led, 1);}
  String message = "Hello from ";
  message += device;
  message += "! running on ";
  message += WiFi.SSID();
  server.send(200, "text/plain", message);
  digitalWrite(led, 0);
}

void handleNotFound()
{
  if(ledON) {digitalWrite(led, 1);}
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void handleLedOn() {
  ledON = true;
}

void handleLedOff() {
  ledON = false;
}
