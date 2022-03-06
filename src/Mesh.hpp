
#ifdef SIMPLE
//************************************************************
// this is a simple example that uses the painlessMesh library
//
// 1. sends a silly message to every node on the mesh at a random time between 1 and 5 seconds
// 2. prints anything it receives to Serial.print
//
//
//************************************************************
#include "painlessMesh.h"
// #include <painlessMesh.h>
#ifndef SECRETS
#include <secrets.h>
#define SECRETS
#endif
// #ifndef TIMER
// #define TIMER
// #include <arduino-timer.h>
// #endif

// #define   MESH_PREFIX     "whateverYouLike"
// #define   MESH_PASSWORD   "somethingSneaky"
// #define   MESH_PORT       5555

Scheduler userScheduler; // to control your personal task
painlessMesh mesh;


#define STATION_PORT 5555
uint8_t station_ip[4] = {0, 0, 0, 0}; // IP of the server


struct nodedata
{
  int nodeid;
  float temp;
  int lastcall;
  int status;
};

nodedata nodearray[5];

float tempC;
int broadcastCount;
uint32_t freeMem;

// User stub
void sendMessage(); // Prototype so PlatformIO doesn't complain
void storeInNodeArray(String msg);
void printNodeArray();

Task taskSendMessage(TASK_SECOND * 1, TASK_FOREVER, &sendMessage);

void sendMessage()
{
  #ifdef ESP32
    freeMem = ESP.getFreeHeap();
#else
    freeMem = system_get_free_heap_size();
#endif
  String msg = " N:";
  msg += device;
  msg += " C:";
  msg += tempC;
  msg += " M:";
  msg += freeMem;
  msg += "\n";
  // mesh.sendBroadcast(msg);
  bool broadcast = mesh.sendBroadcast(msg);
    if (broadcast)
    {
      Serial.println("Message sent ok");
    }
    else
    {
      Serial.println("Broadcast not sent!!!!");
    }
    storeInNodeArray(msg);
    printNodeArray();
  taskSendMessage.setInterval(random(TASK_SECOND * 1, TASK_SECOND * 5));
}

// Needed for painless library
void receivedCallback(uint32_t from, String &msg)
{
  // Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
#ifdef ESP32
  Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
#else
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
#endif
  storeInNodeArray(msg);
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

void meshSetup()
{
  Serial.begin(115200);

  // mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  //  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  mesh.setDebugMsgTypes(ERROR | MESH_STATUS | SYNC | MSG_TYPES);

  // mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
      // Channel set to 6. Make sure to use the same channel for your mesh and for you other
    // network (STATION_SSID)
    mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 6);
#ifdef ESP32
    // Setup over the air update support
    mesh.initOTAReceive("bridge");

    mesh.stationManual(STATION_SSID, STATION_PASSWORD, STATION_PORT, station_ip);
#else

    mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);

#endif
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

#ifdef ESP32
  // Bridge node, should (in most cases) be a root node. See [the wiki](https://gitlab.com/painlessMesh/painlessMesh/wikis/Possible-challenges-in-mesh-formation) for some background
  mesh.setRoot(true);
#else
  mesh.setRoot(false);
#endif
  // This node and all other nodes should ideally know the mesh contains a root, so call this on all nodes
  mesh.setContainsRoot(true);
  Serial.print("getStationIP: ");
  Serial.print(mesh.getStationIP());
  Serial.print("\tMesh APIP: ");
  Serial.println(mesh.getAPIP());

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
}

void meshLoop()
{
  // it will run the user scheduler as well
  mesh.update();
}

void storeInNodeArray(String msg)
{
  char *end;
  std::string temperature = msg.c_str();

  std::string mDelimiter = "M:";
  std::string memToken = temperature.substr(temperature.find(mDelimiter) + 2); // extract to end
  const char *nodeMem = memToken.c_str();

  std::string cDelimiter = "C:";
  std::string tempToken = temperature.substr(temperature.find(cDelimiter) + 2, temperature.find(mDelimiter) - 4);
  const char *tempC = tempToken.c_str();

  std::string nDelimiter = "N:";
  std::string nodeToken = temperature.substr(temperature.find(nDelimiter) + 2, temperature.find(cDelimiter) - 4);
  int zeroPoint = nodeToken.find("0");
  std::string subNodeToken = nodeToken.substr(zeroPoint);

  int node = atoi(subNodeToken.c_str()) - 1; // Adjusted for array
  nodearray[node].temp = std::strtod(tempC, &end);
  nodearray[node].lastcall = 0;
  nodearray[node].nodeid = node + 1;
  nodearray[node].status = std::strtod(nodeMem, &end);
}
void printNodeArray()
{
  int callTotal = 0;
  Serial.print("\nNode: ");
  Serial.print("\tTemp: ");
  Serial.print("\t\tLastcall: ");
  Serial.print("\tStatus: ");
  Serial.print("\n");
  for (int node = 0; node <= 4; ++node)
  {
    Serial.print(nodearray[node].nodeid);
    Serial.print("\t");
    Serial.print(nodearray[node].temp);
    Serial.print("\t\t");
    Serial.print(nodearray[node].lastcall);
    Serial.print("\t\t");
    Serial.print(nodearray[node].status);
    Serial.print("\n");
    callTotal = callTotal + nodearray[node].lastcall;
  }
  Serial.print("\n\n");

  if (callTotal >= MAXLASTCALL)
  {
    Serial.println("Last Call Count MAXED OUT! ... Restarting ");
    delay(5000);
    // ESP.restart();
  }
}
#endif // SIMPLE

