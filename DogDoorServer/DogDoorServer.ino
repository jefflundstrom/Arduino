#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* ssid = "twodolphins";
const char* password = "twodolphinsrock";

ESP8266WebServer server(80);
const int forwardMotor = 5;
const int backwardMotor = 4;
const int limitSwitchUp = 12;
const int limitSwitchDown = 13;
// 0 = stopped, 1 = open, -1 = close
int motorMovement = 0;

void setup(void){
  
  //Motor right
  pinMode(forwardMotor, OUTPUT);
  //Motor left
  pinMode(backwardMotor, OUTPUT);
  //Limit Switch
  pinMode(limitSwitchUp, INPUT);
  pinMode(limitSwitchDown, INPUT);

  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });
  server.on("/open", [](){
    motorMovement = 1;
    String message = "OPENING - UP LIMIT =";
    if ( HIGH == digitalRead(limitSwitchUp)) 
      message += "HIGH";
    else
      message += "LOW";
    message += " Down LIMIT =";
    if( HIGH == digitalRead(limitSwitchDown))
      message +=  "HIGH";
    else
      message += "LOW";
      
    //digitalWrite(forwardMotor, HIGH);
    server.send(200, "text/plain", message);
  });
  
  server.on("/close", [](){
    motorMovement = -1;
    String message = "CLOSING - UP LIMIT =";
    if ( HIGH == digitalRead(limitSwitchUp)) 
      message += "HIGH";
    else
      message += "LOW";
    message += " Down LIMIT =";
    if( HIGH == digitalRead(limitSwitchDown))
      message +=  "HIGH";
    else
      message += "LOW";
    server.send(200, "text/plain", message);
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}
void handleRoot() {
  server.send(200, "text/plain", "hello from esp8266!");
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void loop(void){
  server.handleClient();
  int forwardCommand = LOW;
  int backwardCommand = LOW;


  //If we are not stopped, see if we need to stop.
  if(motorMovement != 0)
  {
    //If we are moving up and the limit switch is not hit, move up
    if(motorMovement == 1 && LOW == digitalRead(limitSwitchUp) )
      forwardCommand = HIGH;
    //If we are moving down and the limit switch is not hit, move down
    else if (motorMovement == -1 && LOW == digitalRead(limitSwitchDown) )
      backwardCommand = HIGH;
    //WE are stopped if we are on a limit.
    else 
      motorMovement = 0;    
  }

  //See if we need to change.
  if( forwardCommand != digitalRead(forwardMotor))
    digitalWrite(forwardMotor, forwardCommand);
  if( backwardCommand != digitalRead(backwardMotor))
    digitalWrite(backwardMotor, backwardCommand);
  
}
