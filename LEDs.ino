
/***********************************************************************************************
 **@file                 : LEDs.ino
 **@description          : This file handles LEDs functionality
 **@copyright            : Copyright 2019 Mars Inc, All Rights Reserved.
 ** Mars Confidential
 ***********************************************************************************************/
/**************************************************************************************************************
 * @Function      : InitLEDs
 * @Description   : This function is Used to initialize LED variable
 * @Param         : bool useKeepAliveLED, 
 *                  int ledPin, 
 *                  int interval
 * @Return        : bool
 ***************************************************************************************************************/
bool InitLEDs(bool useKeepAliveLED, int ledPin, int interval)
{
  activeKeepAliveLED = useKeepAliveLED;
  keepAliveLEDPin = ledPin;
  keepAliveLEDState = LOW;
  keepAlivePreviousMillis = 0;
  keepAliveLEDinterval = interval;
  msec = millis();
  redLedDuration = 0;
  greenLedDuration = 0;
  blueLedDuration = 0;

  pinMode(redLed, OUTPUT);
  digitalWrite(redLed, LOW);
  pinMode(blueLed, OUTPUT);
  digitalWrite(blueLed, LOW);
  pinMode(greenLed, OUTPUT);
  digitalWrite(greenLed, LOW);

  return true;
}
/**************************************************************************************************************
 * @Function      : ControlLed
 * @Description   : This function is Used to flash LED pin
 * @Param         : int pin, 
 *                  bool turnOn
 * @Return        : void
 ***************************************************************************************************************/
void ControlLed(int pin, bool turnOn)
{
  if (turnOn)
  {
    digitalWrite(pin, HIGH);
  }
  else
  {
    digitalWrite(pin, LOW);
  }
}
/**************************************************************************************************************
 * @Function      : LedWatch
 * @Description   : This function is Used to check LED status
 * @Param         : void
 * @Return        : void
 ***************************************************************************************************************/
void LedWatch()
{
  long timeLapsed = millis() - msec;
  if (redLedDuration > 0) 
  {
    digitalWrite(redLed, HIGH);
    redLedDuration -= timeLapsed;
  } 
  else 
  {
    digitalWrite(redLed, LOW);
    redLedDuration = 0;
  }

  if (greenLedDuration > 0) {
    digitalWrite(greenLed, HIGH);
    greenLedDuration -= timeLapsed;
  } 
  else 
  {
    digitalWrite(greenLed, LOW);
    greenLedDuration = 0;
  }

  if (blueLedDuration > 0) {
    digitalWrite(blueLed, HIGH);
    blueLedDuration -= timeLapsed;
  } 
  else 
  {
    digitalWrite(blueLed, LOW);
    blueLedDuration = 0;
  }
  msec = millis();
}
/**************************************************************************************************************
 * @Function      : FlashLED
 * @Description   : This function is Used to flash LED for given duraion
 * @Param         : int led, 
 *                  int duration, 
 *                  int numberOfFlashes
 * @Return        : void
 ***************************************************************************************************************/
void FlashLED(int led, int duration, int numberOfFlashes)
{
  for (int i = 0; i < numberOfFlashes; i++) 
  {
    digitalWrite(led, HIGH);
    delay(duration);
    digitalWrite(led, LOW);
    delay(duration);
  }
}
/**************************************************************************************************************
 * @Function      : KeepAliveLed
 * @Description   : This function is Used to keep  LED alive
 * @Param         : void
 * @Return        : void
 ***************************************************************************************************************/
void KeepAliveLed(void)
{
  if (!activeKeepAliveLED)
  {
    return;
  }

  unsigned long currentMillis = millis();

  if (currentMillis -  keepAlivePreviousMillis >= keepAliveLEDinterval / 2)
  {
    /* save the last time you blinked the LED  */
    keepAlivePreviousMillis = currentMillis;

    /* if the LED is off turn it on and vice-versa:  */
    if (ledState == LOW) 
    {
      ledState = HIGH;
    } 
    else 
    {
      ledState = LOW;
    }
    digitalWrite(keepAliveLEDPin, ledState);
  }

}
