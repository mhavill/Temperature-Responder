#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

const char THING_ID[] = "0906ceb0-7557-4401-ae7f-5e5adf2293e9";
const char DEVICE_LOGIN_NAME[] = "7fff1822-9568-4a4f-8297-9b39a405cd58";

const char *device = "Temperature04";
float temp04;
void setTemp(float temp)
{
    // Sert IOT Cloud variable
    temp04 = temp;
}

void initProperties()
{

    ArduinoCloud.setBoardId(DEVICE_LOGIN_NAME);
    ArduinoCloud.setSecretDeviceKey(DEVICE_KEY);
    ArduinoCloud.setThingId(THING_ID);
    ArduinoCloud.addProperty(temp04, READ, 10 * SECONDS, NULL);
}

WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);