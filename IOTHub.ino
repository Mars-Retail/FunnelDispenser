/***********************************************************************************************
 **@file                 : IoTHub.ino
 **@description          : This file handles IoT Hub functionality
 **@copyright            : Copyright 2019 Mars Inc, All Rights Reserved.
 ** Mars Confidential
 ***********************************************************************************************/
/**********************************************************************************
 Message formats:
 Send Device State to Hub:        {"Name": "Send", "Parameters":"{}"}
 Display/Not Display FreeHeap:    {"Name": "DisplayFreeHeap", "Parameters" : { "Display" : true}}
 Display Battery Information:     {"Name": "DisplayBattery", "Parameters": "{}"}
 Display Environment Values:      {"Name": "DisplayEnvironment", "Parameters": "{}"}
 Turn on LEDs:                    {"Name": "SetLED", "Parameters": { "Color" : "green", "Duration": 4000}}
 Clear WIFI info                  {"Name": "ClearWIFI", "Parameters":"{}"}
 Update Config Interval Values:   {"Name": "ConfigInterval", "Parameters": { "Key" : "AllStateInt", "Mills": 40000}}
 Config Interval Keys : AllStateInt, DeepSleepInt, TempPostInt, DistancePostInt, BatteryStateInt
*******************************************************************************************************/
/**************************************************************************************************************
 * @Function      : InitIOTHub
 * @Description   : This function  Sets up the message callback to be invoked when IoT Hub issues a message to the device
 * @Param         : void
 * @Return        : true : after success   
 *                  false :  unsuccess 
 ***************************************************************************************************************/
bool InitIOTHub(void)
{
  iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(DeviceConfig.ConnectionString, HTTP_Protocol);

  if (iotHubClientHandle == NULL)
  {
    return false;
  }

  if (IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, ReceiveMessageCallback, &receiveContext) != IOTHUB_CLIENT_OK)
  {
    return false;
  }
  
  return true;
}
/**************************************************************************************************************
 * @Function      : ComposeDeviceStateMessage
 * @Description   : This function compose device state message
 * @Param         : char *payload, 
 *                  char *deviceState, 
 *                  int numberOfErrors, 
 *                  char *lastError, 
 *                  char *notes
 * @Return        : void
 ***************************************************************************************************************/
void ComposeDeviceStateMessage(char *payload, char *deviceState, int numberOfErrors, char *lastError, char *notes)
{   
    StaticJsonBuffer<MESSAGE_MAX_LEN> jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();       
    String dateTime = String(year()) + "-" + String(month()) + "-" + String(day()) + " " + String(hour()) + ":" + String(minute()) + ":" + String(second());
    root["MessageType"] = deviceState;
    root["ReadDateTime"] = dateTime;
    root["Distance"] = DeviceData.Distance; 
    root["Temperature"] = DeviceData.Temperature;
    root["Humidity"] = DeviceData.Humidity;
    root["BatteryPercentage"] = DeviceData.BatteryPercentage;
    root["BatteryState"] = DeviceData.BatteryState;
    root["MotionCount"] = DeviceData.MotionCount;
    root["SwitchCount"] = DeviceData.SwitchCount;
    root["NumberOfErrors"] = numberOfErrors;
    root["LastError"] = lastError;
    root["Notes"] = notes;
    
    root.printTo(payload, MESSAGE_MAX_LEN);
}
/**************************************************************************************************************
 * @Function      : QueueMessage
 * @Description   : This function queued the message for delivery
 * @Param         : IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle, 
 *                  char *buffer
 * @Return        : void
 ***************************************************************************************************************/
static void QueueMessage(IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle, char *buffer)
{
    IOTHUB_MESSAGE_HANDLE messageHandle = IoTHubMessage_CreateFromByteArray((const unsigned char *)buffer, strlen(buffer));
    if (messageHandle == NULL)
    {
        Serial.println("Unable to create a new IoTHubMessage.");
    }
    else
    {
        Serial.printf("Queueing message: %s.\r\n", buffer);
        if (IoTHubClient_LL_SendEventAsync(iotHubClientHandle, messageHandle, OnMessageSentCallback, NULL) != IOTHUB_CLIENT_OK)
        {
            Serial.println("Failed to hand over the message to IoTHubClient.");
        }
        else
        {
            messagePending = true;
            Serial.println("IoTHubClient queued the message for delivery.");
        }

        IoTHubMessage_Destroy(messageHandle);
    }
}
/**************************************************************************************************************
 * @Function      : OnMessageSentCallback
 * @Description   : This is sent callback function 
 * @Param         : IOTHUB_CLIENT_CONFIRMATION_RESULT result, 
 *                  void *userContextCallback
 * @Return        : void
 ***************************************************************************************************************/