#ifdef FINAL
// /*******************************
//  * Header
//  * Name:Mesh.cpp
//  * Purpose: Manage ESP mesh
//  * Created Date: 09/02/2022
//  *******************************/

// /*******************************
//  * Includes
//  *******************************/
#include <painlessMesh.h>
#ifndef SECRETS
#include <secrets.h>
#define SECRETS
#endif
#ifndef TIMER
#define TIMER
#include <arduino-timer.h>
#endif

  /*******************************
   * Protptypes
   *******************************/
  void meshSetup();
  void meshLoop();
  void sendMessage();
  void receivedCallback(uint32_t from, String & msg);
  void newConnectionCallback(uint32_t nodeId);
  void changedConnectionCallback();
  void nodeTimeAdjustedCallback(int32_t offset);
  void delayReceivedCallback(uint32_t from, int32_t delay);
  void storeInNodeArray(String msg);
  void printNodeArray();
  bool bumpLastCall(void *);
  bool sendMessage2(void *);
  bool meshupdate2(void *);
  bool showNodes(void *);
  IPAddress getlocalIP();

// /*******************************
//  * Definitions
//  *******************************/
#define LED 2 // GPIO number of connected LED, ON ESP-12 IS GPIO2

  painlessMesh mesh;
  Scheduler userScheduler;                                           // to control your personal task
  Task taskSendMessage(TASK_SECOND * 1, TASK_FOREVER, &sendMessage); // start with a one second interval
  auto timer_mesh = timer_create_default();                          // create a timer with default settings
  uint32_t freeMem;

#define STATION_PORT 5555
  uint8_t station_ip[4] = {0, 0, 0, 0}; // IP of the server

  bool calc_delay = false;
  SimpleList<uint32_t> nodes;

  struct nodedata
  {
    int nodeid;
    float temp;
    int lastcall;
    int status;
  };

  nodedata nodearray[5];

  float tempC;
  int broadcastCount;

  bool meshConnected = false;

  // /*******************************
  //  * Setup
  //  *******************************/
  void meshSetup()
  {

    pinMode(LED, OUTPUT);

    // mesh.setDebugMsgTypes(ERROR | STARTUP); // set before init() so that you can see startup messages
    // mesh.setDebugMsgTypes(ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE | DEBUG);
    // mesh.setDebugMsgTypes(ERROR | DEBUG | STARTUP); // set before init() so that you can see error messages
    mesh.setDebugMsgTypes(ERROR | MESH_STATUS | SYNC | MSG_TYPES);
    // Channel set to 6. Make sure to use the same channel for your mesh and for you other
    // network (STATION_SSID)
    mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 6);
#ifdef ESP32
    // Setup over the air update support
    mesh.initOTAReceive("bridge");

    mesh.stationManual(STATION_SSID, STATION_PASSWORD, STATION_PORT, station_ip);
#else

    mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);

#endif
    mesh.onReceive(&receivedCallback);
    mesh.onNewConnection(&newConnectionCallback);
    mesh.onChangedConnections(&changedConnectionCallback);
    mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
    mesh.onNodeDelayReceived(&delayReceivedCallback);

    userScheduler.addTask(taskSendMessage);
    taskSendMessage.enable();
#ifdef ESP32
    // Bridge node, should (in most cases) be a root node. See [the wiki](https://gitlab.com/painlessMesh/painlessMesh/wikis/Possible-challenges-in-mesh-formation) for some background
    mesh.setRoot(true);
#else
    mesh.setRoot(false);
