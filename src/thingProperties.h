
#ifdef IOTCLOUD
#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>
#endif

#ifndef SECRETS
#include <secrets.h>
#define SECRETS
#endif


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
char *ESPPROWL_APP_NAME = "Relay Alerts";
#ifdef ESP32
char *device = "Temperature02";
#else
char *device = "Temperature05";
#endif

const int MAXLASTCALL = 60;