static void OnMessageSentCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void *userContextCallback)
{
    if (IOTHUB_CLIENT_CONFIRMATION_OK == result)
    {
        Serial.println("Message sent!");
        ControlLed(redLed, true);
        delay(250);
        
        ControlLed(redLed, false);
        
        while(digitalRead(resetWIFISwitch) == LOW)
        {
            CheckWIFIReset();
        }
    }
    else
    {
        Serial.println("Failed to send message to Azure IoT Hub");
    }
    messagePending = false;
}
/**************************************************************************************************************
 * @Function      : ReceiveMessageCallback
 * @Description   : This is the callback specified by the device for receiving messages from IoT Hub.
 * @Param         : IOTHUB_MESSAGE_HANDLE message, 
 *                  void* userContextCallback
 * @Return        :IOTHUBMESSAGE_DISPOSITION_RESULT
 ***************************************************************************************************************/
static IOTHUBMESSAGE_DISPOSITION_RESULT ReceiveMessageCallback(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback)
{
  /* 1 = SendData
     2 = DisplayBattery
     3 = DisplayEnvironment
     4 = DisplayDistance
     5 = ClearWIFI
     6 = DisplayFreeHeap
     7 = HideFreeHeap
     8 = SetLED
     9 = ConfigInterval   */
    
    int commandID;       
    int* counter = (int*)userContextCallback;
    const char* buffer;
    size_t size;
    MAP_HANDLE mapProperties;
    const char* messageId;
    const char* correlationId;

    /* Message properties  */
    if ((messageId = IoTHubMessage_GetMessageId(message)) == NULL)
    {
        messageId = "<null>";
    }

    if ((correlationId = IoTHubMessage_GetCorrelationId(message)) == NULL)
    {
        correlationId = "<null>";
    }

    /* Message content  */
    if (IoTHubMessage_GetByteArray(message, (const unsigned char**)&buffer, &size) != IOTHUB_MESSAGE_OK)
    {
        (void)printf("unable to retrieve the message data\r\n");
    }
    else
    {
                    
        if (size == (strlen("SendData") * sizeof(char)) && memcmp(buffer, "SendData", size) == 0)
        {
          commandID = 1;
          SendData();
        }
       
        if (size == (strlen("DisplayBattery") * sizeof(char)) && memcmp(buffer, "DisplayBattery", size) == 0)
        {
            commandID = 2;
            printBatteryStats();
        }

        if (size == (strlen("DisplayEnvironment") * sizeof(char)) && memcmp(buffer, "DisplayEnvironment", size) == 0)
        {
            commandID = 3;
            PrintEnvironment();
        }

         if (size == (strlen("DisplayDistance") * sizeof(char)) && memcmp(buffer, "DisplayDistance", size) == 0)
        {
            commandID = 4;
            GetDistance();           
        }
        
        if (size == (strlen("ClearWIFI") * sizeof(char)) && memcmp(buffer, "ClearWIFI", size) == 0)
        {
            commandID = 5;
            (void)printf("Clearing WIFI:\r\n");
            strcpy(DeviceConfig.SSIDName, "");
            strcpy(DeviceConfig.SSIDPass, "");
            saveConfig();
            (void)printf("WIFI info cleared:\r\n");
            resetDevice = true;
        }
        
        if (size == (strlen("DisplayFreeHeap") * sizeof(char)) && memcmp(buffer, "DisplayFreeHeap", size) == 0)
        {
            commandID = 6;
            displayFreeHeapFlag = true;
        }

        if (size == (strlen("HideFreeHeap") * sizeof(char)) && memcmp(buffer, "HideFreeHeap", size) == 0)
        {
            commandID = 7;
            displayFreeHeapFlag = false;
        }

        if (size == (strlen("SetLED") * sizeof(char)) && memcmp(buffer, "SetLED", size) == 0)
        {
            commandID = 8;            
        }

        if (size == (strlen("ConfigInterval") * sizeof(char)) && memcmp(buffer, "ConfigInterval", size) == 0)
        {
            commandID = 9;      
            (void)printf("ConfigInterval\r\n");      
        }
    } 

    /* Retrieve properties from the message    */
    mapProperties = IoTHubMessage_Properties(message);
    if (mapProperties != NULL)
    {
        const char*const* keys;
        const char*const* values;
        size_t propertyCount = 0;
        if (Map_GetInternals(mapProperties, &keys, &values, &propertyCount) == MAP_OK)
        {
            if(commandID == 8 || commandID == 9)
            {                              
                if (propertyCount > 0)
                {
                    size_t index;
                       
                    for (index = 0; index < propertyCount; index++)
                    {   
                        String keyStr = String(keys[index]);
                        String val = String(values[index]);
                                            
                        if(commandID == 8)
                        {
                          
                          if(keyStr.equalsIgnoreCase("red"))
                            redLedDuration = val.toInt();

                          if(keyStr.equalsIgnoreCase("green"))
                            greenLedDuration = val.toInt();

                          if(keyStr.equalsIgnoreCase("blue"))
                            blueLedDuration = val.toInt();
                                                  
                          Serial.println("Setting LEDs\r\n");                                                    
                        }

                        if(commandID == 9)
                        {                                                     
                            if(keyStr.equalsIgnoreCase("DeepSleepInt"))
                              DeviceConfig.DeepSleepInt = val.toInt();
  
                            if(keyStr.equalsIgnoreCase("TempPostInt"))
                              DeviceConfig.TempPostInt = val.toInt();

                            if(keyStr.equalsIgnoreCase("DistancePostInt"))
                              DeviceConfig.DistancePostInt = val.toInt(); 

                            if(keyStr.equalsIgnoreCase("BatteryStateInt"))
                              DeviceConfig.BatteryStateInt = val.toInt();

                            if(keyStr.equalsIgnoreCase("AllStateInt"))
                              DeviceConfig.AllStateInt = val.toInt();                               
                                                    
                        }
                    }

                   
                    if(commandID == 9)
                    {
                      if (!saveConfig()) {
                        (void)printf("Failed to save config");
                      } 
                      else 
                      {
                        (void)printf("Config saved");
                      }
                    }
                          
                    (void)printf("\r\n");
                }
            }
        }
    }

    (*counter)++;
    return IOTHUBMESSAGE_ACCEPTED;
}
/**************************************************************************************************************
 * @Function      : GetDeviceId
 * @Description   : This function is Used to get device ID
 * @Param         : char* connectionString, 
 *                  char* deviceID, size_t size
 * @Return        : size_t
 ***************************************************************************************************************/
