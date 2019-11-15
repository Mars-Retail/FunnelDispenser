/***********************************************************************************************
 **@file                 : Config.ino
 **@description          : This file handles save and load configuration functionality
 **@copyright            : Copyright 2019 Mars Inc, All Rights Reserved.
 ** Mars Confidential
 ***********************************************************************************************/
/*****************************************************************************************************
 * @Function      : loadConfig
 * @Description   : This function loads configuration from file system  to DeviceConfig structure
 * @Param         : void
 * @Return        : true : after successful load   
 *                  false :  failed load
 ******************************************************************************************************/
bool loadConfig() 
{
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) 
  {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) 
  {
    Serial.println("Config file size is too large");
    return false;
  }

  /* Allocate a buffer to store contents of the file. */
  std::unique_ptr<char[]> buf(new char[size]);

  /* We don't use String here because ArduinoJson library requires the input
     buffer to be mutable. If you don't use ArduinoJson, you may as well
     use configFile.readString instead.   */
  configFile.readBytes(buf.get(), size);

  StaticJsonBuffer<400> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());

  if (!json.success())
  {
    Serial.println("Failed to parse config file");
    return false;
  }

  strcpy(DeviceConfig.ConnectionString, json["CS"]);
  strcpy(DeviceConfig.SSIDName, json["SN"]);
  strcpy(DeviceConfig.SSIDPass, json["SP"]);
  DeviceConfig.DeepSleepInt = (json["DSI"]);
  DeviceConfig.TempPostInt = (json["TPI"]);
  DeviceConfig.DistancePostInt = (json["DPI"]);
  DeviceConfig.BatteryStateInt = (json["BSI"]); 
  DeviceConfig.AllStateInt = (json["ASI"]);
  DeviceConfig.PostAllInfo = json["PAI"];

  return true;
}
/*****************************************************************************************************
 * @Function      : saveConfig
 * @Description   : This function save configuration from  DeviceConfig structure to config.json
 * @Param         : void
 * @Return        : true : after successful save
 *                  false :  failed to save
 ******************************************************************************************************/
bool saveConfig()
{
  StaticJsonBuffer<400> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  json["CS"] = DeviceConfig.ConnectionString;
  json["SN"] = DeviceConfig.SSIDName;
  json["SP"] = DeviceConfig.SSIDPass;
  json["DSI"] = DeviceConfig.DeepSleepInt;
  json["TPI"] = DeviceConfig.TempPostInt;
  json["DPI"] = DeviceConfig.DistancePostInt;
  json["BSI"] = DeviceConfig.BatteryStateInt;
  json["ASI"] = DeviceConfig.AllStateInt;
  json["PAI"] = DeviceConfig.PostAllInfo;
  
  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) 
  {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  json.printTo(configFile);
  return true;
}
/*****************************************************************************************************
 * @Function      : printFile
 * @Description   : This function Prints the content of a file to the Serial
 * @Param         : const char *filename 
 * @Return        : void
 ******************************************************************************************************/  
void printFile(const char *filename) 
{  
   /* Open file for reading */
  File file = SPIFFS.open(filename, "r");         
  if (!file)
  {
    Serial.println(("Failed to read file"));
    return;
  }

  /* Extract each characters one by one   */
  while (file.available())
  {
    Serial.print((char)file.read());
  }
  Serial.println();

  /* Close the file (File's destructor doesn't close the file)  */
  file.close();
}
/*****************************************************************************************************
 * @Function      : printMemory
 * @Description   : This function print the data from RTC Buffer to Serial Monitor
 * @Param         : void
 * @Return        : void
 ******************************************************************************************************/
void printMemory() 
{
  char buf[3];
  uint8_t *ptr = (uint8_t *)&rtcData;
  for (size_t i = 0; i < sizeof(rtcData); i++) 
  {
    sprintf(buf, "%02X", ptr[i]);
    Serial.print(buf);
    if ((i + 1) % 32 == 0)
    {
      Serial.println();
    } 
    else 
    {
      Serial.print(" ");
    }
  }
  Serial.println();
}

