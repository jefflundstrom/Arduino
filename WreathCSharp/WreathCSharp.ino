#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 12
//#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "18c55bef4c284348a47a9ea5df4a98cd";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "twodolphins";
char pass[] = "twodolphinsrock";

int numPixels = 100;

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(numPixels, PIN, NEO_RGB + NEO_KHZ800);

//////////////////////////////////////////////////////////////////////////////
//CLASSES
//////////////////////////////////////////////////////////////////////////////
class _Colors
{
  public:
    uint32_t Black = strip.Color(0, 0, 0);
    uint32_t White = strip.Color(127, 127, 127);
    uint32_t Red = strip.Color(127, 0, 0);
    uint32_t Green = strip.Color(0, 127, 0);
    uint32_t Blue = strip.Color(0, 0, 127);
};

_Colors Colors;


//////////////////////////////////////////////////////////////////////////////
// Utility functions
//////////////////////////////////////////////////////////////////////////////
int masterOffset = 25;
int OffsetPixels(int pixel)
{
    return ((pixel - masterOffset) + numPixels) % numPixels;
}

/// <summary>
/// Sets a single pixel color.
/// </summary>
/// <param name="i"></param>
/// <param name="color"></param>
void SetPixelColor(int i, uint32_t color)
{
    strip.setPixelColor(OffsetPixels(i), color);
}

void ShowPixels(int wait)
{
  strip.show();
  delay(wait);
}


//////////////////////////////////////////////////////////////////////////////
// BLYNK READ AND WRITES
//////////////////////////////////////////////////////////////////////////////
int scheme = 0;
BLYNK_WRITE(V1) //Button Widget is writing to pin V1
{
  scheme = param.asInt(); 
  Blynk.virtualWrite(2, scheme);

  //Clear our lights.
  for(int i = 0; i < strip.numPixels(); i++)
      strip.setPixelColor(i, 0);
  strip.show();

}

BLYNK_READ(V6) // Widget in the app READs Virtal Pin V5 with the certain frequency
{
  // This command writes Arduino's uptime in seconds to Virtual Pin V5
  Blynk.virtualWrite(6, millis() / 1000);
}


//////////////////////////////////////////////////////////////////////////////
// The setup
//////////////////////////////////////////////////////////////////////////////
void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code


  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}


//////////////////////////////////////////////////////////////////////////////
// The main loop
//////////////////////////////////////////////////////////////////////////////
void loop() {

  //If you want Blynk to update you ahve to run it here in the main loop
  //that means that you cant do long processes below.
  Blynk.run();

  //Depending on the scheme, we will do that step, each scheme has a step that it needs to take.
  switch(scheme)
  { 
      case 0:
      Blynk.virtualWrite(5, "Random twinkle");
      Serial.println("Random twinkle");
      twinkle(300); // Random
      break;

      case 1: 
      //Shut the lights off.
      Blynk.virtualWrite(5, "OFF");
      Serial.println("OFF");
      colorWipe(Colors.Black, 5);
      break;

      case 2:
      Blynk.virtualWrite(5, "Red Wipe");
      Serial.println("Red Wipe");
      colorWipe(Colors.Red, 50); // Red
      break;      

      case 3: 
      Blynk.virtualWrite(5, "Green Wipe");
      Serial.println("Green Wipe");
      colorWipe(Colors.Green, 50); // Green
      break;      
      
      case 4: 
      Blynk.virtualWrite(5, "Blue Wipe");
      Serial.println("Blue Wipe");
      colorWipe(Colors.Blue, 50); // Blue
      break;
            
      case 5: 
      Blynk.virtualWrite(5, "Theater White");
      Serial.println("Theater White");
      theaterChase(Colors.White, 50); // White
      break;
            
      case 6: 
      Blynk.virtualWrite(5, "Theater Red");
      Serial.println("Theater Red");
      theaterChase(Colors.Red, 50); // Red
      break;
            
      case 7: 
      Blynk.virtualWrite(5, "Theater Blue");
      Serial.println("Theater Blue");
      theaterChase(Colors.Blue, 50); // Blue
      break;
            
      case 8: 
      Blynk.virtualWrite(5, "Rainbow");
      Serial.println("Rainbow");
      rainbow(20);
      break;

      case 9: 
      Blynk.virtualWrite(5, "Rainbow in whole");
      Serial.println("Rainbow in whole");
      rainbow2(false, 20);
      break;

      case 10: 
      Blynk.virtualWrite(5, "Rainbow in full");
      Serial.println("Rainbow in full");
      rainbow2(true, 20);
      break;

      case 11: 
      Blynk.virtualWrite(5, "Theater Chase Rainbow");
      Serial.println("Theater Chase Rainbow");
      theaterChaseRainbow(50);
      break;

      case 12: 
      Blynk.virtualWrite(5, "Theater Chase Random");
      Serial.println("Theater Chase Random");
      theaterChaserandom(70);
      break;

      case 13: 
      Blynk.virtualWrite(5, "Alt Blocks 5");
      Serial.println("Alt Blocks 5");
      altBlocks(Colors.Red, Colors.Green, 5, 500, 0);
      break;

      case 14: 
      Blynk.virtualWrite(5, "Alt Blocks 50");
      Serial.println("Alt Blocks 50");
      altBlocks(Colors.Red, Colors.Green, 50, 500, 5);
      break;

      case 15: 
      Blynk.virtualWrite(5, "Level Fill Top");
      Serial.println("Level Fill Top");
      LevelFill(Colors.Green, Colors.White, true, 50);
      break;

      case 16: 
      Blynk.virtualWrite(5, "Level Fill Bottom");
      Serial.println("Level Fill Bottom");
      LevelFill(Colors.Green, Colors.White, false, 50);
      break;

      case 17: 
      Blynk.virtualWrite(5, "Level Fill Both");
      Serial.println("Level Fill Both");
      LevelFillBoth(Colors.Green, Colors.White, 50);
      break;

      case 18: 
      Blynk.virtualWrite(5, "Color Wipe");
      Serial.println("Color Wipe");
      colorWipe(50);
      break;

      case 19: 
      Blynk.virtualWrite(5, "Round the clock");
      Serial.println("Round the clock");
      RoundTheClock(Colors.White, 0, 500, true, 2, 5);
      break;

      default:
      Blynk.virtualWrite(5, "Rainbow Cycle");
      Serial.println("Rainbow Cycle");
      rainbowCycle(20);
      break;
  }
}

