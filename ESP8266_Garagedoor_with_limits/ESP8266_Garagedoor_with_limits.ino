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

//Garage door
// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "4b761ac9af1d46079ba89375db8669b0";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "twodolphins";
char pass[] = "twodolphinsrock";

#define PINRELAY 12


void setup()
{
  pinMode(PINRELAY, OUTPUT);
  digitalWrite(PINRELAY, HIGH);

  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);

  
}

void loop()
{
  Blynk.run();
}


BLYNK_WRITE(V1) //Button Widget is writing to pin V1
{
 if(param.asInt() != 0)
 { 
    Blynk.virtualWrite(2, "Opening");
  
    digitalWrite(PINRELAY, LOW);
    delay(750);  
    digitalWrite(PINRELAY, HIGH);

    Blynk.virtualWrite(2, "Opened");
 }
}

