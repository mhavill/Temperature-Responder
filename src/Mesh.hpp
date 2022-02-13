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


/*******************************
 * Protptypes
 *******************************/
void meshSetup();
void meshLoop();
void sendMessage();
void receivedCallback(uint32_t from, String &msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback();
void nodeTimeAdjustedCallback(int32_t offset);
void delayReceivedCallback(uint32_t from, int32_t delay);
void storeInNodeArray(String msg);
void printNodeArray();
bool bumpLastCall(void *);
IPAddress getlocalIP();

// /*******************************
//  * Definitions
//  *******************************/
#define LED 2 // GPIO number of connected LED, ON ESP-12 IS GPIO2

painlessMesh mesh;
Scheduler userScheduler;                                           // to control your personal task
Task taskSendMessage(TASK_SECOND * 1, TASK_FOREVER, &sendMessage); // start with a one second interval
uint32_t freeMem;

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

// /*******************************
//  * Setup
//  *******************************/
void meshSetup()
{
  

  pinMode(LED, OUTPUT);
  
  // mesh.setDebugMsgTypes(ERROR | STARTUP); // set before init() so that you can see startup messages
  mesh.setDebugMsgTypes(ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE | DEBUG);
  // mesh.setDebugMsgTypes(ERROR | DEBUG); // set before init() so that you can see error messages

  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  mesh.onNodeDelayReceived(&delayReceivedCallback);

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
}
/*******************************
 * Loop
 *******************************/
void meshLoop()
{
  mesh.update();
}
/*******************************
 * Utility Functions
 *******************************/
void receivedCallback(uint32_t from, String &msg)
{
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
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

bool bumpLastCall(void *)
{
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
  bool result = mesh.sendBroadcast(msg);
  if (result)
    Serial.println("message sent ok");
  else
    Serial.println("Broadcast not sent!!!!");
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

  Serial.printf("Sending message: %s\n", msg.c_str());

  taskSendMessage.setInterval(random(TASK_SECOND * 1, TASK_SECOND * 5)); // between 1 and 5 seconds
}

// /*******************************
//  * Finite State Machine
//  *******************************/