#endif
    // This node and all other nodes should ideally know the mesh contains a root, so call this on all nodes
    mesh.setContainsRoot(true);
    Serial.print("getStationIP: ");
    Serial.print(mesh.getStationIP());
    Serial.print("\tMesh APIP: ");
    Serial.println(mesh.getAPIP());

    // meshConnected = true;
    timer_mesh.every(9000, sendMessage2); // Mesh Broadcast
    // timer_mesh.every(5000, meshupdate2);
    timer_mesh.every(5000, showNodes);
    timer_mesh.every(10000, bumpLastCall); // A mesh function
  }
  /*******************************
   * Loop
   *******************************/
  // bool meshupdate2(void *)
  // {
  //   Serial.println("updating mesh");

  //   mesh.update();
  //   Serial.print("getStationIP: ");
  //   Serial.print(mesh.getStationIP());
  //   Serial.print("\tMesh APIP: ");
  //   Serial.println(mesh.getAPIP());
  //   // sendMessage();
  //   // changedConnectionCallback();
  //   // printNodeArray();
  //   return true;
  // }
  void meshLoop()
  {
    mesh.update();
    // sendMessage();
    // changedConnectionCallback();
    // printNodeArray();
  }
  /*******************************
   * Utility Functions
   *******************************/
  void receivedCallback(uint32_t from, String & msg)
  {
#ifdef ESP32
    Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
#else
    Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
#endif
    storeInNodeArray(msg);
  }

  void storeInNodeArray(String msg)
  {
    char *end;
    std::string temperature = msg.c_str();

    std::string mDelimiter = "M:";
    std::string memToken = temperature.substr(temperature.find(mDelimiter) + 2); // extract to end
    const char *nodeMem = memToken.c_str();

    std::string cDelimiter = "C:";
    std::string tempToken = temperature.substr(temperature.find(cDelimiter) + 2, temperature.find(mDelimiter) - 4);
    const char *tempC = tempToken.c_str();

    std::string nDelimiter = "N:";
    std::string nodeToken = temperature.substr(temperature.find(nDelimiter) + 2, temperature.find(cDelimiter) - 4);
    int zeroPoint = nodeToken.find("0");
    std::string subNodeToken = nodeToken.substr(zeroPoint);

    int node = atoi(subNodeToken.c_str()) - 1; // Adjusted for array
    nodearray[node].temp = std::strtod(tempC, &end);
    nodearray[node].lastcall = 0;
    nodearray[node].nodeid = node + 1;
    nodearray[node].status = std::strtod(nodeMem, &end);
  }

  void newConnectionCallback(uint32_t nodeId)
  {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
    Serial.printf("--> startHere: New Connection, %s\n", mesh.subConnectionJson(true).c_str());
  }

  bool showNodes(void *)
  {
    // changedConnectionCallback();
    return true;
  }
  void changedConnectionCallback()
  {
    Serial.printf("Changed connections\n");

    nodes = mesh.getNodeList();

    Serial.printf("Num nodes: %d\n", nodes.size());
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
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
  }

  void delayReceivedCallback(uint32_t from, int32_t delay)
  {
    Serial.printf("Delay to node %u is %d us\n", from, delay);
  }
  void printNodeArray()
  {
    int callTotal = 0;
    Serial.print("\nNode: ");
    Serial.print("\tTemp: ");
    Serial.print("\t\tLastcall: ");
    Serial.print("\tStatus: ");
    Serial.print("\n");
    for (int node = 0; node <= 4; ++node)
    {
      Serial.print(nodearray[node].nodeid);
      Serial.print("\t");
      Serial.print(nodearray[node].temp);
      Serial.print("\t\t");
      Serial.print(nodearray[node].lastcall);
      Serial.print("\t\t");
      Serial.print(nodearray[node].status);
      Serial.print("\n");
      callTotal = callTotal + nodearray[node].lastcall;
    }
    Serial.print("\n\n");

    if (callTotal >= MAXLASTCALL)
    {
      Serial.println("Last Call Count MAXED OUT! ... Restarting ");
      delay(5000);
      // ESP.restart();
    }
  }

  bool sendMessage2(void *)
  {
    Serial.println("Sending Message ");
    sendMessage();
    return true;
  }

  bool bumpLastCall(void *)
  {
    Serial.println("Bumping ");
    for (int i = 0; i <= 4; ++i)
    {
      nodearray[i].lastcall++;
    }
    count = ++broadcastCount;
    temp01 = nodearray[0].temp;
    temp02 = nodearray[1].temp;
    temp03 = nodearray[2].temp;
    temp04 = nodearray[3].temp;
    temp05 = nodearray[4].temp;

    return true;
  }
  IPAddress getlocalIP()
  {
    return IPAddress(mesh.getStationIP());
  }

  void sendMessage()
  {

#ifdef ESP32
    freeMem = ESP.getFreeHeap();
#else
    freeMem = system_get_free_heap_size();
#endif

    String msg = " N:";
    msg += device;
    msg += " C:";
    msg += tempC;
    msg += " M:";
    msg += freeMem;
    msg += "\n";
    bool broadcast = mesh.sendBroadcast(msg);
    if (broadcast)
    {
      Serial.println("Message sent ok");
    }
    else
    {
      Serial.println("Broadcast not sent!!!!");
    }
    storeInNodeArray(msg);
    printNodeArray();

    if (calc_delay)
    {
      SimpleList<uint32_t>::iterator node = nodes.begin();
      while (node != nodes.end())
      {
        mesh.startDelayMeas(*node);
        node++;
      }
      calc_delay = false;
    }

    // Serial.printf("Sending message: %s\n", msg.c_str());

    taskSendMessage.setInterval(random(TASK_SECOND * 1, TASK_SECOND * 5)); // between 1 and 5 seconds
  }

// /*******************************
//  * Finite State Machine
//  *******************************/
#endif // FINAL
