/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <WiFi.h>
#include <TimeLib.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <WidgetRTC.h>
#include <Time.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "edabd76afcf84cbfa0040a489ffa8282";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "twodolphins";
char pass[] = "twodolphinsrock";



///////////////////////////////////////////////////////////////////////////////
//The Temperature amp
#include <Adafruit_MAX31865.h>
// use hardware SPI, just pass in the CS pin
Adafruit_MAX31865 max = Adafruit_MAX31865(5);
// The value of the Rref resistor. Use 430.0 for PT100 and 4300.0 for PT1000
#define RREF      430.0
// The 'nominal' 0-degrees-C resistance of the sensor
// 100.0 for PT100, 1000.0 for PT1000
#define RNOMINAL  100.0

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);


//////////////////////////////////////////////////////////////////////////////
//TheRelay information
int relayAuger = 17;                 // IN1 connected to digital pin 7
int relayfan = 27;                 // IN2 connected to digital pin 8
int relayHot = 26;                 // IN3 connected to digital pin 9
int relayLight = 25;                 // IN4 connected to digital pin 10
//////////////////////////////////////////////////////////////////////////////

bool temperatureSimulator = false;
//For syncing the time.
WidgetRTC rtc;
WidgetLED ledAuger(V50);
WidgetLED ledFan(V51);
WidgetLED ledHot(V52);
WidgetLED ledCool(V53);

double timeAugerOff=0;
int smokeSecondsOn = 15;
int smokeSecondsOff = 65;
int FahrenheitSetVariance = 10;
void setup(void) {
    
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);
  u8g2.begin();
  
  //Initialize the amp for temperature reading.
  max.begin(MAX31865_3WIRE);  // set to 2WIRE or 4WIRE as necessary

  // Initialize the relay pins for control
  pinMode(relayAuger, OUTPUT);      // sets the digital pin as output
  pinMode(relayfan, OUTPUT);      // sets the digital pin as output
  pinMode(relayHot, OUTPUT);      // sets the digital pin as output
  pinMode(relayLight, OUTPUT);      // sets the digital pin as output
  digitalWrite(relayAuger, HIGH);        // Prevents relays from starting up engaged
  digitalWrite(relayfan, HIGH);        // Prevents relays from starting up engaged
  digitalWrite(relayHot, HIGH);        // Prevents relays from starting up engaged
  digitalWrite(relayLight, HIGH);        // Prevents relays from starting up engage

  timeAugerOff = 0;
}

//The temperature we are going to,
double FahrenheitSet = 150.0f;
time_t  timeHotStarted = 0;
time_t  timeAugerStarted = 0;
time_t  timeFanStarted = 0;
time_t  timeCoolingStarted = 0;

