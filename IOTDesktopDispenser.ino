/***********************************************************************************************
 **@file                 : IOTDesktopDispenser.ino
 **@description          : This file handles the IoT Dispenser ESP8266 WiFi Module functionality
 **@copyright            : Copyright 2019 Mars Inc, All Rights Reserved.
 ** Mars Confidential
 ***********************************************************************************************/
#include <Wire.h>
#include <Arduino.h>
#include <Time.h>
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <BQ27441_Definitions.h>
#include <SparkFunBQ27441.h>
#include <SparkFunBME280.h>
#include <AzureIoTHub.h>
#include <AzureIoTProtocol_HTTP.h>
#include <ArduinoJson.h>
#include "Adafruit_VL6180X.h"
#include "FS.h"
#include "Includes.h"
#define ARDUINOJSON_ENABLE_STD_STRING 1

/* static variables */
static double Prev_Buffer[4];

/* Global Varisbles */
/* This Flag is used check timeout data send if difference not found for 4Hrs*/
int Critical_Flag = 0;      
/* This Flag is used to verify that if data difference found then stop sleep mode utill "Message Sent" message recieved*/
int Run_Flag=0;             
/* For Timeout if program stuck somewhere in the loop*/
int Counter=0; 

Adafruit_VL6180X vl;

/* Function */
/*Sleep Mode Function declaration*/
void Sleep_Mode();
/******************************************************************************************************************************************
 * @Function      :  setup()
 * @Description   :  This function is Used to initialize variables,baudrate ,pin modes and  it will run only after each powerup or reset 
 * @Param         : void
 * @Return        : void    
 *******************************************************************************************************************************************/
void setup()
{
  /* Configured I2C Pin i.e  IO5--SDA & IO4--SCL*/
  Wire.begin(I2C_SDA,I2C_SCL);                       
  
  Critical_Flag=1;
  /* Configured  SHDN_Pin in output mode */
  pinMode(SHDN_Pin, OUTPUT);
  /* Pull HIGH SHDN Pin i.e. Initiate Distance Sensor */
  digitalWrite(SHDN_Pin,HIGH);  
  /* Configured  blueLed in output mode */
  pinMode(blueLed, FUNCTION_3);
  delay(1000);
   /* opens serial port and  sets data rate to 115200 bps */
  Serial.begin(115200);   
  
  Serial.println("\nSoftware version:"+SoftwareVersion);                         
  
  resetDevice = false;
  WIFIResetCount = 0;
  WIFIConnectionTryCnt = 0;
  previousSendMillis = millis();
  previousTimeoutMillis = millis();

  displayFreeHeapFlag = false;

  hubConnected = false;
  messagePending = false;
  
  Serial.println("Mounting FS...");
 /*begin() function will return true when mounts SPIFFS file system successfully */
  if(!SPIFFS.begin())                               
  {
    Serial.println("Failed to mount file system");
    return;
  }
 
  Serial.println(ESP.getFreeHeap());
  if (!loadConfig())
  {
    Serial.println("Failed to load config");
  } 
  else 
  {
    Serial.println("Config loaded");
  }

  while(digitalRead(resetWIFISwitch) == LOW)        
  {
    /* Monitor WiFi reset switch */
     CheckWIFIReset();                                  
  }

  /* Print the Data from config.json file to Serial Monitor*/
  printFile("/config.json");

  delay(1000);
  /* Checking WIFI is configured or not*/
  if (strlen(DeviceConfig.ConnectionString) == 0 || strlen(DeviceConfig.SSIDName) == 0)
  {
    AP = true;
  }
  else
  {
    AP = false;
  }

  /* set resetWIFISwitch as INPUT_PULLUP   */
  WIFIResetInit();                                      
  /* Monitor WiFi reset switch */
  while(digitalRead(resetWIFISwitch) == LOW)
  {
     CheckWIFIReset();                                     
  }
  /*  Check  LED status  */
  if (!InitLEDs(true, greenLed, 60000))                 
  {
    Serial.println("Init LEDs error.");
  }
   /* check Battery sensor Initialization   */
  if (!InitBattery())                                   
  {
    Serial.println("Battery sensor error.");
    /* if Initialization fail set batterySensor = false   */
    batterySensor = false;                                                
  }
  /* check Environment Sensor Initialization   */
  if (!InitEnvironment())                               
  {
    Serial.println("Enviroment sensor error.");
    /* if Initialization fail set environmentSensor = false   */
    environmentSensor = false;                           
  }
  /* check Distance Sensor Initialization  */
  if(!Init6180())                                       
  {
    Serial.println("6180 sensor error.");
    /* if Initialization fail set distanceSensor = false   */
    distanceSensor = false;                             
  }
  /* If AP mode Found */
  if (AP == true)                                        
  {    
    /* Initiaize access point  */    
    InitAccessPoint();  
    /* Initiaize web server  */                                
    InitWebServer();                                    
  } 
  else 
  {
    server.close();
    /* set wifi in STATION mode */
    WiFi.mode(WIFI_STA);   
    /* connect to the network */                              
    connect();
    initTime();

    Serial.println("Starting UDP");
    Udp.begin(localPort);
    Serial.print("Local port: ");
    Serial.println(Udp.localPort());
    Serial.println("waiting for sync");
    setSyncProvider(getNtpTime);
    setSyncInterval(300);
    
    while (year() == 1970 || year() > 2020) 
    {
      setSyncProvider(getNtpTime);
      setSyncInterval(300);
      Serial.println("Year = 1970!");
      /*Timeout if Stuck for 3 times*/
      Counter++;
      if(Counter >= 3)
      {
        Counter = 0;
        /*Sleep Mode For Battery Saving*/
        Sleep_Mode();
      }
    }
    /* Initialize IoT hub */
    if (InitIOTHub())                                                
    {
      hubConnected = true;
      Serial.println("Connected to Hub");
    } 
    else 
    {
      Serial.println("Failed to connect to hub");
    }
  }
}
/**************************************************************************************************************************************
 * @Function      :  loop()
 * @Description   :  This function is Used to initialize variables,baudrate , pin modes and  it will run only after each powerup or reset 
 * @Param         : void
 * @Return        : void    
 **************************************************************************************************************************************/
