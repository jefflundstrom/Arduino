/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial
#define TRIGGER 5 //D1
#define ECHO    4 //D2
#define MOTIONSENSOR 13 //D7
#define TAILLIGHTRELAY 14 //D5
#define BRAKELIGHTRELAY 12  //D6
 

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <TimeLib.h>
#include <WidgetRTC.h>
#include <Time.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "a1db02cc15694e9f8922924b624022f9";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "twodolphins";
char pass[] = "twodolphinsrock";
//For syncing the time.
WidgetRTC rtc;
bool _lightOn = false;
//Distance in CM's
long _distanceToCar = 10000;
long _delayToTurnOff = 300000;
long _delayToTurnOn = 2000;

long _distanceToBrake = 120;
long _distanceToFlash = 305;

#define BRAKEOFF 0
#define BRAKEFLASH 1
#define BRAKEON 2
//Brake light status
long _brakeLightStatus = BRAKEOFF;

void setup()
{
  // Debug console
Serial.begin (9600);
  pinMode(TRIGGER, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(MOTIONSENSOR,INPUT);
  pinMode(TAILLIGHTRELAY, OUTPUT);
  pinMode(BRAKELIGHTRELAY, OUTPUT);

  digitalWrite(TAILLIGHTRELAY, 0);
  Blynk.begin(auth, ssid, pass);
  // Synchronize time on connection
  rtc.begin();  

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

unsigned long _startTime = 0;
unsigned long _endTime = 0;
bool _lastMotion = false;

//Return the motion if there is motion in the area or not.
bool IsThereMotion()
{
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

//Simple time checker for flashing the light.
long _lastBlink =0;
bool _lastBlinkOn = false;
//Blink every 1/4 second if possible.
int _blinkRate = 250;

///the main loop.
void loop()
{
  Blynk.run();

    _lightOn = IsThereMotion();
    digitalWrite(TAILLIGHTRELAY, _lightOn?1:0);

    //We only care to measure if there is movement.
    if(_lightOn){
      MeasureDistance();

      switch(_brakeLightStatus)
      {
        case BRAKEON:
          digitalWrite(BRAKELIGHTRELAY, 1);
        break;
        case BRAKEFLASH:
          if((millis() - _lastBlink) > _blinkRate){
            _lastBlinkOn = !_lastBlinkOn;
            digitalWrite(BRAKELIGHTRELAY, _lastBlinkOn ? 1 : 0);
            _lastBlink = millis();
          }
        break;
        default:
          digitalWrite(BRAKELIGHTRELAY, 0);
        
      }
    }
    else{
    
       digitalWrite(BRAKELIGHTRELAY, 0);
    }
   
    delay(50);

}

BLYNK_CONNECTED() {

  // Request Blynk server to re-send latest values for all pins
  Blynk.syncAll();
}

BLYNK_WRITE(V1)
{
  _delayToTurnOff= param.asDouble()* 1000; 
}

BLYNK_WRITE(V2)
{
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