bool bPowerOn = false;
bool bManualOverride = true;
double lastSimulatedFarenheit = 0;
double GetSimulatedFValue()
{
  lastSimulatedFarenheit += IsAugerOn() ? 1 : -1;

  return lastSimulatedFarenheit;
}
void loop(void) {

  Blynk.run();

  uint16_t rtd = max.readRTD();

  //Serial.print("RTD value: "); Serial.println(rtd);
  float ratio = rtd;
  ratio /= 32768;
  //Serial.print("Ratio = "); Serial.println(ratio,8);
  //Serial.print("Resistance = "); Serial.println(RREF*ratio,8);
  float Celsius = max.temperature(RNOMINAL, RREF);
  double Fahrenheit = (!temperatureSimulator || !bPowerOn) ? ((Celsius * 9.0f)/5.0f) + 32.0f : GetSimulatedFValue();
  if(!bManualOverride)
    CommandTraeger(Fahrenheit);

  String sCelsius = String( String(Celsius) + " C");
  //Serial.println(sCelsius);
  String sFahrenheit = String("Cur Temp =" + String(Fahrenheit) + " F");
  String sFahrenheitSet = String("CMD Temp =" + String(FahrenheitSet) + " F");
  //Serial.println(sFahrenheit);

  // Check and print any faults
  uint8_t fault = max.readFault();
  if (fault) {
    Serial.print("Fault 0x"); Serial.println(fault, HEX);
    if (fault & MAX31865_FAULT_HIGHTHRESH) {
      Serial.println("RTD High Threshold"); 
    }
    if (fault & MAX31865_FAULT_LOWTHRESH) {
      Serial.println("RTD Low Threshold"); 
    }
    if (fault & MAX31865_FAULT_REFINLOW) {
      Serial.println("REFIN- > 0.85 x Bias"); 
    }
    if (fault & MAX31865_FAULT_REFINHIGH) {
      Serial.println("REFIN- < 0.85 x Bias - FORCE- open"); 
    }
    if (fault & MAX31865_FAULT_RTDINLOW) {
      Serial.println("RTDIN- < 0.85 x Bias - FORCE- open"); 
    }
    if (fault & MAX31865_FAULT_OVUV) {
      Serial.println("Under/Over voltage"); 
    }
    max.clearFault();
  }

  u8g2.clearBuffer();					// clear the internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
  char bufferString[100];
  sFahrenheit.toCharArray(bufferString, sizeof(bufferString));
  u8g2.drawStr(0,10,bufferString);	// write something to the internal memory
  u8g2.drawStr(0,20, IsAugerOn()? "AUGER ON" : "AUGER OFF");  // write something to the internal memory
  u8g2.drawStr(0,30, IsFanOn() ? "FAN ON" : "FAN OFF");  // write something to the internal memory
  u8g2.drawStr(0,40, IsHotOn()? "GLOW ON" : "GLOW OFF");  // write something to the internal memory
  if(bPowerOn == false)
  {
    u8g2.drawStr(0,50, "POWER IS OFF");  // write something to the internal memory
  }
  else if(FahrenheitSet < 151.0f)
  {
    u8g2.drawStr(0,50, "SMOKE");  // write something to the internal memory
  }
  else
  {
    sFahrenheitSet.toCharArray(bufferString, sizeof(bufferString));
    u8g2.drawStr(0,50,bufferString);  // write something to the internal memory
  }
  u8g2.sendBuffer();          // transfer internal memory to the display

  //Update the states of the led's showing what is going on.
  IsAugerOn() ? ledAuger.on() : ledAuger.off();
  IsFanOn() ? ledFan.on() : ledFan.off();
  IsHotOn() ? ledHot.on() : ledHot.off();
  (timeCoolingStarted != 0 && fanOn != false) ? ledCool.on() : ledCool.off();

}
bool bConnecting = false;
BLYNK_CONNECTED() {
  // Synchronize time on connection
  rtc.begin();  

  bConnecting = true;
  // Request Blynk server to re-send latest values for all pins
  Blynk.syncAll();
  bConnecting = false;

  // Let's write your hardware uptime to Virtual Pin 2
  int value = millis() / 1000;
  Blynk.virtualWrite(V2, value);
}

bool IsAugerOn(){
  return digitalRead(relayAuger) == LOW;  
}
bool IsFanOn(){
  return digitalRead(relayfan) == LOW;  
}
bool IsHotOn(){
  return digitalRead(relayHot) == LOW;  
}


