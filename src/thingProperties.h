
#ifdef ESP32
const char *device = "Temperature02";
#else
const char *device = "Temperature01";
#endif

float tempC;

// float temp01; //TODO get these from the array
// float temp02;
// float temp03;
// float temp04;
// float temp05;


int broadcastCount; // Used to count number of intervals when no Callback received and then restart broadcast
// const int COUNT_RESET = 5;

struct nodedata
{
    int nodeid;
    float temp;
    int lastcall;
    int status;
};

nodedata nodearray[5];
