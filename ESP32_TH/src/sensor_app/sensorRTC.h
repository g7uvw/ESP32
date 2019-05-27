#ifndef SensorRTC_h
#define SensorRTC_h
#include <rom/rtc.h>

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
This is where we define some persistant storage to survive across wakeups of
the sensor. This save reading the storage space every time, should be a bit
faster and save some power too.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

RTC_DATA_ATTR unsigned int _interval = 300;
RTC_DATA_ATTR int _caloffset;
RTC_DATA_ATTR time_t _time;
RTC_DATA_ATTR String _sensorID;

#endif