//////////////////////////////////////////////////////////////////////////////
// The functions.
//////////////////////////////////////////////////////////////////////////////
//

int level = 0;
/// <summary>
/// Fills the bottom up to the top.
/// </summary>
/// <param name="c1"></param>
/// <param name="c2"></param>
/// <param name="wait"></param>
void LevelFillBoth(uint32_t c1, uint32_t c2, int wait)
{
    for (int i = 0; i < numPixels; i++)
        SetPixelColor(i, (i < level || i > (numPixels - level)) || (i < (numPixels / 2 - level) || i > (numPixels / 2 + level)) ? c1 : c2);

    ShowPixels(wait);
    
    level = level + 1;
    if (level > numPixels/2)
        level = 0;
}

/// <summary>
/// Fills the bottom up to the top.
/// </summary>
/// <param name="c1"></param>
/// <param name="c2"></param>
/// <param name="wait"></param>
void LevelFill(uint32_t c1, uint32_t c2, bool top, int wait)
{
    for (int i = 0; i < numPixels; i++)
        if(top)
            SetPixelColor(i, ((i < level) || (i > numPixels - level)) ? c1 : c2);
        else
            SetPixelColor(i, (i < (numPixels / 2 - level) || i > (numPixels / 2 + level)) ? c1 : c2);

    ShowPixels(wait);
 
    level++;
    if (level > numPixels/2)
        level = 0;
}

int currentSpin = 0;
bool alternate = true;
//Alternate colors in blocks of 5
void altBlocks(uint32_t c1, uint32_t c2, int nBlocks, int wait, int spinRate)
{
    currentSpin += spinRate;
    if (currentSpin > numPixels)
        currentSpin = 0;

    for (int i = 0; i < numPixels; i++)
    {
        int lightNumber = ((i + currentSpin) + numPixels) % numPixels;

        if(alternate)
            SetPixelColor(lightNumber, (i % (nBlocks * 2) < nBlocks) ? c1 : c2);
        else
            SetPixelColor(lightNumber, (i % (nBlocks * 2) < nBlocks) ? c2 : c1);
    }

    ShowPixels(wait);
    alternate = !alternate;

}

int nextClockItem = 0;

// Fill the dots one after the other with a color
void RoundTheClock(uint32_t onColor, uint32_t offColor, int wait, bool forward, int numLights, int advance)
{
    for (int i = 0; i < numPixels; i++)
        SetPixelColor(i, (i >= nextClockItem && i < nextClockItem + numLights) ? onColor : offColor);

    ShowPixels(wait);

    nextClockItem = nextClockItem + (forward ? advance : -1 *advance);

    if (forward && nextClockItem + 1 >= numPixels)
        nextClockItem = 0;
    if (!forward && nextClockItem < 0)
        nextClockItem = numPixels-advance;

}

// Fill the dots one after the other with a color
void twinkle(int wait)
{
    for (int i = 0; i < numPixels; i++)
        SetPixelColor(i, random(100) > 95 ? Colors.Black : Colors.White);

    ShowPixels(wait);
}

