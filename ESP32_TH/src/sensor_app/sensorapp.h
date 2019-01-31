#ifndef SensorApp_h
#define SensorApp_h
#include <time.h>
#include "Arduino.h"
#include "SPIFFS.h"
#include "FS.h"
#include <DHTesp.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h> //use the new ESP32 port of the webserver
#include "SPIFFS.h"
#include "wifi_stuff/WiFi_Stuff.h"

class SensorApp
{
  public:
     // Constructor
     SensorApp();

     // Functions that do the work
     void takeReading(void);
     void doWiFi(void);
     void goToSleep(void);

     // Utility functions
     void   setSensorID(String ID);
     String getSensorID(void);

     void   setSensorCalOffset(int);
     int    getSensorCalOffset(void);

     void   setSensorInterval(unsigned int interval);
     unsigned int getSensorInterval();

     void   setSensorDateTime(String dt);
     void   saveSettings(void);

private:
     String _sensorID;
     unsigned int _interval = 300;
     int _caloffset;
     time_t _time;

};


#endif