//This will command the traeger depending on given temperature
void CommandTraeger(double currentTemp){

  Blynk.virtualWrite(V5, (int)currentTemp);
  Blynk.virtualWrite(V6, FahrenheitSet);

  bool hotOn = IsHotOn();
  bool fanOn = IsFanOn();
  bool augerOn = IsAugerOn();

  if(bPowerOn)
  {
    //Clear the cooling flag, we are no longer cooling, if we were.
    timeCoolingStarted = 0; 
    Blynk.virtualWrite(V13, timeCoolingStarted);

    //FAN IS ALWAYS ON WHEN WE ARE ON.
    fanOn = true;

    //HOT Is ONLY ON For the first 10 mintues.
    if(timeHotStarted < 1 && currentTemp < 200.0f)
    {
      hotOn = true;
    }
    else if((timeHotStarted + 300) < now())
    {
      hotOn = false;
    }

    //Now for the auger, that depends on the temperature.   
    //For smoking, if so, we need to go on for 5 seconds, and off for the PValue
    if(FahrenheitSet < 151.0f)
    {
      Serial.println("IN SMOKE"); 
      //This is smoke!  We will run for 15 seconds then off for P Seconds.  
      if(IsAugerOn() && (timeAugerStarted + smokeSecondsOn) < now())
      {
        augerOn = false;
        timeAugerOff = now();
      }
      else if(!IsAugerOn() && (timeAugerOff + smokeSecondsOff) < now()) 
      {
        augerOn = true;
        timeAugerOff = 0;
      }
    }
    else if(IsAugerOn() && (currentTemp > (FahrenheitSet + FahrenheitSetVariance)))
    {
      augerOn= false;
    }
    else if(!IsAugerOn() && (currentTemp < (FahrenheitSet - FahrenheitSetVariance)))
    {
      augerOn= true;
    }
  }
  else
  {
    //When we have no power, we will NOT need hotstick or the auger on
    hotOn = augerOn = false;

    //The only thing we need to worry about is if we are cooled, fan needs to run for 10 minutes after shutdown
    if(timeCoolingStarted < 1)
    {
      timeCoolingStarted = now();
      Blynk.virtualWrite(V13, timeCoolingStarted);
    }
    //We only run the fan for 10 minutes after we shut off.
    else if((timeCoolingStarted + 600) < now())
    {
      fanOn = false;
    }
  }
  
  //Start by reporting the fan.
  if(IsHotOn() != hotOn  & hotOn)
  {
      Serial.println("HOT ON"); 
      timeHotStarted = now();
      Blynk.virtualWrite(V10, timeHotStarted);
  }
  if(IsAugerOn() != augerOn)
  {
      Serial.println("auger ON"); 
      timeAugerStarted = augerOn ? now() : 0;
      Blynk.virtualWrite(V11, timeAugerStarted);
  }
  if(IsFanOn() != fanOn)
  {
      Serial.println("FAN ON"); 
      timeFanStarted = fanOn ? now() : 0;
      Blynk.virtualWrite(V12, timeFanStarted);
  }

  //Command the states.
  digitalWrite(relayAuger, augerOn ? LOW : HIGH);        
  digitalWrite(relayfan, fanOn ? LOW : HIGH);        
  digitalWrite(relayHot, hotOn ? LOW : HIGH);        

}

//This is the set Temperature.
BLYNK_WRITE(V6)
{
  FahrenheitSet = param.asDouble(); 
}

//This is the set variance Temperature.
BLYNK_WRITE(V7)
{
  FahrenheitSetVariance = param.asDouble(); 
}

//This is the set variance Temperature.
BLYNK_WRITE(V8)
{
  smokeSecondsOn = param.asDouble(); 
}

//This is the set variance Temperature.
BLYNK_WRITE(V9)
{
  smokeSecondsOff = param.asDouble(); 
}

//This is the exact time the HotStarted.
BLYNK_WRITE(V11)
{
  timeAugerStarted = param.asDouble(); 
}
//This is the exact time the FanStarted.
BLYNK_WRITE(V12)
{
  timeFanStarted = param.asDouble(); 
}

//This is the exact time the CoolingStarted.
BLYNK_WRITE(V13)
{
  timeCoolingStarted = param.asDouble(); 
}

//Is the Power On or off.
BLYNK_WRITE(V20)
{
  bPowerOn = param.asInt() == 0 ? false : true;   
}

//Is Manually overriden.
BLYNK_WRITE(V22)
{
  bManualOverride = param.asInt() == 0 ? false : true;  
}


