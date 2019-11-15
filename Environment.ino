/***********************************************************************************************
 **@file                 : Environment.ino
 **@description          : This file handles Environment sensor functionality
 **@copyright            : Copyright 2019 Mars Inc, All Rights Reserved.
 ** Mars Confidential
 ***********************************************************************************************/
/**************************************************************************************************************
 * @Function      : InitEnvironment
 * @Description   : This function is Used to establish a communication with Environment sensor through I2C bus
 * @Param         : void
 * @Return        : true : after successful communication   
 *                  false :  unsuccessful communication
 ***************************************************************************************************************/ 
bool InitEnvironment(void)
{
    mySensor.settings.commInterface = I2C_MODE;
    mySensor.settings.I2CAddress = 0x77;
                    
    /*runMode can be:
      0, Sleep mode
      1 or 2, Forced mode
      3, Normal mode   */   
     mySensor.setMode(MODE_SLEEP);                               

    /*Standby can be:
      0, 0.5ms
      1, 62.5ms
      2, 125ms
      3, 250ms
      4, 500ms
      5, 1000ms
      6, 10ms
      7, 20ms  */     
    mySensor.settings.tStandby = 0;

    /*filter can be off or number of FIR coefficients to use:
      0, filter off
      1, coefficients = 2
      2, coefficients = 4
      3, coefficients = 8
      4, coefficients = 16 */      
    mySensor.settings.filter = 0;

    /*tempOverSample can be:
      0, skipped
      1 through 5, oversampling *1, *2, *4, *8, *16 respectively  */
    mySensor.settings.tempOverSample = 1;

    /*pressOverSample can be:
      0, skipped
      1 through 5, oversampling *1, *2, *4, *8, *16 respectively*/
    mySensor.settings.pressOverSample = 1;

    /*humidOverSample can be:
      0, skipped
      1 through 5, oversampling *1, *2, *4, *8, *16 respectively */
    mySensor.settings.humidOverSample = 1;
    /*Make sure sensor had enough time to turn on. BME280 requires 2ms to start up */     
    delay(10);                                           

    Serial.println("Starting BME280...");
   
    if(mySensor.begin() == 0x60)
    {
        return true;
    }
    else
    {
        return false;
    }
}
/**************************************************************************************************************
 * @Function      : SetEnvironment
 * @Description   : This function is Used to take reading from sendor
 * @Param         : void
 * @Return        : void
 ***************************************************************************************************************/
void SetEnvironment()
{
  if(!environmentSensor)
  {
    return;
  }
    
  unsigned long StartMillis = millis();
  /* Wake up sensor and take reading */
  mySensor.setMode(MODE_FORCED);                       

  long startTime = millis();
  /*Wait for sensor to start measurment */
  while(mySensor.isMeasuring() == false)                
  {
    if((millis() - StartMillis) > 2000)
    {
      break;
    }
  }
  
  StartMillis = millis();
   /*Hang out while sensor completes the reading */
  while(mySensor.isMeasuring() == true)                
  {
      if((millis() - StartMillis) > 2000)
      {
        break;    
      }
  }
     
  long endTime = millis();

  /*Sensor is now back asleep but we get the data   */
  DeviceData.Temperature = mySensor.readTempF();
  DeviceData.Humidity = mySensor.readFloatHumidity();   
}
/*****************************************************************************************************
 * @Function      : PrintEnvironment
 * @Description   :  This function print  Environment sensor data on serial
 * @Param         : void
 * @Return        : void
 ******************************************************************************************************/
void PrintEnvironment()
{
  if(environmentSensor)
  {    
    Serial.print("Humidity: ");
    Serial.print(mySensor.readFloatHumidity(), 0);
    Serial.println();
    
    Serial.print("Pressure: ");
    Serial.print(mySensor.readFloatPressure(), 0);
    Serial.println();
    
    Serial.print("Temp: ");
    Serial.print(mySensor.readTempF(), 2);
    Serial.println();
  } 
  else 
  {
    Serial.print("Environment Sensor Failed Init!");
  }
/* Enter in sleep Mode */
  mySensor.setMode(MODE_SLEEP);                                          
   
}