static size_t GetDeviceId(char* connectionString, char* deviceID, size_t size)
{
    size_t result;
    char* runStr = connectionString;
    char ustate = 0;
    char* start = NULL;

    if (runStr == NULL)
    {
        result = 0;
    }
    else
    {
        while (*runStr != '\0')
        {
            if (ustate == 0)
            {
                if (strncmp(runStr, "DeviceId=", 9) == 0)
                {
                    runStr += 9;
                    start = runStr;
                }
                ustate = 1;
            }
            else
            {
                if (*runStr == ';')
                {
                    if (start == NULL)
                    {
                        ustate = 0;
                    }
                    else
                    {
                        break;
                    }
                }
                runStr++;
            }
        }

        if (start == NULL)
        {
            result = 0;
        }
        else
        {
            result = runStr - start;
            if (deviceID != NULL)
            {
                for (size_t i = 0; ((i < size - 1) && (start < runStr)); i++)
                {
                    *deviceID++ = *start++;
                }
                *deviceID = '\0';
            }
        }
    }

    return result;
}
/**************************************************************************************************************
 * @Function      : InitEnvironment
 * @Description   : This function is Used to lower case
 * @Param         : char* _str
 * @Return        : char *
 ***************************************************************************************************************/
char* strlow(char* _str)
{
  for(unsigned int c = 0; _str[c]; _str[c++] = tolower(_str[c]));
  return _str;
}
/**************************************************************************************************************
 * @Function      : SendData
 * @Description   : This function is Used to sent data 
 * @Param         : void
 * @Return        : void
 ***************************************************************************************************************/
void SendData()
{
  String errorStr = "";
  int errorCnt = 0;
  char messagePayload[MESSAGE_MAX_LEN];
        
  Serial.println("Will send to hub from SendData");
        
        if(distanceSensor)
        {
          GetDistance();
         } 
         else
         {
          if(errorStr.length() == 0)
          {
            errorStr = "Distance sensor failed";
          }
          else
          {
            errorStr = errorStr + ";" + "Distance sensor failed";
          }
          errorCnt++;
        }

        if(environmentSensor)
        {
          SetEnvironment();
          PrintEnvironment();
        } 
        else 
        {
          if(errorStr.length() == 0)
          {
            errorStr = "Environment sensor failed";
          }
          else
          {
            errorStr = errorStr + ";" + "Environment sensor failed";
          }
          errorCnt++;
        }

        if(batterySensor)
        {
          SetBatteryStats();
          printBatteryStats();
        } 
        else 
        {
          if(errorStr.length() == 0)
          {
            errorStr = "Battery sensor failed";
          }
          else
          {
            errorStr = errorStr + ";" +  "Battery sensor failed";
          }
          errorCnt++;
        }        

        Serial.println(errorStr);
        char errors[(errorStr.length() + 1)];
        errorStr.toCharArray(errors, (errorStr.length() + 1));
      
        int heap = ESP.getFreeHeap();
        char heapStr[16];
        itoa(heap, heapStr, 10);
             
        ComposeDeviceStateMessage(messagePayload, "DeviceState", errorCnt, errors, heapStr);
        QueueMessage(iotHubClientHandle, messagePayload);
}