int lastColorWipe = 0;

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, int wait)
{
    SetPixelColor(lastColorWipe, c);
    ShowPixels(wait);

    lastColorWipe++;
    if (lastColorWipe >= numPixels)
    {
        //Clear our lights.
        for (int i = 0; i < numPixels; i++)
            SetPixelColor(i, 0);
        ShowPixels(0);

        //Start over.
        lastColorWipe = 0;
    }
}

// Fill the dots one after the other with a color
void colorWipe(int wait)
{
    SetPixelColor(lastColorWipe,
        strip.Color((byte) random(255), (byte) random(255), (byte) random(255)));
    ShowPixels(wait);

    lastColorWipe++;
    if (lastColorWipe >= numPixels)
        //Start over.
        lastColorWipe = 0;
}

int lastRainbow = 0;
void rainbow(int wait)
{
    int i;

    for (i = 0; i < numPixels; i++)
        SetPixelColor(i, Wheel((i + lastRainbow) & 255));

    ShowPixels(wait);

    if (lastRainbow++ > 256)
        lastRainbow = 0;

}

/// <summary>
/// Using a different color generator
/// </summary>
/// <param name="wait"></param>
void rainbow2(bool full, int wait)
{
    for (int i = 0; i < numPixels; i++)
        SetPixelColor(i, ColorWheel(numPixels, full ? lastRainbow : (i + lastRainbow) % numPixels));

    ShowPixels(wait);

    if (lastRainbow++ > numPixels)
        lastRainbow = 0;

}

int lastRainbowCycle = 0;
// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(int wait)
{
    int i, j;

    for (i = 0; i < numPixels; i++)
        SetPixelColor(i, Wheel(((i * 256 / numPixels) + lastRainbowCycle) & 255));

    ShowPixels(wait);

    if (lastRainbowCycle++ > 256 * 5)
        lastRainbowCycle = 0;

}

int qItems = 0;
//Theatre-style crawling lights.
void theaterChase(uint32_t c, int wait)
{
    for (int i = 0; i < numPixels-3; i = i + 3)
        SetPixelColor(i + qItems, c);    //turn every third pixel on

    ShowPixels(wait);

    for (int i = 0; i < numPixels-3; i = i + 3)
        SetPixelColor(i + qItems, 0);        //turn every third pixel off

    if (qItems++ >= 3)
        qItems = 0;


}

int lastJChaseRainbow = 0;
int lastQChaseRainbow = 0;
//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(int wait)
{
    for (int i = 0; i < numPixels-3; i = i + 3)
        SetPixelColor(i + lastQChaseRainbow, Wheel((i + lastJChaseRainbow) % 255));    //turn every third pixel on

    ShowPixels(wait);

    for (int i = 0; i < numPixels-3; i = i + 3)
        SetPixelColor(i + lastQChaseRainbow, 0);        //turn every third pixel off

    if (lastQChaseRainbow++ >= 3)
    {
        lastJChaseRainbow++;
        lastQChaseRainbow = 0;
    }

    if (lastJChaseRainbow >= 256)
        lastJChaseRainbow = 0;
}

int lastQrandomChase = 0;

//Theatre-style crawling lights with rainbow effect
void theaterChaserandom(int wait)
{
    for (int i = 0; i < numPixels-3;i = i + 3)
        SetPixelColor(i + lastQrandomChase, strip.Color((byte)random(256), (byte)random(256), (byte)random(256)));    //turn every third pixel on


    ShowPixels(wait);

    for (int i = 0; i < numPixels-3;i = i + 3)
        SetPixelColor(i + lastQrandomChase, 0);        //turn every third pixel off

    if (lastQrandomChase++ >= 3)
        lastQrandomChase = 0;
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(int WheelPosIn)
{
    byte WheelPos = (byte) WheelPosIn;
    WheelPos = (byte)(255 - WheelPos);
    if (WheelPos < 85)
        return strip.Color((byte)(255 - (WheelPos * 3)), 0, (byte)(WheelPos * 3));

    if (WheelPos < 170)
    {
        WheelPos -= 85;
        return strip.Color(0, (byte)(WheelPos * 3), (byte)(255 - WheelPos * 3));
    }

    WheelPos -= 170;

    return strip.Color((byte)(WheelPos * 3), (byte)(255 - WheelPos * 3), 0);
}



//Gets the color based on the count of colors (number of bulbs and the bulb number)
/// <summary>
/// Color generator for the full range of colors based with numberOfPixels as the N
/// </summary>
uint32_t ColorWheel(int numberOfPixels, int wheelPos)
{
    double frequency = 5.0/(double)numberOfPixels;
    uint32_t returnColor =  strip.Color(
        (byte) (int)((sin(frequency  * wheelPos + 0) * 127 + 128)),
        (byte) (int)((sin(frequency  * wheelPos + 2) * 127 + 128)),
        (byte) (int)((sin(frequency  * wheelPos + 4) * 127 + 128)));

    return returnColor;
}


