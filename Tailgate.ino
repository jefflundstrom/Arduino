#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <TimeLib.h>
#include <WidgetRTC.h>
#include <Time.h>

//External Sensors
#define TRIGGER 5 //D1
#define ECHO    4 //D2
#define MOTIONSENSOR 13 //D7

//12V Light Relays
#define TAILLIGHTRELAY 14 //D5
#define LEFTBRAKELIGHTRELAY 12  //D6
#define RIGHTBRAKELIGHTRELAY 16  //D0

 //These are the states of the brake lights.
#define BRAKEOFF 0
#define BRAKEFLASHLEFT 1
#define BRAKEFLASHRIGHT 2
#define BRAKEFLASH 3
#define BRAKEALTERNATE 4
#define BRAKEON 5

// Auth Token for Blynk App.
char auth[] = "a1db02cc15694e9f8922924b624022f9";

// Your WiFi credentials.
char ssid[] = "twodolphins";
char pass[] = "twodolphinsrock";

//Delays to turn on and off, we dont want it blinking when there is a 
//bad sensor reading.
long _delayToTurnOff = 300000;       //BLINK V1  In Seconds from Blynk
long _delayToTurnOn = 2000;          //BLINK V2  In Seconds from Blynk

//Distance in CM's For Controlling the feedback in lights.
long _distanceToCar = 10000;
long _distanceToBrake = 120;         //BLINK V4
long _distanceToFlash = 305;         //BLINK V3

//Blink every 1/4 second if possible.
int _blinkRate = 250;

//Current Brake light Status
long _brakeLightStatus = BRAKEOFF;   //BLINK V5

//Allow manual control over the blinking 
//for fun with a time out so we dont 
//forget to turn it off..
int _manualOverride = 0;             //BLINK V6

//Manual override will only last an hour at max.
long _manualOverrideTimeLimit = 360000;

//For syncing the time.
WidgetRTC rtc;

//This is just a toggel so we can flash the lights,
bool _lightOn = false;

