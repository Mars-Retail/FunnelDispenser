
//Setup Configuration Guide
/***************************************************
 After downloading from Github verify following
 1. FIle -> Preferences -> Set Sketchbook Location as per downloaded folder path
 2. Restart the IDE 
 3. Verify the Setting in Tools-> Board: Generic ESP8266 Module
                                  Flash Size: 4M(3M SPIFFS)
                                  Debug Port: Serial1                                
***************************************************/

// Version 0.0.1
/***************************************************
1. Added low power mode for battery saving. 
2. After every 10 min, module wake up and check if there is any difference in data then send data to IoT hub.
3. Set threshold for temp,humidity and distance sensor.
4. Heartbeat send after every 4hrs even if there is no difference in data. 
5. RTC read/write implemented for sensor states 
6. Send All the sensors to sleep mode
***************************************************/

