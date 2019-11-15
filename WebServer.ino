/***********************************************************************************************
 **@file                 : Webserver.ino
 **@description          : This file handles webserver functionality
 **@copyright            : Copyright 2019 Mars Inc, All Rights Reserved.
 ** Mars Confidential
 ***********************************************************************************************/
/**************************************************************************************************************
 * @Function      : InitWebServer
 * @Description   : This function is Used to initialize web server 
 * @Param         : void 
 * @Return        : bool
 ***************************************************************************************************************/
bool InitWebServer(void)
{
  /* Call the 'handleRoot' function when a client requests URI "/"   */
  server.on ( "/", handleRoot );                   
  server.on ( "/siotcs", handleSetup );
  server.on ( "/images/characters.png", handleImage );
  /*When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound" */
  server.onNotFound ( handleNotFound );            
  /* Actually start the server  */
  server.begin();                                   
  Serial.println("HTTP server started");
}
/**************************************************************************************************************
 * @Function      : handleRoot
 * @Description   : This function is Used to handle request of wifi configuration of ESP8266 module from IOS application
 * @Param         : void 
 * @Return        : void
 ***************************************************************************************************************/
void handleRoot()
{
  /*To check if "SSID" argument exists  or not*/
  if(server.hasArg("SSID"))
  {
    Serial.println("Will root handle submit");
    /* it handle data received from IOS application   */
    handleSubmit();
  }
  else 
  {
    /* if "SSID" argument is not exist  */
    Serial.println("Will not root handle submit");
    /* open file /wwwroot/index.html */
    File file = SPIFFS.open("/wwwroot/index.html", "r");
    /* And send it to the client   */
    server.streamFile(file, "text/html");
    /* Then close the file again */
    file.close();
  }
}
/**************************************************************************************************************
 * @Function      : handleSetup
 * @Description   : This function is Used to handle device ID request
 * @Param         : void 
 * @Return        : void
 ***************************************************************************************************************/
void handleSetup()
{
  /*To check if "DI" argument exists  or not  */ 
  if (server.hasArg("DI"))
  {
    Serial.println("Will handle setup");
     /* it handle data received from IOS application   */
    handleSetupSubmit();
  }
  else 
  {
     /* if "DI" argument is not exist then stream the file from file system */
    Serial.println("Will not handle submit");
  /* open file /wwwroot/device.html */
    File file = SPIFFS.open("/wwwroot/device.html", "r");
    /* And send it to the client   */
    server.streamFile(file, "text/html");
    /* Then close the file again */
    file.close();
  }
}
/**************************************************************************************************************
 * @Function      : handleImage
 * @Description   : This function is Used to handleImage
 * @Param         : void 
 * @Return        : void
 ***************************************************************************************************************/
void handleImage()
{  
    Serial.println("Will not handle image");
      /* open file /wwwroot/images/characters.png*/
    File file = SPIFFS.open("/wwwroot/images/characters.png", "r");
      /* And send it to the client   */
    server.streamFile(file, "image/png");
     /* Then close the file again */
    file.close();
}
/**************************************************************************************************************
 * @Function      : returnFail
 * @Description   : This function return fail message
 * @Param         : String msg
 * @Return        : void
 ***************************************************************************************************************/
void returnFail(String msg)
{
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(500, "text/plain", msg + "\r\n");
}
/**************************************************************************************************************
 * @Function      : handleSubmit
 * @Description   : This function is handleSubmit 
 * @Param         : void 
 * @Return        : void
 ***************************************************************************************************************/
void handleSubmit()
{
   /*To check if "SSID" argument exists  or not*/
  if (!server.hasArg("SSID")) 
  {
    return returnFail("BAD ARGS");
  }

  strcpy(DeviceConfig.SSIDName, server.arg("SSID").c_str());
  strcpy(DeviceConfig.SSIDPass, server.arg("SSIDPass").c_str());
  /* if SSID argument exist then save configuration */
  saveConfig();
  
  delay(2000);
  /* open file /wwwroot/thanku.html */
  File file = SPIFFS.open("/wwwroot/thanku.html", "r");
    /* And send it to the client   */
  server.streamFile(file, "text/html");
   /* Then close the file again */
  file.close();
  resetDevice = true;
}
/**************************************************************************************************************
 * @Function      : handleSetupSubmit
 * @Description   : This function is Used to handleSetupSubmit
 * @Param         : void 
 * @Return        : void
 ***************************************************************************************************************/
void handleSetupSubmit()
{
  if (!server.hasArg("DI"))
  {
    return returnFail("BAD ARGS");
  }

  strcpy(DeviceConfig.ConnectionString, server.arg("DI").c_str());
  
  Serial.println(DeviceConfig.ConnectionString);
  delay(2000);
  saveConfig();
   /* open file /wwwroot/csconf.html*/
  File file = SPIFFS.open("/wwwroot/csconf.html", "r");
   /* And send it to the client   */
  server.streamFile(file, "text/html");
   /* Then close the file again */
  file.close();
  resetDevice = true;
}
/**************************************************************************************************************
 * @Function      : returnok
 * @Description   : This function is return ok 
 * @Param         : void 
 * @Return        : void
 ***************************************************************************************************************/
void returnOK()
{
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "OK\r\n");
}
/**************************************************************************************************************
 * @Function      : handleNotFound
 * @Description   : This function is Used to check handle
 * @Param         : void 
 * @Return        : void
 ***************************************************************************************************************/
void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) 
  {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }
  server.send ( 404, "text/plain", message );
}
/**************************************************************************************************************
 * @Function      : getContentType
 * @Description   : This function is Used to get  type 
 * @Param         : String filename
 * @Return        : string
 ***************************************************************************************************************/
String getContentType(String filename) 
{
  if (server.hasArg("download")) 
  {
    return "application/octet-stream";
  } else if (filename.endsWith(".htm")) 
  {
    return "text/html";
  } else if (filename.endsWith(".html")) 
  {
    return "text/html";
  } else if (filename.endsWith(".css")) 
  {
    return "text/css";
  } else if (filename.endsWith(".js"))
  {
    return "application/javascript";
  } else if (filename.endsWith(".png")) 
  {
    return "image/png";
  } else if (filename.endsWith(".gif")) 
  {
    return "image/gif";
  } else if (filename.endsWith(".jpg")) 
  {
    return "image/jpeg";
  } else if (filename.endsWith(".ico")) 
  {
    return "image/x-icon";
  } else if (filename.endsWith(".xml")) 
  {
    return "text/xml";
  } else if (filename.endsWith(".pdf")) 
  {
    return "application/x-pdf";
  } else if (filename.endsWith(".zip")) 
  {
    return "application/x-zip";
  } else if (filename.endsWith(".gz")) 
  {
    return "application/x-gzip";
  }
  return "text/plain";
}
