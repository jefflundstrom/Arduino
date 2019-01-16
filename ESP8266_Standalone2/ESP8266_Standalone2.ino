/**************************************************************
 * Blynk is a platform with iOS and Android apps to control
 * Arduino, Raspberry Pi and the likes over the Internet.
 * You can easily build graphic interfaces for all your
 * projects by simply dragging and dropping widgets.
 *
 *   Downloads, docs, tutorials: http://www.blynk.cc
 *   Blynk community:            http://community.blynk.cc
 *   Social networks:            http://www.fb.com/blynkapp
 *                               http://twitter.com/blynk_app
 *
 * Blynk library is licensed under MIT license
 * This example code is in public domain.
 *
 **************************************************************
 * This example runs directly on ESP8266 chip.
 *
 * You need to install this for ESP8266 development:
 *   https://github.com/esp8266/Arduino
 *
 * Please be sure to select the right ESP8266 module
 * in the Tools -> Board menu!
 *
 * Change WiFi ssid, pass, and Blynk auth token to run :)
 *
 **************************************************************/

#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "da2e988f08844129b42760f72f9f3399";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "twodolphins";
char pass[] = "twodolphinsrock";

const int forwardMotor = 5;
const int backwardMotor = 4;
const int limitSwitchUp = 13;
const int limitSwitchDown = 12;
const int buttonPushDown = 14;

// 0 = stopped, 1 = open, -1 = close
int motorMovement = 0;

void setup()
{
    //Motor right
  pinMode(forwardMotor, OUTPUT);
  //Motor left
  pinMode(backwardMotor, OUTPUT);

  //Limit Switch
  pinMode(limitSwitchUp, INPUT);
  pinMode(limitSwitchDown, INPUT);

  //Demand Button
  pinMode(buttonPushDown, INPUT);

  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);

  if(HIGH == digitalRead(limitSwitchUp))
     Blynk.virtualWrite(4, "Opened");
  else if(HIGH == digitalRead(limitSwitchDown))   
     Blynk.virtualWrite(4, "Closed");
  else
     Blynk.virtualWrite(4, "Restarted Unknown");

  Blynk.virtualWrite(5, "Restarted Stopped");
}

//This comes from the app telling us to open or close the door.
BLYNK_WRITE(V6) //Button Widget is writing to pin V1
{
  if(param.asInt() != 1)
   {
    //Open the door
    motorMovement = 1;
    Blynk.virtualWrite(5, "Commanded Opening");
   }
  else
  {    //Close the door
    motorMovement = -1;     
    Blynk.virtualWrite(5, "Commanded Closing");
  }
}

void loop()
{
  Blynk.run();
  
  int forwardCommand = LOW;
  int backwardCommand = LOW;

  //See if the push button is pressed, if so, open close or stop
  if(HIGH == digitalRead(buttonPushDown))
  {
    //They have to let up before we continue.
    while(HIGH == digitalRead(buttonPushDown))
      delay(100);
      
    //The user is trying to do something with the switch.
    //If we are moving, the user is wanting us to stop!
    if(motorMovement != 0)
      motorMovement =0;
    //If the door is open completely, we will go down.
    else if(HIGH == digitalRead(limitSwitchUp))
      motorMovement = -1;
    else
      //Otherwise, we are going to go up!
      motorMovement = 1;
  }
  
  //If we are not stopped, see if we need to stop.
  if(motorMovement != 0)
  {
    Blynk.virtualWrite(4, "Partially Open");
    //If we are moving up and the limit switch is not hit, move up
    if(motorMovement == 1 && LOW == digitalRead(limitSwitchUp) )
    {
        forwardCommand = HIGH;
        Blynk.virtualWrite(5, "Opening");
    }
    //If we are moving down and the limit switch is not hit, move down
    else if (motorMovement == -1 && LOW == digitalRead(limitSwitchDown) )
    {
      backwardCommand = HIGH;
      Blynk.virtualWrite(5, "Closing");
    }
    //WE are stopped if we are on a limit.
    else 
    {
      motorMovement = 0;
      Blynk.virtualWrite(5, "Stopped");
      if(HIGH == digitalRead(limitSwitchDown))
         Blynk.virtualWrite(4, "Closed");
      else   
         Blynk.virtualWrite(4, "Opened");
    }
  }

  //See if we need to change.
  if( forwardCommand != digitalRead(forwardMotor))
    digitalWrite(forwardMotor, forwardCommand);
  if( backwardCommand != digitalRead(backwardMotor))
    digitalWrite(backwardMotor, backwardCommand);
  
}