//Basic setup for the 8266
void setup()
{
  // Debug console
  Serial.begin (9600);
  pinMode(TRIGGER, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(MOTIONSENSOR,INPUT);
  pinMode(TAILLIGHTRELAY, OUTPUT);
  pinMode(LEFTBRAKELIGHTRELAY, OUTPUT);
  pinMode(RIGHTBRAKELIGHTRELAY, OUTPUT);

  //Initialize BLINK
  Blynk.begin(auth, ssid, pass);

  //Initialize the lights off.
  digitalWrite(TAILLIGHTRELAY, 0);
  digitalWrite(LEFTBRAKELIGHTRELAY, 0);
  digitalWrite(RIGHTBRAKELIGHTRELAY, 0);
}

///Uses the ultra sonic sensor to detect the distance from the Car, if it is there.
void MeasureDistance()
{
    // defines variables
    long duration;
    float distance;
    float Mdistance;

    // Clears the trigPin
    digitalWrite(TRIGGER, LOW);
    delayMicroseconds(2);
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(TRIGGER, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER, LOW);
    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(ECHO, HIGH);
     
    // Calculating the distance
    distance= (duration*0.0228)/2;  // Distance with Cm
    //Mdistance= distance/100;        // Distance with m

    _distanceToCar = distance;
    Blynk.virtualWrite(V10, _distanceToCar);

    //Dont worry about flashing if we have manual override on.
    if(_manualOverride != 0)
    {
      //If the Distance is close enough to Brake, do that.
      if(distance < _distanceToBrake)
        _brakeLightStatus = BRAKEON;
      //if the Distance is close enough to flash, do that.
      else if(distance < _distanceToFlash)
        _brakeLightStatus = BRAKEFLASH;
      //Do neither
      else
        _brakeLightStatus = BRAKEOFF;
    }
}


//Return the motion if there is motion in the area or not based on time needed to turn on and off motion
//There maybe motion in the area, but it has to happen for a set number of seconds before we say there is
//and the motion might be gone, but we wait so many seconds before we state no motion..
//This avoids sensor errors and blinking of on off.
bool IsThereMotion()
{
  //These are state variables to keep track of movement to we can turn on and off based on number of seconds.
  static unsigned long _startTime = 0;
  static unsigned long _endTime = 0;
  static bool _lastMotion = false;
  
  //Assume we have no change in motion.
  bool motionInTheArea = _lastMotion;
  
  //See if motion is currently being detected
  unsigned long motionTime = 0;
  bool currentMotionDetected = (digitalRead(MOTIONSENSOR) == HIGH);

  //In order to turn on or off the lights, we need to see if we see movement for at least 2 seconds before we change the state, if so, we will turn the lights on.
  if(currentMotionDetected)
  {
    //We have motion, so if we were counting an endTIme, clear it.
    _endTime = 0;
    //See if we already have a start time, if not, start one.
    if(_startTime == 0)
      _startTime = millis();
    else if(!_lastMotion && ((millis() - _startTime) >= _delayToTurnOn))
      motionInTheArea = true;
    
  }
  else
  {
    //WE have no motion, so if we were counting a start time clear it.
    _startTime = 0;
    //We just stopped moving, so clear the start time.
    if(_endTime == 0)
      _endTime = millis();
    else if(_lastMotion && ((millis() - _endTime) >= _delayToTurnOff))
      motionInTheArea = false;
  }
  
  //Save the current value for the next time we come around.
  _lastMotion = motionInTheArea;
  
  return motionInTheArea;
}

///the main loop.
void loop()
{
    Blynk.run();

    //Flag for when we turn on the lights so we only trun them off once.
    static bool lightsOn = false;

    //We only care to measure if there is movement.
    if(IsThereMotion()){
      //The lights are going on.
      lightsOn = true;
      //There is motion, turn on the taillights.
      digitalWrite(TAILLIGHTRELAY, 1);

      //Measure the distance to see what to do with the brake lights.
      MeasureDistance();

      switch(_brakeLightStatus){
        
        case BRAKEON:
            digitalWrite(LEFTBRAKELIGHTRELAY, 1);
            digitalWrite(RIGHTBRAKELIGHTRELAY, 1);
        break;

        case BRAKEFLASHLEFT:
        case BRAKEFLASHRIGHT:
        case BRAKEFLASH:
        case BRAKEALTERNATE:
          //Simple time checker for flashing the light.
          static long _lastBlink =0;
          static bool _lastBlinkOn = false;
          if((millis() - _lastBlink) > _blinkRate){
            _lastBlinkOn = !_lastBlinkOn;
            if((_brakeLightStatus == BRAKEFLASHLEFT) || (_brakeLightStatus >= BRAKEFLASH))
              digitalWrite(LEFTBRAKELIGHTRELAY, _lastBlinkOn ? 1 : 0);
            if((_brakeLightStatus == BRAKEFLASHRIGHT) || (_brakeLightStatus >= BRAKEFLASH))
              digitalWrite(RIGHTBRAKELIGHTRELAY, _lastBlinkOn ? ((_brakeLightStatus == BRAKEALTERNATE) ? 1:0) : ((_brakeLightStatus == BRAKEALTERNATE) ? 0:1));
            _lastBlink = millis();
          }
        break;

        //Default is LIGHTS OFF!
        default:
            digitalWrite(LEFTBRAKELIGHTRELAY, 0);
            digitalWrite(RIGHTBRAKELIGHTRELAY, 0);
      }
    }
    //Shut the lights off if they were on.
    else if(lightsOn){
      lightsOn = false;
      //Default to lights off, we never leave them on when there is no movement.
      digitalWrite(TAILLIGHTRELAY, 0);
      digitalWrite(LEFTBRAKELIGHTRELAY, 0);
      digitalWrite(RIGHTBRAKELIGHTRELAY, 0);
    }

    //We only want to check motion and distance at the most ever 50 milliseconds max
    delay(50);

    //We only manual override for a set amout of time, we see if manual is over
    if(_manualOverride != 0 && millis() > _manualOverride)
      _manualOverride = 0;

}

//////////////////////////////////////////////////////////////////////////////////////
// Blink Code
/////////////////////////////////////////////////////////////////////////////////////
BLYNK_CONNECTED() {

  // Request Blynk server to re-send latest values for all pins
  Blynk.syncAll();

  // Synchronize time with BLINK on connection
  rtc.begin();  

}

BLYNK_WRITE(V1)
{
  //Convert from seconds to millis as the blynk app is sending seconds not millis
  _delayToTurnOff= param.asDouble()* 1000; 
}

BLYNK_WRITE(V2)
{
  //Convert from seconds to millis as the blynk app is sending seconds not millis
  _delayToTurnOn= param.asDouble()* 1000; 
}

BLYNK_WRITE(V3)
{
  _distanceToFlash= param.asDouble(); 
}

BLYNK_WRITE(V4)
{
  _distanceToBrake= param.asDouble(); 
}

BLYNK_WRITE(V5)
{
  _brakeLightStatus= param.asInt(); 
}

BLYNK_WRITE(V6)
{
  _manualOverride = param.asInt();
  if(_manualOverride != 0)
    _manualOverride = millis() + _manualOverrideTimeLimit;

}
