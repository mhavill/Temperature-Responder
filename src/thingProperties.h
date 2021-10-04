#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

// const char THING_ID[] = "0fb1770a-3dc9-4de9-a56a-b2ce0aa59a80";
// const char DEVICE_LOGIN_NAME[] = "779c67f6-204e-48e1-a3f4-45866fda3b2c";
// const char *device = "Temperature01";
// float temp01;

const char THING_ID[] = "a2956bd6-04df-489a-a2ec-88df4f9b73cc";
const char DEVICE_LOGIN_NAME[] = "cbc27fce-ca2f-49c7-a14b-9ad0ceed3175";
const char *device = "Temperature02";
float temp02;

// const char THING_ID[] = "491ac29d-546c-4ee0-b643-fdc1eebe36f0";
// const char DEVICE_LOGIN_NAME[] = "e56cabf4-331b-4870-957a-13c770d2ee1e";
// const char *device = "Temperature03";
// float temp03;

// const char THING_ID[] = "0906ceb0-7557-4401-ae7f-5e5adf2293e9";
// const char DEVICE_LOGIN_NAME[] = "7fff1822-9568-4a4f-8297-9b39a405cd58";
// const char *device = "Temperature04";
// float temp04;

// const char THING_ID[] = "74c7cab9-93b0-4c2c-8e27-2bb5a69c2505";
// const char DEVICE_LOGIN_NAME[] = "179f990f-e64c-4903-af50-b9f0a5430474";
// const char *device = "Temperature05";
// float temp05;

void setTemp(float temp)
{
    // Set IOT Cloud variable
    // temp01 = temp;
    temp02 = temp;
    // temp03 = temp;
    // temp04 = temp;
    //  temp05 = temp;
}

void initProperties()
{

    ArduinoCloud.setBoardId(DEVICE_LOGIN_NAME);
    ArduinoCloud.setSecretDeviceKey(DEVICE_KEY);
    ArduinoCloud.setThingId(THING_ID);
    // ArduinoCloud.addProperty(temp01, READ, 10 * SECONDS, NULL);
    ArduinoCloud.addProperty(temp02, READ, 10 * SECONDS, NULL);
    // ArduinoCloud.addProperty(temp03, READ, 10 * SECONDS, NULL);
    // ArduinoCloud.addProperty(temp04, READ, 10 * SECONDS, NULL);
    //  ArduinoCloud.addProperty(temp05, READ, 10 * SECONDS, NULL);
}

WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);