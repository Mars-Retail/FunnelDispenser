/***********************************************************************************************
 **@file                 : WiFiReset.ino
 **@description          : This file handles WiFi reset functionality
 **@copyright            : Copyright 2019 Mars Inc, All Rights Reserved.
 ** Mars Confidential
 ***********************************************************************************************/
/**************************************************************************************************************
 * @Function      : WIFIResetInit
 * @Description   : This function is Used to set reset switch in INPUT_PULLIUP 
 * @Param         : void 
 * @Return        : bool
 ***************************************************************************************************************/
boolean WIFIResetInit(void)
{
  pinMode(resetWIFISwitch, INPUT_PULLUP);
  return true;
}
/**************************************************************************************************************
 * @Function      : CheckWIFIReset
 * @Description   : This function is Used to check reset switch 
 * @Param         : void 
 * @Return        : void
 ***************************************************************************************************************/
void CheckWIFIReset(void)
{
  if (digitalRead(resetWIFISwitch) == LOW)
  {
    WIFIResetCount++;
   
    ControlLed(blueLed, false);
    ControlLed(greenLed, false);
    ControlLed(redLed, true);
    messagePending = true;
  } 
  else 
  {
    WIFIResetCount = 0;
    ControlLed(redLed, false);
  }

  if (WIFIResetCount > 4)
  {
    ControlLed(blueLed, false);
    ControlLed(greenLed, false);
    ControlLed(redLed, true);
    (void)printf("Clearing WIFI:\r\n");
    strcpy(DeviceConfig.SSIDName, "");
    strcpy(DeviceConfig.SSIDPass, "");
    saveConfig();
    (void)printf("WIFI info cleared:\r\n");
    delay(3000);
    resetDevice = true;
    WIFIResetCount = 0;
  }
}
