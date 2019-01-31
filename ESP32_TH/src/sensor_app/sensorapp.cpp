#include "sensorapp.h"
#include "sensorRTC.h"

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
This is where the magic happens!
We arrive here right after the sensor wakes up. There are three things that can
wake the sensor up:
  1) When power is connected
  2) When the sensor wakes up to take a reading
  3) When the user presses the button to configure the sensor or download data

First thing we do here is to find out which of the three ways got us here, then
we decide what to do about it.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

SensorApp::SensorApp(const int DHTPin, DHTesp::DHT_MODEL_t DHT_Type)
{
  _DHTPin = DHTPin;
  _DHT_Type = DHT_Type;
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
    // If we got here by pushing the button then we hit case 1 or case 2
    case 1  :  
    case 2  : Serial.println("Wakeup caused by button"); 
      // TODO Call the WiFi Stuff
      break;
    // If we got here because the sensor woke up from sleep to take a reading
    // then case 3 occurs and we take a reading. We'll also do the same for
    // cases 4 and 5, because we do not actually use them, and they shouldn't
    // occur, but if they do we'll just take another reading then sleep.
    case 3  : Serial.println("Wakeup to take a reading");
    case 4  :
    case 5  :
      takeReading(_DHTPin,_DHT_Type);
      // TODO take a reading, then sleep again.  
      break;
    // If we arrive here, then who knows how we got here, we could find out
    // but we don't really care. It was probably power-on or something. We'll
    // just go back to sleep and wait for a real reason to wake up.
    default : Serial.println("Wakeup was not caused by deep sleep"); 
      // TODO go to sleep.
      break;
  }


  // Initialize DHT sensor
  //DHTesp dht;
  // Init the filesystem
  SPIFFS.begin();
  File file = SPIFFS.open("/config.txt");
  if(!file || file.isDirectory())
    {
      // file not found, so use some defaults
       _sensorID = "Sensor";
       _interval = 300;
       _caloffset = 0;
    }

  while(file.available())
    {
      _sensorID = file.readStringUntil('\n');
      _interval = file.readStringUntil('\n').toInt();
      _caloffset = file.readStringUntil('\n').toInt();
    }
file.close();
}

void SensorApp::takeReading(int _DHTPin, DHTesp::DHT_MODEL_t _DHTTYPE)
{
  // Initialize DHT sensor
  DHTesp dht;
  dht.setup(_DHTPin, _DHTTYPE);

}



void SensorApp::setSensorID(String ID)
{
  _sensorID = ID;
}

String SensorApp::getSensorID(void)
{
  return _sensorID;
}

void   SensorApp::setSensorCalOffset(int offset)
{
  _caloffset = offset;
}

int    SensorApp::getSensorCalOffset(void)
{
  return _caloffset;
}

void   SensorApp::setSensorInterval(unsigned int interval)
{
  _interval = interval;
}

unsigned int SensorApp::getSensorInterval()
{
  return _interval;
}
void   SensorApp::setSensorDateTime(String dt)
{
  // not implemented yet
}


void   SensorApp::saveSettings()
{
  // create new file
  // write new settings to it
  // if all ok, delete old file and rename new once
  // else flag error and leave old file

  SPIFFS.begin();

  File file = SPIFFS.open("/config.new","w");
  if(!file)


  SPIFFS.remove("/config.txt");


}
