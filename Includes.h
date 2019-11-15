/***********************************************************************************************
 **@file                 : Include.h
 **@description          : This file have all global variables and #define
 **@copyright            : Copyright 2019 Mars Inc, All Rights Reserved.
 ** Mars Confidential
 ***********************************************************************************************/

#ifndef IOT_CONFIGS_H
#define IOT_CONFIGS_H

ESP8266WebServer server(80);
IPAddress    apIP(192, 168, 4, 1);  // Defining a static IP address for AP: local & gateway
                                    // Default IP in AP mode is 192.168.4.1
BME280 mySensor;


/* Software version 0.0.1 for basic MARS functionality   */
String SoftwareVersion = "0.0.1";
 
#define redLed 13
#define blueLed 3
#define greenLed 15
#define resetWIFISwitch 0
#define SHDN_Pin 14
#define I2C_SDA 5
#define I2C_SCL 4

#define SLEEP_TIME 600e6        // 600 Sec = 10 Minute

#define MAX_DEVICE_ID_SIZE  20
#define MESSAGE_MAX_LEN 512

bool ledState = false;
bool environmentSensor = true;
bool distanceSensor = true;
bool batterySensor = true;

/* This are the WiFi access point settings. */
char *APssid = "IOTDispenserAP";
char *password = "password1234";

const unsigned int BATTERY_CAPACITY = 2650;

float humidity, temperature; 
int WIFIResetCount;
int WIFIConnectionTryCnt;

// NTP Servers:
static const char ntpServerName[] = "us.pool.ntp.org";
//static const char ntpServerName[] = "time.nist.gov";
//static const char ntpServerName[] = "time-a.timefreq.bldrdoc.gov";
//static const char ntpServerName[] = "time-b.timefreq.bldrdoc.gov";
//static const char ntpServerName[] = "time-c.timefreq.bldrdoc.gov";

//const int timeZone = 1;     // Central European Time
const int timeZone = -5;  // Eastern Standard Time (USA)
//const int timeZone = -4;  // Eastern Daylight Time (USA)
//const int timeZone = -8;  // Pacific Standard Time (USA)
//const int timeZone = -7;  // Pacific Daylight Time (USA)

WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

time_t getNtpTime();
void digitalClockDisplay();
void printDigits(int digits);
void sendNTPpacket(IPAddress &address);
time_t prevDisplay = 0; // when the digital clock was displayed

static bool messagePending = false;
int *receiveContext;
bool AP = false;
bool bootup = true;
int resetCount  = 0;

static IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;
/*enum of Threshold for different Sensors*/
enum Threshold 
{
  Max_Time = 23,                          //Every Sleep Mode Time 10 min. So, for 4 hr --- Max_Time = 24
  DISTANCE = 2,
  TEMPERATURE = 2,
  HUMIDITY = 2,
  Init_Time=10000
};
/*Structure for WIFI setting data Handling*/
struct Config 
{
  char ConnectionString[180];
  char SSIDName[64];
  char SSIDPass[32];
  int DeepSleepInt;
  int TempPostInt;
  int DistancePostInt;
  int BatteryStateInt;
  int AllStateInt;
  bool PostAllInfo;
};
/*Structure for Data handling of Sensor*/
struct DataDevice
{
    double Distance;
    double Temperature;
    double Humidity;
    int BatteryPercentage;
    int BatteryState;
    int MotionCount;
    int SwitchCount;
};
/*Structure for RTC Buffer Data Handling*/
struct rtcData
{
  double Distance_RTC;
  double Temperature_RTC;
  double Humidity_RTC;
  int Critical_Flag;
};


struct rtcData rtcData;
struct Config DeviceConfig;
struct DataDevice DeviceData;

long duration, resetMills;
bool hubConnected;
bool displayFreeHeapFlag;
bool resetDevice;

static unsigned long timer,msec;
unsigned long previousSendMillis;
unsigned long previousTimeoutMillis;
int redLedDuration;
int greenLedDuration;
int blueLedDuration;
double cm, inches;

bool activeKeepAliveLED;
int keepAliveLEDPin;
int keepAliveLEDState;           
unsigned long keepAlivePreviousMillis;    
unsigned long keepAliveLEDinterval;
#endif /* IOT_CONFIGS_H */
