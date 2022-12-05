/*******************************
 * Header
 * Name: vanMesh.h
 * Purpose: Mesh code for Vanguard project
 * Created Date: 06/04/2022
 *******************************/
// DONE remove reference to temperature
// TODO remove non mesh elements
// DONE make into called from main
// TODO make bridge selectable
// DONE make this work with Vanessa WiFi - Changed channel to 5

/*******************************
 * Includes
 *******************************/
// #include <Arduino.h>
#ifndef ATIMER
#include <arduino-timer.h>
#define ATIMER
#endif // ATIMER

#ifdef ESP32
// #include <WiFiClientSecure.h>
#else
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#endif

#ifndef THINGPROPERTIES
#include "thingProperties.hpp"
#define THINGPROPERTIES
#endif // THINGPROPERTIES

#ifndef SECRETS
#include <secrets.h>
#define SECRETS
#endif // SECRETS

#include <painlessMesh.h>

#ifdef IOTCLOUD
#include <PubSubClient.h>
#include "espprowl.hpp"
#endif // IOTCLOUD

#include <iostream>
#include <string>
#include <ctime>
#include <time.h>
#include <sys/time.h>

/*******************************
 * Protptypes
 *******************************/
#ifdef IOTCLOUD
void mqttCallback(char *topic, uint8_t *payload, unsigned int length);
bool mqttloop(void *);
#endif // IOTCLOUD
void receivedCallback(const uint32_t &from, const String &msg);
void newConnectionCallback(uint32_t nodeId);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback();
void nodeTimeAdjustedCallback(int32_t offset);
void reconnect();
IPAddress getlocalIP();
String scanprocessor(const String &var);
bool printNodeArray(void *);
bool meshloop(void *);
void storeInNodeArray(uint32_t from, String msg);
bool bumpLastCall(void *);
void sendMessage();
void initialiseMessage();
static void printStr(const char *str, int len);
void systemDate();
void setSysTime();
void resetLastcall(int);
std::string getToken(std::string str_msg, std::string from, std::string to);
// void sendProwl();
#ifndef GPS
static void printStr(const char *str, int len);
#endif // GPS

/*******************************
 * Definitions
 *******************************/
// TODO check if required
Scheduler userScheduler; // to control your personal task
painlessMesh mesh;
WiFiClient wifiClient;

// uint8_t station_ip[4] = {0, 0, 0, 0}; // IP of the server
IPAddress getlocalIP();
IPAddress myIP(0, 0, 0, 0);
IPAddress myAPIP(0, 0, 0, 0);

#ifdef IOTCLOUD
// hivemq pubblic broker address and port
char mqttBroker[] = "broker.hivemq.com";
#define MQTTPORT 1883
#define PUBPLISHSUFFIX "VanpainlessMesh/from/"
#define SUBSCRIBESUFFIX "VanpainlessMesh/to/"
#define PUBPLISHFROMGATEWAYSUFFIX PUBPLISHSUFFIX "gateway"
#define CHECKCONNDELTA 120 // check interval ( seconds ) for mqtt connection
PubSubClient mqttClient;
#endif // IOTCLOUD

bool calc_delay = false;
SimpleList<uint32_t> nodes;
uint32_t nsent = 0;
char buff[512];
uint32_t nexttime = 0;
uint8_t initialized = 0;

// TODO check sendmessage function  & update timer - changed to 5 secs from 1
Task taskSendMessage(TASK_SECOND * 5, TASK_FOREVER, &sendMessage);
extern bool initialised;
bool messageStored = false;
bool timeset = false;

bool verbose = false;

/*******************************
 * Setup
 *******************************/
