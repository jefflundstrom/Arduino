#include <SoftwareSerial.h>
SoftwareSerial RFID(1,2); //RX and TX
int i;
void setup()
{
  RFID.begin(9600);
  Serial.begin(9600);
}

void loop()
{
  if(RFID.available() > 0)
  {
   i = RFID.read();
   Serial.print(i, DEC);
   Serial.print(" ");
  }
}

