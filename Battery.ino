/***********************************************************************************************
 **@file                 : Battery.ino
 **@description          : This file handles battery sensor  functionality
 **@copyright            : Copyright 2019 Mars Inc, All Rights Reserved.
 ** Mars Confidential
 ***********************************************************************************************/
/*****************************************************************************************************
 * @Function      :  InitBattery
 * @Description   :  This function is Used to establish a communication with battery sensor through I2C bus
 * @Param         : void
 * @Return        : true : after successful communication   
 *                  false :  unsuccessful communication
 ******************************************************************************************************/
bool InitBattery(void)
{
  /* begin() will return true if communication is successful  */
  if (!lipo.begin())                                                 
  {
    Serial.println("Error: Unable to communicate with BQ27441.");
    Serial.println("  Check wiring and try again.");
    Serial.println("  (Battery must be plugged into Battery Babysitter!)");
    return false;
  }
  Serial.println("Connected to BQ27441!");
/* configure design capacity of battery as (unsigned int) 2650; */ 
  lipo.setCapacity(BATTERY_CAPACITY);                                 
  return true;
}
/*****************************************************************************************************
 * @Function      :  SetBatteryStats
 * @Description   :  This function is reading battery health 
 * @Param         : void
 * @Return        : void
 ******************************************************************************************************/
void SetBatteryStats()
{
  if(!batterySensor)
  {
    return;
  }
    
  /* Read battery stats from the BQ27441-G1A   */
  unsigned int soc = lipo.soc();                 /* Read state-of-charge (%) */
  int health = lipo.soh();                       /* Read state-of-health (%)*/

  DeviceData.BatteryPercentage = soc;
  DeviceData.BatteryState = health;
}
/*****************************************************************************************************
 * @Function      : printBatteryStats
 * @Description   :  This function print battery stats
 * @Param         : void
 * @Return        : void
 ******************************************************************************************************/
void printBatteryStats()
{
  if(batterySensor)
  {
    /* Read battery stats from the BQ27441-G1A   */
    unsigned int soc = lipo.soc();                              /* Read state-of-charge (%)   */
    unsigned int volts = lipo.voltage();                        /* Read battery voltage (mV)   */
    int current = lipo.current(AVG);                            /* Read average current (mA)   */
    unsigned int fullCapacity = lipo.capacity(FULL);            /* Read full capacity (mAh)   */
    unsigned int capacity = lipo.capacity(REMAIN);              /* Read remaining capacity (mAh)  */
    int power = lipo.power();                                   /* Read average power draw (mW)   */
    int health = lipo.soh();                                    /* Read state-of-health (%)     */
  
    /* Now print out those values:       */
    String toPrint = String(soc) + "% | ";
    toPrint += String(volts) + " mV | ";
    toPrint += String(current) + " mA | ";
    toPrint += String(capacity) + " / ";
    toPrint += String(fullCapacity) + " mAh | ";
    toPrint += String(power) + " mW | ";
    toPrint += String(health) + "%";
  
    Serial.println(toPrint);
  } 
  else 
  {
    Serial.println("Battery sensor failed Init!");
  }
}
/*****************************************************************************************************
 * @Function      : I2CWriteBytes
 * @Description   :  This function write on I2C bus
 * @Param         : subAddress,
 *                  * src, 
 *                  count
 * @Return        : uint16_t
 ******************************************************************************************************/
uint16_t I2CWriteBytes(uint8_t subAddress, uint8_t * src, uint8_t count)
{
  /* I2C communication start */
  Wire.beginTransmission(BQ72441_I2C_ADDRESS);
  Wire.write(subAddress);
  for (int i=0; i<count; i++)
  {
    Wire.write(src[i]);
  } 
  Wire.endTransmission(true);
}
/*****************************************************************************************************
 * @Function      : Hibernate
 * @Description   : This function Activated Battery Module in Hibernate Mode 
 * @Param         : void
 * @Return        : void
 ******************************************************************************************************/
void Hibernate()
{
    uint8_t subCommandMSB = (0x0011 >> 8);
    uint8_t subCommandLSB = (0x0011 & 0x00FF);
    uint8_t command[2] = {subCommandLSB, subCommandMSB};
    uint8_t data = 1;

    if(I2CWriteBytes((uint8_t) 0, command, 2))
    {
      if(I2CWriteBytes((uint8_t) 0, &data, 1))
      {
        Serial.println("Battery Module Hibernate Mode Activated"); 
      } 
    }  
}
/*****************************************************************************************************
 * @Function      : Clear_Hibernate
 * @Description   : This function deactivated Battery Module in Hibernate Mode 
 * @Param         : void
 * @Return        : void
 ******************************************************************************************************/
void Clear_Hibernate()
{
  uint8_t subCommandMSB = (0x0012 >> 8);
  uint8_t subCommandLSB = (0x0012 & 0x00FF);
  uint8_t command[2] = {subCommandLSB, subCommandMSB};
  uint8_t data = 0;

  if(I2CWriteBytes((uint8_t) 0, command, 2))
  {
    if(I2CWriteBytes((uint8_t) 0, &data, 1))
    {
      Serial.println("Battery Module Hibernate Mode Cleared"); 
    } 
  }
}

