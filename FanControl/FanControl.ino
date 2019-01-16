
include <PWM.h>
int32_t frequency = 25000; 
void setup() {

  InitTimersSafe();
  bool success = SetPinFrequencySafe(9, frequency);
  
  // put your setup code here, to run once:

  pinMode(3,OUTPUT);

  Serial.begin(9600);

}

int fanSpeed = 0;
void loop() {
  // put your main code here, to run repeatedly:
  analogWrite(3, fanSpeed);

  Serial.print("commanded fan speed = ");
  Serial.println(fanSpeed);

  //fanSpeed -= 50;
  //if(fanSpeed < 0)
    //fanSpeed = 250;


  delay(10000);
}
