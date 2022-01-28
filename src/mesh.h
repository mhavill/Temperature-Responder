/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-mesh-esp32-esp8266-painlessmesh/
  
  This is a simple example that uses the painlessMesh library: https://github.com/gmag11/painlessMesh/blob/master/examples/basic/basic.ino
*/

#include "painlessMesh.h"
#include <iostream>
#include <string>

Scheduler userScheduler; // to control your personal task
painlessMesh mesh;

// User stub
void sendMessage(); // Prototype so PlatformIO doesn't complain
void meshSetup();
void meshLoop();
void receivedCallback(uint32_t from, String &msg);
void newConnectionCallback(uint32_t nodeId);
void nodeTimeAdjustedCallback(int32_t offset);
void storeInNodeArray(String &msg);
void printNodeArray();

Task taskSendMessage(TASK_SECOND * 5, TASK_FOREVER, &sendMessage);
uint32_t freeMem;
void sendMessage()
{
    freeMem = system_get_free_heap_size();
    String msg = " N:";
    msg += device;
    msg += " C:";
    msg += tempC;
    msg += " M:";
    msg += freeMem;
    msg += "\n";
    msg += mesh.getNodeId();
    mesh.sendBroadcast(msg);
    taskSendMessage.setInterval(random(TASK_SECOND * 5, TASK_SECOND * 5));
    storeInNodeArray(msg);
    printNodeArray();
}

// Needed for painless library
void receivedCallback(uint32_t from, String &msg)
{
    storeInNodeArray(msg);

    //   String topic = "painlessMesh/from/" + String(from);
    //   mqttClient.publish(topic.c_str(), msg.c_str());
}

void storeInNodeArray(String &msg)
{
    char *end;
    // Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
    broadcastCount = 0; //got a message from someone so reset counter
    std::string temperature = msg.c_str();

    std::string mDelimiter = "M:";
    std::string memToken = temperature.substr(temperature.find(mDelimiter) + 2); //extract to end
    const char *nodeMem = memToken.c_str();

    std::string cDelimiter = "C:";
    std::string tempToken = temperature.substr(temperature.find(cDelimiter) + 2, temperature.find(mDelimiter) - 4);
    const char *tempC = tempToken.c_str();

    std::string nDelimiter = "N:";
    std::string nodeToken = temperature.substr(temperature.find(nDelimiter) + 2, temperature.find(cDelimiter) - 4);
    int zeroPoint = nodeToken.find("0");
    std::string subNodeToken = nodeToken.substr(zeroPoint);

    int node = atoi(subNodeToken.c_str()) - 1; //Adjusted for array
    nodearray[node].temp = std::strtod(tempC, &end);
    nodearray[node].lastcall = 0;
    nodearray[node].nodeid = node + 1;
    nodearray[node].status = std::strtod(nodeMem, &end);
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

void droppedConnection(uint32_t nodeId)
{
    Serial.print("--> End Here: Oh no! Lost Connection, nodeId = ");
    Serial.println(String(nodeId));
}

void printNodeArray()
{
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
    }
    Serial.print("\n\n");
}

void meshSetup()
{
    //   Serial.begin(115200);

    //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
    mesh.setDebugMsgTypes(ERROR | STARTUP); // set before init() so that you can see startup messages
    // mesh.setDebugMsgTypes(ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE);

    mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, 6);
    mesh.onReceive(&receivedCallback);
    mesh.onNewConnection(&newConnectionCallback);
    mesh.onChangedConnections(&changedConnectionCallback);
    mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
    mesh.onDroppedConnection(&droppedConnection);

    userScheduler.addTask(taskSendMessage);
    taskSendMessage.enable();
// Bridge node, should (in most cases) be a root node. See [the wiki](https://gitlab.com/painlessMesh/painlessMesh/wikis/Possible-challenges-in-mesh-formation) for some background
#ifdef ESP32

    mesh.setRoot(true);
#else
    mesh.setRoot(false);
#endif
    // This node and all other nodes should ideally know the mesh contains a root, so call this on all nodes
    mesh.setContainsRoot(true);
}

void meshLoop()
{
    // it will run the user scheduler as well
    mesh.update();
}
