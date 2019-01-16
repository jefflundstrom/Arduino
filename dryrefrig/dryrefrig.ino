
#include <TimeLib.h>

// Include the correct display library
// For a connection via I2C using Wire include
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`
// Include the UI lib
#include "OLEDDisplayUi.h"

// Include custom images
#include "images.h"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <WidgetRTC.h>
#include <Time.h>

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

DHT_Unified dht(25, DHT11);
DHT_Unified dht2(16, DHT11);
uint32_t delayMS;


// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "47aa5b1ad0234900855fbfe614f673e4";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "twodolphins";
char pass[] = "twodolphinsrock";

//For syncing the time.
WidgetRTC rtc;

// Initialize the OLED display using Wire library
SSD1306Wire  display(0x3c, 5, 4);
// SH1106 display(0x3c, D3, D5);

OLEDDisplayUi ui ( &display );

int screenW = 128;
int screenH = 64;
int clockCenterX = screenW/2;
int clockCenterY = ((screenH-16)/2)+16;   // top yellow part is 16 px height
int clockRadius = 23;

// utility function for digital clock display: prints leading 0
String twoDigits(int digits){
  if(digits < 10) {
    String i = '0'+String(digits);
    return i;
  }
  else {
    return String(digits);
  }
}

void clockOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {

}

void analogClockFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
//  ui.disableIndicator();

  // Draw the clock face
//  display->drawCircle(clockCenterX + x, clockCenterY + y, clockRadius);
  display->drawCircle(clockCenterX + x, clockCenterY + y, 2);
  //
  //hour ticks
  for( int z=0; z < 360;z= z + 30 ){
  //Begin at 0° and stop at 360°
    float angle = z ;
    angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
    int x2 = ( clockCenterX + ( sin(angle) * clockRadius ) );
    int y2 = ( clockCenterY - ( cos(angle) * clockRadius ) );
    int x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 8 ) ) ) );
    int y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 8 ) ) ) );
    display->drawLine( x2 + x , y2 + y , x3 + x , y3 + y);
  }

  // display second hand
  float angle = second() * 6 ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  int x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 5 ) ) ) );
  int y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 5 ) ) ) );
  display->drawLine( clockCenterX + x , clockCenterY + y , x3 + x , y3 + y);
  //
  // display minute hand
  angle = minute() * 6 ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 4 ) ) ) );
  y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 4 ) ) ) );
  display->drawLine( clockCenterX + x , clockCenterY + y , x3 + x , y3 + y);
  //
  // display hour hand
  angle = hour() * 30 + int( ( minute() / 12 ) * 6 )   ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 2 ) ) ) );
  y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 2 ) ) ) );
  display->drawLine( clockCenterX + x , clockCenterY + y , x3 + x , y3 + y);
}

void digitalClockFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setFont(ArialMT_Plain_16);
  display->drawString(clockCenterX + x , 10, "Time" );
  String timenow = String(hour())+":"+twoDigits(minute())+":"+twoDigits(second());
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_24);
  display->drawString(clockCenterX + x , clockCenterY + y, timenow );
}

float humidity = 0.0;
float temperature = 0;
float humidity2 = 0.0;
float temperature2 = 0;

void digitalTempFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {

  display->setFont(ArialMT_Plain_16);
  display->drawString(clockCenterX + x , 10, "Temperature" );

  String timenow = String(temperature)+"°F";
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(clockCenterX + x , 25, timenow );

  timenow = String(temperature2)+"°F";
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(clockCenterX + x , 40, timenow );
}

void digitalHumidityFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {

  display->setFont(ArialMT_Plain_16);
  display->drawString(clockCenterX + x , 10, "Humidity" );

  String timenow = String(twoDigits(humidity) + "%" );
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(clockCenterX + x , 25, timenow );

  timenow = String(twoDigits(humidity2) + "%" );
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(clockCenterX + x , 40, timenow );
}

// This array keeps function pointers to all frames
// frames are the single views that slide in
FrameCallback frames[] = { digitalClockFrame, digitalTempFrame, digitalHumidityFrame };

// how many frames are there?
int frameCount = 3;

// Overlays are statically drawn on top of a frame eg. a clock
OverlayCallback overlays[] = { clockOverlay };
int overlaysCount = 1;

void setup() {
  Serial.begin(9600);
    // Initialize device.
  dht.begin();
  dht2.begin();

  sensor_t sensor;
  dht.humidity().getSensor(&sensor);
  // Set delay between sensor readings based on sensor details.
  delayMS = sensor.min_delay / 1000;

  Blynk.begin(auth, ssid, pass);
    // Synchronize time on connection
  rtc.begin();  


	// The ESP is capable of rendering 60fps in 80Mhz mode
	// but that won't give you much time for anything else
	// run it in 160Mhz mode or just set it to 30 fps
  ui.setTargetFPS(60);

	// Customize the active and inactive symbol
  ui.setActiveSymbol(activeSymbol);
  ui.setInactiveSymbol(inactiveSymbol);

  // You can change this to
  // TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorPosition(TOP);

  // Defines where the first frame is located in the bar.
  ui.setIndicatorDirection(LEFT_RIGHT);

  // You can change the transition that is used
  // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_LEFT);

  // Add frames
  ui.setFrames(frames, frameCount);

  // Add overlays
  ui.setOverlays(overlays, overlaysCount);

  // Initialising the UI will init the display too.
  ui.init();

  display.flipScreenVertically();

  unsigned long secsSinceStart = millis();
  // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
  const unsigned long seventyYears = 2208988800UL;
  // subtract seventy years:
  unsigned long epoch = secsSinceStart - seventyYears * SECS_PER_HOUR;
  setTime(epoch);

}

double lastrun = 0;
void UpdateTempAndHumidity()
{  
    if(millis() > (lastrun + delayMS))
    {
      lastrun = millis();
      sensors_event_t event;  
      // Get temperature event and print its value.
      dht.temperature().getEvent(&event);
      if (!isnan(event.temperature)){
        temperature = (event.temperature * 1.8)+ 32;
        Blynk.virtualWrite(V2, temperature);
      }
      
      // Get temperature event and print its value.
      dht.humidity().getEvent(&event);
      if (!isnan(event.relative_humidity)) 
        humidity = event.relative_humidity;
        Blynk.virtualWrite(V3, humidity);

      
      // Get temperature event and print its value.
      dht2.temperature().getEvent(&event);
      if (!isnan(event.temperature)){
        temperature2 = (event.temperature * 1.8)+ 32;
        Blynk.virtualWrite(V4, temperature2);
      }
      
      // Get temperature event and print its value.
      dht2.humidity().getEvent(&event);
      if (!isnan(event.relative_humidity)) 
        humidity2 = event.relative_humidity;
        Blynk.virtualWrite(V5, humidity2);
    }
}

void loop() {

  Blynk.run();
  UpdateTempAndHumidity();
  
  int remainingTimeBudget = ui.update();

  if (remainingTimeBudget > 0) {
    // You can do some work here
    // Don't do stuff if you are below your
    // time budget.
    delay(remainingTimeBudget);

  }
  
}

BLYNK_CONNECTED() {

  // Request Blynk server to re-send latest values for all pins
  Blynk.syncAll();
}