void loop()
{
   /* Read Data from RTC Memory */
   if((!AP)&&(ESP.rtcUserMemoryRead(0, (uint32_t*) &rtcData, sizeof(rtcData)))&&(Run_Flag==0) && !messagePending)     
   {
     Serial.println("RTC Stored Read: ");
     Serial.println("Distance Data: ");
     Serial.println(rtcData.Distance_RTC);
     Serial.println("Temp Data: ");
     Serial.println(rtcData.Temperature_RTC);
     Serial.println("Humidity Data: ");
     Serial.println(rtcData.Humidity_RTC);  
     Serial.println();
     /* Wake Distance Snesor */
     digitalWrite(SHDN_Pin,HIGH);                                                
     delay(100); 
     /* Wake Battery Baby Sitter */
     Clear_Hibernate();                                                         
     /* Wake Sensor & Read Data of TEMP & HUMIDITY  */
     SetEnvironment(); 
     /* Read Data of Distance Sensor */                                                          
     GetDistance();                                                             
     
   }

   /* If difference found in Sensors Data then send to IoT hub  and Max_Time is for heatbeat timeout (i.e.even if there is no difference in data after 4 Hrs data sent to IoT hub */  
   if((AP)||(rtcData.Critical_Flag >= Max_Time)||(abs(rtcData.Distance_RTC-DeviceData.Distance) > DISTANCE)||(abs(rtcData.Temperature_RTC-DeviceData.Temperature) > TEMPERATURE)||(abs(rtcData.Humidity_RTC-DeviceData.Humidity) > HUMIDITY))               
   {            
      unsigned long currentMillis = millis();
      String errorStr = "";
      int errorCnt = 0;
      /* If difference found in data then set Run_Flag utill it will send to Internet*/
      Run_Flag=1;                                                             
      /*Check if Reset is required i.e. checking that ESP released from Flash Mode*/
      CheckWIFIReset();
      
      if(resetDevice)
      {
        resetCount++;
        Serial.println("Will reset!");
        if (resetCount == 20)
        {
          resetDevice = false;
          /*Reset ESP Module*/
          ESP.reset();
        }
      }
        
      if (displayFreeHeapFlag)
      {
        Serial.println(ESP.getFreeHeap());
      }
    
      KeepAliveLed();
    
      LedWatch();
      
      if(AP) 
      {
        /* Listen for HTTP requests from clients */
        server.handleClient();           
        ControlLed(blueLed, true);
        blueLedDuration = 3000;
      } 
      else 
      {
         if(hubConnected)
         {
           /*it will check for  10 msec over and message pending flag the send send data to IOT hub*/
            if(((currentMillis - previousSendMillis) >= Init_Time) && !messagePending)        
            {
                /* Reset Run_Flag if data ready send to Internet   */
                Run_Flag=0; 
                /* Get Distance Sensor Data*/
                rtcData.Distance_RTC=DeviceData.Distance; 
                /* Get Temperature Data*/
                rtcData.Temperature_RTC=DeviceData.Temperature;
                /* Get Humidity Data*/
                rtcData.Humidity_RTC=DeviceData.Humidity;
                /* Clear Critical Flag if data ready send to internet*/
                rtcData.Critical_Flag=0;
                Critical_Flag=0;    
                
                if(WIFIResetCount > 0 || resetDevice == true)
                {
                  Serial.println("RESET");
                  Serial.println(WIFIResetCount);
                  Serial.println(resetDevice);                
                  delay(300);
                  return;
                }        
                  
                char messagePayload[MESSAGE_MAX_LEN];
                previousSendMillis = currentMillis;
        
                Serial.println("Will send to hub");
        
                if (distanceSensor)
                {
                  /* Get data from distance Sensor*/
                  GetDistance();
                } 
                else 
                {
                  if (errorStr.length() == 0)
                  {
                    errorStr = "Distance sensor failed";
                  }
                  else
                  {
                    errorStr = errorStr + ";" + "Distance sensor failed";
                  }
                  errorCnt++;
                }
        
                if (environmentSensor)
                {
                  /* Get the data from Sensor*/
                  SetEnvironment();
                  /* Print the data to Serial Monitor*/
                  PrintEnvironment();
                } 
                else 
                {
                  if (errorStr.length() == 0)
                  {
                    errorStr = "Environment sensor failed";
                  }
                  else
                  {
                    errorStr = errorStr + ";" + "Environment sensor failed";
                  }
                  errorCnt++;
                }
        
                if (batterySensor)
                {
                  /* Get Data from Battery Baby Sitter*/
                  SetBatteryStats();
                  /* Print data to Serial Monitor*/
                  printBatteryStats();
                } 
                else 
                {
                  if (errorStr.length() == 0)
                  {
                    errorStr = "Battery sensor failed";
                  }
                  else
                  {
                    errorStr = errorStr + ";" +  "Battery sensor failed";
                  }
                  errorCnt++;
                }
        
                char errors[(errorStr.length() + 1)];
                errorStr.toCharArray(errors, (errorStr.length() + 1));
        
                int heap = ESP.getFreeHeap();
                char heapStr[16];
                itoa(heap, heapStr, 10);
               
                ComposeDeviceStateMessage(messagePayload, "DeviceState", errorCnt, errors, heapStr);
                QueueMessage(iotHubClientHandle, messagePayload);
               
            }
            /* Handle IoT hub client request */
            IoTHubClient_LL_DoWork(iotHubClientHandle);               
          }  
          else 
          {
            Serial.println("Not connected to hub");
          }      
      }
      delay(500);
   }
   /* Checking Status for Entering in SLEEP MODE i.e. Message is not Pending or sent to Server*/
   if((!AP)&&(Run_Flag==0) && !messagePending)                          
   {
      Serial.println("TEMP & HUM Sensor in SLEEP MODE");
      /* Sleep Mode Activate of Temp & Humidity Sensor  */
      mySensor.setMode(MODE_SLEEP);                              
      Serial.println("Battery Sensor in SLEEP MODE");  
      /* Hibernate Battery Baby Sitter */
      Hibernate();                                                
      Serial.println("Distance Sensor in SLEEP MODE");
      /* Sleep Mode of Distance Sensor    */
      digitalWrite(SHDN_Pin,LOW); 
                                      
      rtcData.Critical_Flag+=Critical_Flag;
      /* Write Current Data of sensors to RTC Memory(Part of Flash Memory) */
      if (ESP.rtcUserMemoryWrite(0, (uint32_t*) &rtcData, sizeof(rtcData)))     
      {
        Serial.println("RTC Stored Write: ");
        /* Print RTC stored memory */ 
        printMemory();
      }
      /* going to sleep mode */
      Sleep_Mode();                                                 
   }
 
}
/*****************************************************************************************************
 * @Function      :  Sleep_Mode()
 * @Description   :  This function is Used to put ESP8266 wifi  module to sleep mode for power saving 
 * @Param         : void
 * @Return        : void    
 ******************************************************************************************************/
void Sleep_Mode()
{
    Serial.println("ESP in SLEEP MODE for 10 Minutes");
    /*  going to Deep Sleep( ESP8266 wifi  module) for 10 minutes  */
    ESP.deepSleep(SLEEP_TIME);                               
}

