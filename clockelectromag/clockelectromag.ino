int clockA=2;
int clockB=3;

int tickPin = clockA;

void setup() {

  pinMode(clockA, OUTPUT);
  pinMode(clockB, OUTPUT);

  digitalWrite(clockA, LOW);
  digitalWrite(clockB, LOW);

  Serial.begin(9600);
}

void doTick()
{
  Serial.write("Tick");
  digitalWrite(tickPin, HIGH);
  delay(10);
  digitalWrite(tickPin, LOW);

  if(tickPin == clockA)
    tickPin = clockB;
  else
    tickPin = clockA;
}

void loop() {

   unsigned long startTime = millis();
   unsigned long temp;

   while(true)
   {
      startTime += 1000;

      while (startTime - millis() > 0){}

      doTick();
   }
}
