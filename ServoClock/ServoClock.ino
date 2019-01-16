#include <Servo.h>
Servo myServo;
void setup() 
{
  // put your setup code here, to run once:
  myServo.attach(9);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  for(int i =0; i< 300; i+=5)
    {
      myServo.write(i);
      delay(60);
    }
}