void vanMeshSetup()
{
  // mesh.setDebugMsgTypes(ERROR | STARTUP); // set before init() so that you can see startup messages
  // mesh.setDebugMsgTypes(ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE);
  // mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION);
  mesh.setDebugMsgTypes(ERROR | STARTUP);

  // Channel set to 10. Make sure to use the same channel for your mesh and for you other
  // network (STATION_SSID)
  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 5); // Channel changed to 5 for Vanessa WiFI
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
#ifdef IOTCLOUD
  mesh.stationManual(STATION_SSID, STATION_PASSWORD);
  mesh.setHostname(HOSTNAME);

  // Bridge node, should (in most cases) be a root node. See [the wiki](https://gitlab.com/painlessMesh/painlessMesh/wikis/Possible-challenges-in-mesh-formation) for some background
  mesh.setRoot(true);
  // This node and all other nodes should ideally know the mesh contains a root, so call this on all nodes
  mesh.setContainsRoot(true);
  mesh.getStationIP();
  mesh.getAPIP();

  // TODO check following scheduler task?
  // userScheduler.addTask(taskSendMessage);
  // taskSendMessage.enable();
  mqttClient.setServer(mqttBroker, MQTTPORT);
  mqttClient.setCallback(mqttCallback);
  mqttClient.setClient(wifiClient);
  Serial.print("getStationIP: ");
  Serial.println(mesh.getStationIP());
#else
  mesh.setRoot(false);
#endif
  // This node and all other nodes should ideally know the mesh contains a root, so call this on all nodes
  mesh.setContainsRoot(true);

  Serial.print("Mesh APIP: ");
  Serial.println(mesh.getAPIP());
  initialiseMessage();
}

/*******************************
 * Loop
 *******************************/
bool meshloop(void *)
{
  mesh.update();
  return true;
}

bool mqttloop(void *)
{
#ifdef IOTCLOUD
  mqttClient.loop();
#endif // IOTCLOUD
  return true;
}

/*******************************
 * Utility Functions
 *******************************/

// messages received from the mqtt broker
void mqttCallback(char *topic, uint8_t *payload, unsigned int length)
{
#ifdef IOTCLOUD
  char *cleanPayload = (char *)malloc(length + 1);
  payload[length] = '\0';
  memcpy(cleanPayload, payload, length + 1);
  std::string msg = std::string(cleanPayload);
  free(cleanPayload);

  Serial.printf("mc t:%s  p:%s\n", topic, payload);

  std::string targetStr = std::string(topic).substr(strlen(SUBSCRIBESUFFIX));

  if (targetStr.compare("gateway") >= 0)
  {
    if (msg.compare("getNodes") >= 0)
    {
      // Serial.println("Getting Nodes");
      auto nodes = mesh.getNodeList(true);
      std::string str;
      for (auto &&id : nodes)
        str = str + std::to_string(id) + " ";
      mqttClient.publish(PUBPLISHFROMGATEWAYSUFFIX, str.c_str());
      // Serial.println(str.c_str());
    }
    if (msg.compare("getrt") >= 0)
    {
      // Serial.println("Getting Route");
      mqttClient.publish(PUBPLISHFROMGATEWAYSUFFIX, mesh.subConnectionJson(false).c_str());
    }
    if (msg.compare("asnodetree") >= 0)
    {
      // mqttClient.publish( PUBPLISHFROMGATEWAYSUFFIX, mesh.asNodeTree().c_str() );
    }
  }
  else if (targetStr.compare("broadcast") >= 0)
  {
    mesh.sendBroadcast(msg.c_str());
  }
  else
  {
    uint32_t target = strtoul(targetStr.c_str(), NULL, 10);
    if (mesh.isConnected(target))
    {
      mesh.sendSingle(target, msg.c_str());
    }
    else
    {
      mqttClient.publish(PUBPLISHFROMGATEWAYSUFFIX, "Client not connected!");
    }
  }
#endif // IOTCLOUD
}

// messages received from painless mesh network
void receivedCallback(const uint32_t &from, const String &msg)
{

  if (verbose)
  {
    Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
#ifdef IOTCLOUD
    String topic = PUBPLISHSUFFIX + String(from);
    mqttClient.publish(topic.c_str(), msg.c_str());
#endif // IOTCLOUD
  }
  if (initialised)
  {
    storeInNodeArray(from, msg);
  }
  else
  {
    (Serial.println("Not Initialised to store messages"));
  }
}

void newConnectionCallback(uint32_t nodeId)
{
  Serial.printf("--> Start: New Connection, nodeId = %u\n", nodeId);
  Serial.printf("--> Start: New Connection, %s\n", mesh.subConnectionJson(true).c_str());
}

