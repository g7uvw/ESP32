#include <Arduino.h>
#include "sensor_app/sensorapp.h"
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  5        /* Time ESP32 will go to sleep (in seconds) */

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Here we do all the setup stuff, this is where we define what type of sensors we
are using, where they are connected and what the buttons on the board will do.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
There are 2 buttons on the board, one is a RESET button, the other is connected
to pin 0 - we can use this to trigger the sensor to start the WiFI link
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
gpio_num_t  GPIO_INPUT_IO_TRIGGER = (gpio_num_t) 0;  

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
There are two common types of temp and humidity sensor, a Blue one, and a Cream
one. The Blue one - DHT11 is slightly cheaper, but not as accurate. The Cream
one is more expensive, but also more accurate. You must tell the code which you
are using.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
 
//#define DHTTYPE DHTesp::DHT11  // If your sensor is BLUE uncomment this line
#define DHTTYPE DHTesp::DHT22   // If your sensor is CREAM uncomment this line

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
The sensor has three wires, two go to the power supply, the third carries the
data. This has to go to a named pin on the sensor board. We can use almost any
pin we like. For simplicity I chose pin 25.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
const int DHTPin = 25;


/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Here is where all the code goes. 
First thing we do is enable some debugging on the serial port - this lets us 
know the sensor is alive. We'll disable this later - it's not needed when the 
sensor is running not connected to a computer.

We then wait two seconds.

Then we start up the sensor application.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void setup() 
{
  Serial.begin(115200);
  delay(2000);
  SensorApp sensor(DHTPin, DHTTYPE);

  /*
    First we configure the wake up source
    We set our ESP32 to wake up every 5 seconds
  */
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
                 " Seconds");

  gpio_pullup_en(GPIO_INPUT_IO_TRIGGER);        // use pullup on GPIO
  gpio_pulldown_dis(GPIO_INPUT_IO_TRIGGER);       // not use pulldown on GPIO
  esp_sleep_enable_ext0_wakeup(GPIO_INPUT_IO_TRIGGER, 0); // Wake if GPIO is low
  Serial.println("Going to sleep now");
  esp_deep_sleep_start();
}


/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
We should never reach the code here. It has to be defined to keep the software
happy, but we never use it.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void loop() 
{
  // Nothing should ever reach here
}