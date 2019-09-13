# ESP32
      ______    _____   _____    ____    ___                                                 
     |  ____|  / ____| |  __ \  |___ \  |__ \                                                
     | |__    | (___   | |__) |   __) |    ) |                                               
     |  __|    \___ \  |  ___/   |__ <    / /                                                
     | |____   ____) | | |       ___) |  / /_                                                
     |______| |_____/  |_|      |____/  |____|                   _                           
     |__   __|                                                  | |                          
        | |      ___   _ __ ___    _ __     ___   _ __    __ _  | |_   _   _   _ __    ___   
        | |     / _ \ | '_ ` _ \  | '_ \   / _ \ | '__|  / _` | | __| | | | | | '__|  / _ \  
        | |    |  __/ | | | | | | | |_) | |  __/ | |    | (_| | | |_  | |_| | | |    |  __/  
        |_|     \___| |_| |_| |_| | .__/   \___| |_|     \__,_|  \__|  \__,_| |_|     \___|  
                                  | |                                                        
      _    _                      |_|      _   _   _                                         
     | |  | |                     (_)     | | (_) | |                                        
     | |__| |  _   _   _ __ ___    _    __| |  _  | |_   _   _                               
     |  __  | | | | | | '_ ` _ \  | |  / _` | | | | __| | | | |                              
     | |  | | | |_| | | | | | | | | | | (_| | | | | |_  | |_| |                              
     |_|  |_|  \__,_| |_| |_| |_| |_|  \__,_| |_|  \__|  \__, |                              
                                                          __/ |                              
                                                         |___/     

This is the code for the ESP32 Temperature &amp; Humidity Sensor Project

The code is based on the On Demand Config WiFi portal example, and uses chunks of this to provide configuration and data services to the sensor.

The project is targeted to the DFRobot Firebeetle board as this was found to be the lowest power consumption board when testing several. It should be portable to other ESP32 boards.

The target hardware is an ESP32 Firebeetle, with a DHT22 T&H sensor and a DS3231 RTC.
