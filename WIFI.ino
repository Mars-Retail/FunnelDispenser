/***********************************************************************************************
 **@file                 : WiFi.ino
 **@description          : This file handles WiFi functionality
 **@copyright            : Copyright 2019 Mars Inc, All Rights Reserved.
 ** Mars Confidential
 ***********************************************************************************************/
/**************************************************************************************************************
 * @Function      : connect
 * @Description   : This function is Used to connect wifi network
 * @Param         : void 
 * @Return        : void
 ***************************************************************************************************************/
void connect()
{
    boolean state = true;
    Serial.println("");
    Serial.println("In Connect ");  
    /* check wifi is connected or not  */
    if (WiFi.status() == WL_CONNECTED)
    {
       ControlLed(2, true);
       delay(500);
       ControlLed(2, false);
       Serial.println("Showing  Connected!!!!!!!!!!!! ");
       return;
    }
    /* Connect to WiFi network    */
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(DeviceConfig.SSIDName);

    WiFi.begin(DeviceConfig.SSIDName, DeviceConfig.SSIDPass);

    while (WiFi.status() != WL_CONNECTED)
    {
        yield();
        ControlLed(greenLed, state);
        if(state == HIGH)
          state = LOW;
        else
          state = HIGH;
          
        delay(500);
        Serial.print(".");
        CheckWIFIReset();
        if (resetDevice)
        {
          resetCount++;
          Serial.println("Will reset!");
          if (resetCount == 5)
          {
            resetDevice = false;
            ESP.reset();
          }
        }  

        WIFIConnectionTryCnt++;

        if(WIFIConnectionTryCnt > 240)
        {
          ESP.restart();
        }
    }

    while(digitalRead(resetWIFISwitch) == LOW)
    {
      CheckWIFIReset();
    }
      
    ControlLed(greenLed, LOW);
    Serial.println("");
    Serial.println("WiFi connected");
     ControlLed(blueLed, true);
     delay(500);
     ControlLed(blueLed, false);
}
/**************************************************************************************************************
 * @Function      : InitAccessPoint
 * @Description   : This function is Used to initialize Access point
 * @Param         : void 
 * @Return        : void
 ***************************************************************************************************************/
void InitAccessPoint(void)
{  
  char ssid[MAX_DEVICE_ID_SIZE];
   
  Serial.println();
  Serial.println("Configuring access point...");
 
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));                /* subnet FF FF FF 00  */ 

  if(strlen(DeviceConfig.ConnectionString) != 0)
  {
      if (GetDeviceId(DeviceConfig.ConnectionString, ssid, MAX_DEVICE_ID_SIZE) > 0)
      {
        strcat(ssid, "_AP");
        Serial.println(ssid); 
        WiFi.softAP(ssid, password);         
      }
  } 
  else
  {
    /* You can remove the password parameter if you want the AP to be open. */
    WiFi.softAP(APssid, password);
  }
  
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
}
