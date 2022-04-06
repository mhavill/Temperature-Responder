/*******************************
 * Header
 * Name: thingProperties.h
 * Purpose: Defines the configuration of this thing (from Arduino IoT Cloud)
 * Created Date: 25/03/2022
*******************************/
// TODO remove unused code

/*******************************
 * Includes
*******************************/
#include <cstring>

#ifdef IOTCLOUD
  #include <ArduinoIoTCloud.h>
  #include <Arduino_ConnectionHandler.h>
#endif // IOTCLOUD

#ifndef SECRETS
  #include <secrets.h>
#define SECRETS
#endif


/*******************************
 * Protptypes
*******************************/
void initProperties();

/*******************************
 * Definitions
*******************************/

#ifdef IOTCLOUD
    #ifdef espressif8266
    // const char device[] = "Emcu-02";
    const char DEVICE_LOGIN_NAME[] = "4835c4c5-4264-4206-b779-8468a3c4e5dc";
    #else
    // const char device[] = "Wroom01";
    const char DEVICE_LOGIN_NAME[] = "a08dbba9-73b0-4866-8beb-137975873db9"; //a08dbba9-73b0-4866-8beb-137975873db9
    #endif
    const char THING_ID[] = "4a56f20b-30b8-4923-b5a3-95bf8f62ec69"; //4a56f20b-30b8-4923-b5a3-95bf8f62ec69
#endif

float temp01;
float temp02;
float temp03;
float temp04;
float temp05;
int count;
char ESPPROWL_APP_NAME[] = "Relay Alerts";


const int MAXLASTCALL = 600;

// #ifdef IOTCLOUD // is this a 'bridge' node?
// const char THING_ID[] = "819ce20c-d5cc-487a-bce0-c971cd822ec5";

// // const char DEVICE_LOGIN_NAME[]  = "a08dbba9-73b0-4866-8beb-137975873db9"; //Wroom1
// const char DEVICE_LOGIN_NAME[] = "6b14735d-83b0-495b-9919-107f6c5d90e7"; // Wroom2


// WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);




struct nodedata
{
    int nodeid;
    char name[12];
    double data1;    //e.g. Latitude, temperature
    double data2;    //e.g. Longitude
    double data3;    //e.g. Speed
    int lastcall;
    int status;
    char date[14];
    char time[14];
    
};

const int NODE_COUNT = 7;
nodedata nodearray[NODE_COUNT];

enum Nodes {GPS01, Temp02, Temp03, Temp04, Temp05, Sound06, PIR07};

char namearray[NODE_COUNT][10] = {"GPS01", "Temp02", "Temp03", "Temp04", "Temp05", "Sound06","PIR07"};
const int node = Temp05;

bool initialised = false;

/*******************************
 * Setup
*******************************/
void initProperties()
{
initialised = false;
std::strcpy (namearray[GPS01],"GPS01");
std::strcpy (namearray[Temp02],"Temp02");
std::strcpy (namearray[Temp03],"Temp03");
std::strcpy (namearray[Temp04],"Temp04");
std::strcpy (namearray[Temp05],"Temp05");
std::strcpy (namearray[Sound06],"Sound06");
std::strcpy (namearray[PIR07],"PIR07");
initialised = true;



}



/*******************************
 * Loop
*******************************/

/*******************************
 * Utility Functions
*******************************/

/*******************************
 * Finite State Machine
*******************************/




