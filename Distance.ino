/***********************************************************************************************
 **@file                 : Distance.ino
 **@description          : This file handles distance sensor functionality
 **@copyright            : Copyright 2019 Mars Inc, All Rights Reserved.
 ** Mars Confidential
 ***********************************************************************************************/
/*****************************************************************************************************
 * @Function      :  Init6180
 * @Description   :  This function is Used to establish a communication with distance sensor through I2C bus
 * @Param         : void
 * @Return        : true : after successful communication   
 *                  false :  unsuccessful communication
 ******************************************************************************************************/
bool Init6180(void)
{
  vl = Adafruit_VL6180X();
  Serial.println("Init VL6180x");
  if (! vl.begin()) 
  {
    Serial.println("Failed to find sensor");
   return false;
  }
  Serial.println("Sensor found!");
  return true;
}
/*****************************************************************************************************
 * @Function      :  GetDistance
 * @Description   :  This function is reading distance data from sensor
 * @Param         : void
 * @Return        : void
 ******************************************************************************************************/
void GetDistance(void)
{
  uint8_t range = vl.readRange();
  uint8_t status = vl.readRangeStatus();

  DeviceData.Distance = range/10;
  if (status == VL6180X_ERROR_NONE) {
    Serial.print("Range: "); Serial.println(range);
  }

  // Some error occurred, print it out!
  
  if  ((status >= VL6180X_ERROR_SYSERR_1) && (status <= VL6180X_ERROR_SYSERR_5)) {
    Serial.println("System error");
  }
  else if (status == VL6180X_ERROR_ECEFAIL)
  {
    Serial.println("ECE failure");
  }
  else if (status == VL6180X_ERROR_NOCONVERGE) 
  {
    Serial.println("No convergence");
  }
  else if (status == VL6180X_ERROR_RANGEIGNORE) 
  {
    Serial.println("Ignoring range");
  }
  else if (status == VL6180X_ERROR_SNR) 
  {
    Serial.println("Signal/Noise error");
  }
  else if (status == VL6180X_ERROR_RAWUFLOW) 
  {
    Serial.println("Raw reading underflow");
  }
  else if (status == VL6180X_ERROR_RAWOFLOW) 
  {
    Serial.println("Raw reading overflow");
  }
  else if (status == VL6180X_ERROR_RANGEUFLOW) 
  {
    Serial.println("Range reading underflow");
  }
  else if (status == VL6180X_ERROR_RANGEOFLOW) 
  {
    Serial.println("Range reading overflow");
  }
  
}
