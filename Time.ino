/***********************************************************************************************
 **@file                 : Time.ino
 **@description          : This file handles timing functionality
 **@copyright            : Copyright 2019 Mars Inc, All Rights Reserved.
 ** Mars Confidential
 ***********************************************************************************************/
const int NTP_PACKET_SIZE = 48;          /* NTP time is in the first 48 bytes of message  */
byte packetBuffer[NTP_PACKET_SIZE];      /*buffer to hold incoming & outgoing packets    */
/**************************************************************************************************************
 * @Function      : initTime
 * @Description   : This function is Used to initialize time
 * @Param         : void
 * @Return        : void
 ***************************************************************************************************************/
void initTime()
{
    time_t epochTime;

    configTime(0, 0, "pool.ntp.org", "time.nist.gov");

    while (true)
    {
        epochTime = time(NULL);

        if (epochTime == 0) 
        {
            Serial.println("Fetching NTP epoch time failed! Waiting 2 seconds to retry.");
            delay(2000);
        } 
        else 
        {
            Serial.print("Fetched NTP epoch time is: ");
            Serial.println(epochTime);
            break;
        }
    }
}
/**************************************************************************************************************
 * @Function      : digitalClockDisplay
 * @Description   : This function is Used to initialize LED variable
 * @Param         : void
 * @Return        : void
 ***************************************************************************************************************/
void digitalClockDisplay()
{
  /* digital clock display of the time  */
  String D_T = String("DATE : ") + String(year()) + "-" + String(month()) + "-" + String(day()) + String(" TIME : ") + String(hour()) + ":" + String(minute()) + ":" + String(second());
  Serial.println(D_T);
}
/**************************************************************************************************************
 * @Function      : getNtpTime
 * @Description   : This function is Used to get NTP time
 * @Param         : void
 * @Return        : time_t
 ***************************************************************************************************************/
time_t getNtpTime()
{
  /* NTP server's ip address  */
  IPAddress ntpServerIP;    
  /* discard any previously received packets  */               
  while (Udp.parsePacket() > 0) ;          
  Serial.println("Transmit NTP Request");
 /* get a random server from the pool  */
  WiFi.hostByName(ntpServerName, ntpServerIP);     
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500)
  {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE)
    {
      Serial.println("Receive NTP Response");
       /* read packet into the buffer  */
      Udp.read(packetBuffer, NTP_PACKET_SIZE);       
      unsigned long secsSince1900;
      /* convert four bytes starting at location 40 to a long integer     */
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  /* return 0 if unable to get the time*/
  return 0;                                          
}
/**************************************************************************************************************
 * @Function      : sendNTPpacket
 * @Description   : This function is Used to send an NTP request to the time server at the given address
 * @Param         : IP Address 
 * @Return        : void
 ***************************************************************************************************************/
void sendNTPpacket(IPAddress &address)
{
  /* set all bytes in the buffer to 0   */
  memset(packetBuffer, 0, NTP_PACKET_SIZE);                       
  
  /* Initialize values needed to form NTP request   */
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  
  /* 8 bytes of zero for Root Delay & Root Dispersion   */
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;

  /*send a packet requesting a timestamp:   */
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
