/*******************************
 * Header
 * Name: PushNotification.cpp
 * Purpose: To manage the connection and issue of push notifications
 * Created Date: 09/02/2022
*******************************/


/*******************************
 * Includes
*******************************/
#include <EspProwl.h>
#ifndef THINGPROPERTIES
#include <thingProperties.h>
#define THINGPROPERTIES
#endif
#ifndef SECRETS
#include <secrets.h>
#endif


/*******************************
 * Protptypes
*******************************/
void prowlSetup();
void sendProwl();
/*******************************
 * Definitions
*******************************/
bool prowlsent;
char *notification;
char *message;
int priority;
int returnCode;
/*******************************
 * Setup
*******************************/
void prowlSetup()
{
    prowlsent = false;
}

/*******************************
 * Loop
*******************************/

/*******************************
 * Utility Functions
*******************************/
void sendProwl()
{
  // Wait for WiFi connection
  if (WiFi.status() == WL_CONNECTED)
  {

  EspProwl.begin();
  EspProwl.setApiKey(ESPPROWL_KEY);
  EspProwl.setApplicationName(ESPPROWL_APP_NAME);
  notification = device;
  message= "Initial restart";
  priority = 0;
  // EspProwl.push(notification, message, priority);
  prowlsent = true;
  }

}


/*******************************
 * Finite State Machine
*******************************/