void changedConnectionCallback()
{
  Serial.printf("Changed connections\n");

  nodes = mesh.getNodeList();
  Serial.printf("Num nodes: %d\n", nodes.size());
#ifdef IOTCLOUD
  node_Count = nodes.size() + 1; // Root included in count
#endif                           // IOTCLOUD
  Serial.printf("Connection list:");
  SimpleList<uint32_t>::iterator node = nodes.begin();
  while (node != nodes.end())
  {
    Serial.printf(" %u", *node);
    node++;
  }
  Serial.println();
  calc_delay = true;
}

void nodeTimeAdjustedCallback(int32_t offset)
{
  if (verbose)
    Serial.printf("Adjusted time %u Offset = %d\n", mesh.getNodeTime(), offset);
}

void onNodeDelayReceived(uint32_t nodeId, int32_t delay)
{
  Serial.printf("Delay from node:%u delay = %d\n", nodeId, delay);
}

void reconnect()
{
#ifdef IOTCLOUD
  // byte mac[6];
  char MAC[9];
  int i;

  // unique string
  // WiFi.macAddress(mac);
  // sprintf(MAC,"%02X",mac[2],mac[3],mac[4],mac[5]);
  sprintf(MAC, "%08X", (uint32_t)ESP.getEfuseMac()); // generate unique addresss.
  // Loop until we're reconnected
  while (!mqttClient.connected())
  {
    Serial.println("Attempting MQTT connection...");
    // Attemp to connect
    if (mqttClient.connect(/*MQTT_CLIENT_NAME*/ MAC))
    {
      Serial.println("MQTT Connected");
      mqttClient.publish(PUBPLISHFROMGATEWAYSUFFIX, "Ready!");
      mqttClient.subscribe(SUBSCRIBESUFFIX "#");
      prowlSetup();
      ProwlInitial();
      aiotcSetup();
    }
    else
    {
      Serial.print("Failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
      mesh.update();
      mqttClient.loop();
    }
  }
#endif // IOTCLOUD
}

IPAddress getlocalIP()
{
  return IPAddress(mesh.getStationIP());
}

String scanprocessor(const String &var)
{
  if (var == ("SCAN"))
    return mesh.subConnectionJson(false);
  return String();
}

void sendMessage()
{
  if (messageStored)
  {
    String msg = " N:";
    msg += nodearray[node].name;
    msg += " D1:";
    msg += nodearray[node].data1;
    msg += " D2:";
    msg += nodearray[node].data2;
    msg += " D3:";
    msg += nodearray[node].data3;
    msg += " L:";
    msg += nodearray[node].lastcall;
    msg += " S:";
    msg += nodearray[node].status;
    msg += " Date:";
    msg += nodearray[node].date;
    msg += " Time:";
    msg += nodearray[node].time;
    // msg += " ID:";
    // msg += nodearray[node].from;
    if (verbose)
    {
      Serial.print("Sending msg: ");
      Serial.println(msg.c_str());
    }
    mesh.sendBroadcast(msg);
        // printNodeArray();
    // TODO change to timer
    // if (nodearray[Bridge02].status >= 0 && nodearray[Bridge02].lastcall <= 3)  // an active connection with Bridge
    //   taskSendMessage.setInterval(random(TASK_SECOND * 10, TASK_SECOND * 15)); // slowed update
    // else
      taskSendMessage.setInterval(random(TASK_SECOND * 5, TASK_SECOND * 5)); // rapid update
  }
}

void storeInNodeArray(uint32_t from, String msg)
{
  char *end;
  // DEBUG
  //  Serial.print("Recd in StoreInNodeArray: ");
  //  Serial.println(from);

  msg += " ID:";
  msg += from;
  std::string str_msg = msg.c_str();

  // const char *tempC = tempToken.c_str();
  // TODO move to one time initialise
  // Parse incoming message - assumes data format is as defined in sendMessage()
  // Set up delimiter strings for each data element
  std::string nDel = "N:";    // Delimiting string
  std::string d1Del = "D1:";  // Delimiting string
  std::string d2Del = "D2:";  // Delimiting string
  std::string d3Del = "D3:";  // Delimiting string
  std::string lDel = "L:";    // Delimiting string - but since this is a mesg from the node Lastcall should get set to zero!
  std::string sDel = "S:";    // Delimiting string
  std::string dDel = "Date:"; // Delimiting string
  std::string tDel = "Time:"; // Delimiting string
  // std::string iDel = "ID:";   // Delimiting string

  std::string nodeToken = str_msg.substr(str_msg.find(nDel) + 2, str_msg.find(d1Del) - 3);
  int zeroPoint = nodeToken.find("0"); // expects to find int begining with a 0 e.g. dev099
  std::string subNodeToken = nodeToken.substr(zeroPoint);

  int node = atoi(subNodeToken.c_str()) - 1; // Adjusted for array (this is a local variable of node - NOT the const!!)
  nodearray[node].nodeid = node + 1;
  // DONE check the lengths of the token are correct
  std::string token;
  token = getToken(str_msg, nDel, d1Del);
  strcpy(nodearray[node].name, token.c_str()); // store the node name as string
  token = getToken(str_msg, d1Del, d2Del);
  nodearray[node].data1 = std::strtod(token.c_str(), &end); // store data1 as double
  token = getToken(str_msg, d2Del, d3Del);
  nodearray[node].data2 = std::strtod(token.c_str(), &end); // store data2 as double
  token = getToken(str_msg, d3Del, lDel);
  nodearray[node].data3 = std::strtod(token.c_str(), &end); // store data3 as double
  nodearray[node].lastcall = 0;                             // since this is a mesg from the node lastcall should get set to zero!
  token = getToken(str_msg, sDel, dDel);
  nodearray[node].status = atoi(token.c_str()); // store status as int
  token = getToken(str_msg, dDel, tDel);
  strcpy(nodearray[node].date, token.c_str()); // store the node date as string
  // token = getToken(str_msg, tDel, iDel);
  token = str_msg.substr(str_msg.find(tDel) + 5);
  strcpy(nodearray[node].time, token.c_str()); // store the node time as string

  nodearray[node].from = from; // store the nodeid
  messageStored = true;
  // Serial.println("Message Stored");
}

std::string getToken(std::string str_msg, std::string from, std::string to)
{
  int pos1 = str_msg.find(from);
  pos1 += from.length();
  int pos2 = str_msg.find(to);
  std::string token = str_msg.substr(pos1, pos2 - pos1 - 1);
  // Debug
  // Serial.print("Token: ");
  // Serial.println(token.c_str());
  return token;
}

bool printNodeArray(void *)
{
  Serial.print("\nNode: ");
  Serial.print("\tData1: ");
  Serial.print("\t\tData2: ");
  Serial.print("\t\tData3: ");
  Serial.print("\tLastcall: ");
  Serial.print("\tStatus: ");
  Serial.print("\tDate: ");
  Serial.print("\t\tTime: ");
  Serial.print("\t\tName: ");
  Serial.print("\t\tID: ");
  Serial.print("\n");
  for (int node = 0; node <= NODE_COUNT - 1; ++node)
  {
    Serial.print(nodearray[node].nodeid);
    Serial.print("\t");
    Serial.print(nodearray[node].data1, 6);
    Serial.print("\t");
    Serial.print(nodearray[node].data2, 6);
    Serial.print("\t");
    Serial.print(nodearray[node].data3);
    Serial.print("\t");
    Serial.print(nodearray[node].lastcall);
    Serial.print("\t\t");
    Serial.print(nodearray[node].status);
    Serial.print("\t\t");
    printStr(nodearray[node].date, 10);
    Serial.print("\t");
    printStr(nodearray[node].time, 9);
    Serial.print("\t");
    printStr(nodearray[node].name, 8);
    Serial.print("\t");
    Serial.print(nodearray[node].from);
    Serial.print("\n");
  }
  Serial.print("\n\n");
  // systemDate();     // TODO fix up data and time functions
  return true;
}
// TODO re-instate bump and reset capability
bool bumpLastCall(void *)
{
  int bumpcount = 0;
  for (int i = 0; i <= NODE_COUNT - 1; ++i)
  {
    nodearray[i].lastcall++;
  }
  for (int i = 0; i <= NODE_COUNT - 1; ++i)
  {
    nodearray[i].lastcall++;
    if (nodearray[i].status >= 0) // only track nodes that were online
    {
      bumpcount += nodearray[i].lastcall;
    }
    if (bumpcount >= MAXLASTCALL) // looks like we are not listening
    {
      resetLastcall(bumpcount);
      // DONE enable auto reset
#ifndef ESP32
      system_restart(); // ESP8266
#else
      esp_restart(); // ESP32
#endif
    }
  }
  return true;
}

void resetLastcall(int bumpcount)
{
  std::string numStr = std::to_string(bumpcount);

  for (int i = 0; i <= NODE_COUNT - 1; ++i)
  {
    nodearray[i].lastcall = 0;

    // set status to 9 to warn other nodes
    nodearray[node].status = 9;

#ifdef IOTCLOUD
    // notification, message, priority
    sendProwl("Lastcall reset", "360", 1);
#endif // IOTCLOUD
  }
}

// to avoid conflicts and racing issues initialise the store with a valid message
void initialiseMessage()
{
  for (int i = 0; i < NODE_COUNT; i++)
  {

    String msg = " N:";
    msg += namearray[i];
    msg += " D1:1.23 D2:5.67 D3:8.90 L:0 S:-1 Date:**/**/****  Time:**:**:** ID:0000000";
    storeInNodeArray(0, msg);
  }
}

void systemDate()
{

  // // current date/time based on current system
  // time_t now = time(0);

  // cout << "Number of sec since January 1,1970 is:: " << now << endl;

  // tm *gmt = gmtime(&now);

  // // print various components of tm structure.
  // cout << "Year:" << 1900 + gmt->tm_year << endl;
  // cout << "Month: " << 1 + gmt->tm_mon << endl;
  // cout << "Day: " << gmt->tm_mday << endl;
  // cout << "Time: " << 5 + gmt->tm_hour << ":";
  // cout << 30 + gmt->tm_min << ":";
  // cout << gmt->tm_sec << endl;
  // setSysTime();
}

void setSysTime()
{
  if (!timeset && initialised)
  {

    // std::string gpsdate = "We are going to store the date and time here!";

    // strcpy(gpsdate.c_str(), nodearray[GPS01].date);

    std::string gpsdate = nodearray[GPS01].date;
    Serial.print("gpsdate: ");
    Serial.println(gpsdate.c_str());
    int dd = atoi(gpsdate.substr(0, 2).c_str());
    int mm = atoi(gpsdate.substr(3, 2).c_str());
    int yyyy = atoi(gpsdate.substr(6, 4).c_str());

    std::string gpstime = nodearray[GPS01].time;
    int hh = atoi(gpstime.substr(0, 2).c_str());
    int min = atoi(gpstime.substr(3, 2).c_str());
    int ss = atoi(gpstime.substr(6, 2).c_str());
    Serial.println(yyyy);

    if (yyyy <= 2020)

    {
      time_t mytime = time(0);
      struct tm *gmt = gmtime(&mytime);
      gmt->tm_year = yyyy - 1970;
      gmt->tm_mon = mm;
      gmt->tm_mday = dd;
      gmt->tm_hour = hh;
      gmt->tm_min = min;
      gmt->tm_sec = ss;
      Serial.print(gmt->tm_year);
      Serial.println(gmt->tm_mon); // TODO sort out why it prints 30 before month
      Serial.println(gmt->tm_mday);
      Serial.println(gmt->tm_hour);
      Serial.println(gmt->tm_min);
      Serial.println(gmt->tm_sec);

      // const struct timeval tv = {mktime(gmt), 0};
      // settimeofday(&gmt, 0);
      // ESP32Time rtc;
      // rtc.setTime(dd, mm, yyyy, hh, min, ss);

      timeset = true;
    }
    else
      Serial.println("Time not received");
  }
}

#ifndef GPS
static void printStr(const char *str, int len)
{
  int slen = strlen(str);
  for (int i = 0; i < len; ++i)
    Serial.print(i < slen ? str[i] : ' ');
}
#endif // GPS
/*******************************
 * Finite State Machine
 *******************************/
