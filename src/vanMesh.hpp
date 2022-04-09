/*******************************
 * Header
 * Name: vanMesh.h
 * Purpose: Mesh code for Vanguard project
 * Created Date: 06/04/2022
 *******************************/
// TODO remove reference to temperature DONE
// TODO remove non mesh elements
// TODO make into called from main  DONE
// TODO make bridge selectable

/*******************************
 * Includes
 *******************************/
#include <Arduino.h>
#include <arduino-timer.h>

#ifdef ESP32
// #include <WiFiClientSecure.h>
#else
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#endif

#ifndef THINGPROPERTIES
#include <thingProperties.hpp>
#define THINGPROPERTIES
#endif // THINGPROPERTIES

#ifndef SECRETS
#include <secrets.h>
#define SECRETS
#endif // SECRETS

#include <painlessMesh.h>

#include <iostream>
#include <string>
#include <ctime>
#include <time.h>
#include <sys/time.h>

/*******************************
 * Protptypes
 *******************************/
void receivedCallback(uint32_t from, String &msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback();
void nodeTimeAdjustedCallback(int32_t offset);

void storeInNodeArray(uint32_t from, String msg);
void printNodeArray();
bool bumpLastCall(void *);
// bool AIOTCupdate(void *);
void sendMessage();
void initialiseMessage();
static void printStr(const char *str, int len);
void systemDate();
void setSysTime();
std::string getToken(std::string str_msg, std::string from, std::string to);
// void sendProwl();
#ifndef GPS
static void printStr(const char *str, int len);
#endif // GPS

/*******************************
 * Definitions
 *******************************/
Scheduler userScheduler; // to control your personal task
painlessMesh mesh;
// WiFiClient wifiClient;
#define STATION_PORT 5555
// uint8_t station_ip[4] = {0, 0, 0, 0}; // IP of the server
// IPAddress getlocalIP();
// IPAddress myIP(0, 0, 0, 0);
Task taskSendMessage(TASK_SECOND * 1, TASK_FOREVER, &sendMessage);
extern bool initialised;
bool messageStored = false;
bool timeset = false;

// uint32_t freeMem;
// bool prowlsent;
/*******************************
 * Setup
 *******************************/
void vanMeshSetup()
{

#ifdef IOTCLOUD

  // Defined in thingProperties.h
  initProperties();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  setDebugMessageLevel(4);
  ArduinoCloud.printDebugInfo();
#endif
  mesh.setDebugMsgTypes(ERROR | STARTUP); // set before init() so that you can see startup messages
  // mesh.setDebugMsgTypes(ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE);
  // mesh.setDebugMsgTypes(ERROR | CONNECTION);

  // mesh.stationManual(STATION_SSID, STATION_PASSWORD);
  // mesh.setHostname(HOSTNAME);
  // mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 6);
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler);
  // mesh.stationManual(STATION_SSID, STATION_PASSWORD, STATION_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();

#ifdef IOTCLOUD
  // Bridge node, should (in most cases) be a root node. See [the wiki](https://gitlab.com/painlessMesh/painlessMesh/wikis/Possible-challenges-in-mesh-formation) for some background
  mesh.setRoot(true);

#else
  mesh.setRoot(false);
#endif
  // This node and all other nodes should ideally know the mesh contains a root, so call this on all nodes
  mesh.setContainsRoot(true);
  //   prowlsent = false;
  Serial.print("getStationIP: ");
  Serial.print(mesh.getStationIP());
  Serial.print("\tMesh APIP: ");
  Serial.println(mesh.getAPIP());
  initialiseMessage();
}

/*******************************
 * Loop
 *******************************/
void vanMeshLoop()
{

  mesh.update();

  //   if (!prowlsent) sendProwl();

  // if (myIP != getlocalIP())
  // {
  //   myIP = getlocalIP();
  //   Serial.println("My IP is " + myIP.toString());
  // }
}
/*******************************
 * Utility Functions
 *******************************/
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
    
    mesh.sendBroadcast(msg);

    printNodeArray();
    taskSendMessage.setInterval(random(TASK_SECOND * 1, TASK_SECOND * 5));
  }
}

// Needed for painless library
void receivedCallback(uint32_t from, String &msg)
{
  // Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
#ifdef IOTCLOUD
  Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
#else
  // Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
#endif
  if (initialised)
  // Serial.println(from);
    storeInNodeArray(from, msg);
}

void newConnectionCallback(uint32_t nodeId)
{
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback()
{
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset)
{
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

// IPAddress getlocalIP()
// {
//   return IPAddress(mesh.getStationIP());
// }

// void sendProwl()
// {
//   // Wait for WiFi connection
//   if (WiFi.status() == WL_CONNECTED)
//   {

//   EspProwl.begin();
//   EspProwl.setApiKey(espProwlKey);
//   EspProwl.setApplicationName(ESPPROWL_APP_NAME);
//   notification = "Temp02";
//   message = "Initial restart";
//   priority = 0;
//   EspProwl.push(notification, message, priority);
//   prowlsent = true;
//   }

// }

void storeInNodeArray(uint32_t from, String msg)
{
  char *end;
  //DEBUG
  // Serial.print("Recd in StoreInNodeArray: ");
  // Serial.println(from);

  msg += " ID:";
  msg += from;
  std::string str_msg = msg.c_str();

  // const char *tempC = tempToken.c_str();

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
  // TODO check the lengths of the token are correct DONE
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
  strcpy(nodearray[node].time, token.c_str());     // store the node time as string

    nodearray[node].from = from; // store the nodeid 
  messageStored = true;
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

void printNodeArray()
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
    printStr(nodearray[node].date, 9);
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
}
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
    if (nodearray[i].status >= 0)
    {
      bumpcount += nodearray[i].lastcall;
    }
    if (bumpcount >= MAXLASTCALL) // looks like we are not listening
    {
       system_restart(); //ESP8266
      // esp_restart(); // ESP32
    }
  }
  return true;
}

// to avoid conflicts and racing issues initialise the store with a valid message
void initialiseMessage()
{
  for (int i = 0; i < NODE_COUNT; i++)
  {

    String msg = " N:";
    msg += namearray[i];
    msg += " D1:1.23 D2:5.67 D3:8.90 L:0 S:-1 Date:01/12/2000  Time:23:59:50 ID:0000000";
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
