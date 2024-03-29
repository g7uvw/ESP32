#include "sensorapp.h"
//#include "sensorRTC.h"
#include <rom/rtc.h>
#include "RTClib.h"

RTC_DATA_ATTR unsigned int _interval;
RTC_DATA_ATTR int _caloffset;
RTC_DATA_ATTR time_t _time;
RTC_DATA_ATTR String _sensorID;

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
   pinMode(LED_BUILTIN, OUTPUT);
  _DHTPin = DHTPin;
  _DHT_Type = DHT_Type;
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
    // If we got here by pushing the button then we hit case 1 or case 2
    case 1  :  
    case 2  : 
    digitalWrite(LED_BUILTIN, HIGH);
      doWiFi();
      //goToSleep();
      break;
    // If we got here because the sensor woke up from sleep to take a reading
    // then case 3 occurs and we take a reading. We'll also do the same for
    // cases 4 and 5, because we do not actually use them, and they shouldn't
    // occur, but if they do we'll just take another reading then sleep.
    case 3  : Serial.println("Wakeup to take a reading");
    case 4  :
    case 5  :
      digitalWrite(LED_BUILTIN, HIGH);
      takeReading(_DHTPin,_DHT_Type); 
      digitalWrite(LED_BUILTIN, LOW);
      goToSleep();
      break;
    // If we arrive here, then who knows how we got here, we could find out
    // but we don't really care. It was probably power-on or something. We'll
    // just go back to sleep and wait for a real reason to wake up.
    default : 
      coldWake();
      goToSleep();
      break;
  }

}

void SensorApp::goToSleep(void)
{
    digitalWrite(LED_BUILTIN, LOW);
    pinMode(LED_BUILTIN, INPUT);
    WiFi.mode(WIFI_OFF);
    btStop();
    Serial.println("Going to sleep for " + String(_interval) + " Seconds");
    esp_sleep_enable_timer_wakeup(_interval * uS_TO_S_FACTOR);
    gpio_pulldown_dis(GPIO_INPUT_IO_TRIGGER);       // not use pulldown on GPIO
    esp_sleep_enable_ext0_wakeup(GPIO_INPUT_IO_TRIGGER, 0); // Wake if GPIO is low
    esp_deep_sleep_start();
}

void SensorApp::coldWake(void)
{
  Serial.println("Wakeup was not caused by deep sleep"); 
      // Init the filesystem
      SPIFFS.begin();
      File file = SPIFFS.open("/config.txt");
      if(!file || file.isDirectory())
      {
        Serial.println("Config.txt error using defaults");
      // file not found, so use some defaults
       _sensorID = "Sensor";
       _interval = 300;
       _caloffset = 0;
      }

    while(file.available())
      {
      _sensorID = file.readStringUntil('\n');
      Serial.print("Sensor ID : ");
      Serial.println(_sensorID);
      _interval = file.readStringUntil('\n').toInt();
      Serial.print("Interval : ");
      Serial.println(_interval);
      _caloffset = file.readStringUntil('\n').toInt();
      Serial.print("Cal Offset : ");
      Serial.println(_caloffset);
      }
    file.close();
}

void SensorApp::takeReading(int _DHTPin, DHTesp::DHT_MODEL_t _DHTTYPE)
{
  char timebuff[25];
  // Init RTC
  // Power on RTC - put pin 14 high
  pinMode(14, OUTPUT);
  digitalWrite(14, HIGH);
  RTC_DS3231 rtc;
  rtc.begin();
    if (rtc.lostPower()) 
    {
    // Set an obviously wrong date
    rtc.adjust(DateTime(2000,4,1,16,38,04));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    }

  // open datafile
  SPIFFS.begin();
  File file = SPIFFS.open("/data.csv",FILE_APPEND);

  // Initialize DHT sensor
  DHTesp dht;
  dht.setup(_DHTPin, _DHTTYPE);
  TempAndHumidity TH;
  TH = dht.getTempAndHumidity();
  
   //uint8_t failcount = 0;
  //while (dht.getStatus() != 0)
  //{
  //  failcount++;
  //  TH = dht.getTempAndHumidity();
  //  if (failcount > 5)
  //  {

  //    break;
  //  }
 // }
    DateTime _now = rtc.now();
    sprintf(timebuff, "%02d-%02d-%02d %02d:%02d:%02d", _now.year(), _now.month(), _now.day(), _now.hour(), _now.minute(), _now.second());
    String datapoint = String(timebuff) + "," + String(TH.temperature) + "," + String(TH.humidity);
    Serial.println(datapoint);
    file.println(datapoint);
    file.close();
    digitalWrite(14, LOW);
    pinMode(14, INPUT_PULLDOWN);
    
}

void SensorApp::doWiFi()
{
Serial.println("Wakeup caused by button, starting portal"); 
      // Serve the Portal page
       WiFiStuff portal;
      //if (!portal.startPortal("Frogs LEGS!!")) 
      const char *c = _sensorID.c_str();
      if (!portal.startPortal(c))
      {
         Serial.println("Can't init portal");
      }
}



