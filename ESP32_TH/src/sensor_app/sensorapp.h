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

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */

class SensorApp
{
  public:
     // Constructor
     SensorApp(const int DHTPin, DHTesp::DHT_MODEL_t);

     // Functions that do the work
     void takeReading(int DHTPin, DHTesp::DHT_MODEL_t);
     void doWiFi(void);
     void goToSleep(void);
     void coldWake(void);

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
     
     int _DHTPin;
     DHTesp::DHT_MODEL_t _DHT_Type;
     /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     There are 2 buttons on the board, one is a RESET button, the other is connected
     to pin 0 - we can use this to trigger the sensor to start the WiFI link
     +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
     gpio_num_t  GPIO_INPUT_IO_TRIGGER = (gpio_num_t) 0;  
};

#endif
