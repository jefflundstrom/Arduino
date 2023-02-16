/**
 * @file streams-sd-audiokit.ino
 * @author Phil Schatzmann
 * @brief Just a small demo, how to use files with the SD library
 * @version 0.1
 * @date 2022-10-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <SPI.h>
#include <SD.h>
#include "AudioTools.h"
#include "AudioLibs/AudioKit.h"
#include "AudioCodecs/CodecMP3Helix.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>

// Replace with your network credentials
const char* ssid = "twodolphins";
const char* password = "twodolphinsrock";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
DNSServer dnsServer;

const int MAXRECORDINGTIME = 30000; //30 seconds

const int hook_pin = 5;  //We will monitor GPI5 for the hook signal.
const int wifi5_pin = 22; //We will monitor GPI22 for the start wifi button for 

const int chipSelect=PIN_AUDIO_KIT_SD_CARD_CS;
AudioKitStream MP3kit; // final output of decoded stream
EncodedAudioStream decoder(&MP3kit, new MP3DecoderHelix()); // Decoding stream
EncodedAudioStream* pEncoder = nullptr;
auto wavEncoder = new WAVEncoder();
uint16_t sample_rate = 48000;
uint8_t channels = 2;  

const int MAX_BUFFER = 512;
uint8_t buf[MAX_BUFFER];
bool deleteAllFiles = false;

AudioKitStream RAWkit;
File file;   // final output stream
int recording = 0;
char file_name[255];
StreamCopy copier; 
File audioFile;

void initWiFi() {

  WiFi.mode(WIFI_STA);
  //WiFi.begin(ssid, password);
  //Serial.print("Connecting to WiFi ..");
  //while (WiFi.status() != WL_CONNECTED) {
  //  Serial.print('.');
  //  delay(1000);
  //}
  //Serial.println(WiFi.localIP());


  // Connect to Wi-Fi network with SSID and password
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP("MagicPhone");
  dnsServer.start(53, "local.com", WiFi.softAPIP());
  
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

}

const char* printRecordingFiles(String prefixLine = "", String postfixLine = "\n", String prefixColumn = " ", String postfixColumn = "")
{
  //Assume we will not find any files.
  String fileList = "";
  bool foundOne = false;
  File root = SD.open("/Recordings");
  if(root){
    //We have a directory, so clear the files.
    while (File entry =  root.openNextFile())
    {
      fileList += prefixLine;
        fileList += prefixColumn;
          fileList += "<a href=\"play?file=";
          fileList += entry.name();
          fileList += "\">";
          fileList += entry.name();
          fileList += "</a>";
        fileList += postfixColumn;
        fileList += prefixColumn;
          fileList += entry.available();
        fileList += postfixColumn;
      fileList += postfixLine;
      entry.close();
      foundOne = true;
    }
    root.close();
  }
  if(!foundOne)
  {
    fileList += prefixLine;
    fileList +="No Recordings Found!";
    fileList += postfixLine;
  }

  return fileList.c_str();
}
// handles uploads
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  
  String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
  Serial.println(logmessage);

  if (!index) {
    logmessage = "Upload Start: " + String(filename);
    // open the file on first call and store the file handle in the request object
    request->_tempFile = SD.open("/" + filename, FILE_WRITE);
    Serial.println(logmessage);
  }

  if (len) {
    // stream the incoming chunk to the opened file
    request->_tempFile.write(data, len);
    logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
    Serial.println(logmessage);
  }

  if (final) {
    logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
    // close the file handle as the upload is now done
    request->_tempFile.close();
    Serial.println(logmessage);
    request->redirect("/");
  }
}

void PutFile(AsyncWebServerRequest *request){
  String putWebPage ="<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='upload' enctype='multipart/form-data' id='upload_form'>"
    "<input type='file' name='update'>"
    "<input type='submit' value='Upload'>"
"</form>"
"<div id='prg'>progress: 0%</div>"
"<script>"
"$('form').submit(function(e){"
    "e.preventDefault();"
      "var form = $('#upload_form')[0];"
      "var data = new FormData(form);"
      " $.ajax({"
            "url: '/update',"
            "type: 'POST',"               
            "data: data,"
            "contentType: false,"                  
            "processData:false,"  
            "xhr: function() {"
                "var xhr = new window.XMLHttpRequest();"
                "xhr.upload.addEventListener('progress', function(evt) {"
                    "if (evt.lengthComputable) {"
                        "var per = evt.loaded / evt.total;"
                        "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
                    "}"
               "}, false);"
               "return xhr;"
            "},"                                
            "success:function(d, s) {"    
                "console.log('success!')"
           "},"
            "error: function (a, b, c) {"
            "}"
          "});"
"});"
"</script>";

    return request->send(500, "text/html", putWebPage.c_str());

}

void PlayFile(AsyncWebServerRequest *request){

  auto fileNameToDelete = request->getParam("file");
  String requestString = String(fileNameToDelete->value());
  Serial.println("Received Play request :" + requestString);

  String filePlay = "<!DOCTYPE HTML><html> <head> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\"><link rel=\"icon\"  type=\"image/png\" href=\"favicon.png\"><title>Magic Phone File Play file</title></head> <body><style> .left   {text-align:left;} .center {text-align:center;} .right{text-align:right;}</style><H2>Magic Phone File Play ";
  filePlay += requestString;
  filePlay += "</H2><audio controls><source src=\"/recordings/";
  filePlay += requestString;
  filePlay += "\" type=\"audio/wav\">Your browser does not support the audio tag.</audio>";
  filePlay += "</body></html>";
  
  return request->send(500, "text/html", filePlay.c_str());

}

void DeleteAllFiles(){
    File root = SD.open("/Recordings");
    if(root){
      //We have a directory, so delete the files.
      while (File entry =  root.openNextFile())
      {
        String fileName = String("/recordings/") + String(entry.name());
        entry.close();
        delay(100);
        if(SD.remove(fileName))
          Serial.print("Deleted file: ");
        else
          Serial.print("File not deleted file: ");

        Serial.println(fileName);
      }
    }

}
void DeleteFile(AsyncWebServerRequest *request){

  auto fileNameToDelete = request->getParam("file");
  String requestString = String(fileNameToDelete->value());
  //Special Case to delete ALL the files in one call.
  if(requestString.equalsIgnoreCase("all")){
    deleteAllFiles = true;
    return request->send(500, "text/html", "All Files delete Requested");
  }
  else
  {
    Serial.println("Received DELETE request :" + requestString);
    if(SD.remove("/recordings/" + requestString))
      return request->send(500, "text/html", "File was deleted");
    else
      return request->send(500, "text/html", "ERROR File was NOT deleted");
  }
}

void GetFileList(AsyncWebServerRequest *request){
  String fileList = "<!DOCTYPE HTML><html> <head> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\"><link rel=\"icon\"  type=\"image/png\" href=\"favicon.png\"><title>Magic Phone Files</title></head> <body><style> .left   {text-align:left;} .center {text-align:center;} .right{text-align:right;}</style><H2>Magic Phone Recorded Files</H2><Table> <tr> <th> File Names </th><th>File Size (KB)</th> </tr>";
  fileList += String(printRecordingFiles("<tr>", "</tr>", "<td class=\"left\">", "</td>"));
  fileList += "</table></body></html>";
  
  return request->send(500, "text/html", fileList.c_str());
}

bool wifiActive = false;

void setup(){
  Serial.begin(115200);
  //AudioLogger::instance().begin(Serial, AudioLogger::Info);  

  if (!SD.begin(chipSelect)) {
    Serial.println("Initialization failed!");
    stop();
  }
  else{
    uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE){
      Serial.println("No SD card attached");
      return;
    }
    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC){
      Serial.println("MMC");
    } else if(cardType == CARD_SD){
      Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
      Serial.println("SDHC");
    } else {
      Serial.println("UNKNOWN");
    }
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
    Serial.println("\n***********READY TO RECORD!*************");
  }
  pinMode(hook_pin, INPUT);
  pinMode(wifi5_pin, INPUT);

}
enum states {
    ONHOOK, WAITING, WELCOME,WELCOME_PLAYING, BEEP, BEEP_PLAYING, RECORD, RECORDING, TIMEOUT, TIMEOUTBEEP   };

states currentState = ONHOOK;
int waitTicks = 0;

void EndRecording(){

      if(recording > 0){
        RAWkit.end();
        file.close(); 
        pEncoder->end();
        file = SD.open(file_name, FILE_READ);
        if (file) 
        {
          int avail_len = file.available();
          Serial.print("Converting recorded file ");
          Serial.print(file_name);
          Serial.print(" ");
          Serial.print((millis() - recording)/1000);
          Serial.print(" seconds to a wav file of size =");
          Serial.println(avail_len);
        }

        //Free the allocated memory    
        free(pEncoder);
        pEncoder = nullptr;

        recording = 0;
    }
}

void EndAllStreams(){
        //the action is complete, move to the next.
      if(currentState == WELCOME_PLAYING)
      {
          currentState = BEEP;
          decoder.end();
          MP3kit.end();
      }
      else if(currentState == BEEP_PLAYING)
      {
          currentState = RECORD;
          decoder.end();
          MP3kit.end();
      }
      else if(currentState == TIMEOUTBEEP)
      {
          currentState = TIMEOUT;
          decoder.end();
          MP3kit.end();
      }
      else if(recording > 0){
        //If we were recording, end that stream.
        EndRecording();
      }
      //End any copying as the streams have stopped.
      copier.end();
}

void PlayBeep(){
      audioFile = SD.open("/BEEP.mp3");

      // setup audiokit
      auto config = MP3kit.defaultConfig(TX_MODE);
      config.sd_active = true;
      MP3kit.setVolume(50);
      MP3kit.begin(config);

      // setup I2S based on sampling rate provided by decoder
      decoder.begin();

      // begin copy
      copier.setCheckAvailableForWrite(false);
      copier.begin(decoder, audioFile);
}

void loop(){
  
  if(wifiActive)
    dnsServer.processNextRequest();

  int hookState = digitalRead(hook_pin);

  if(hookState == 0 && currentState != ONHOOK && currentState != WAITING)
  {
    Serial.println("Setting ONHOOK");
    currentState = ONHOOK;
    EndAllStreams();
  }
  else if(hookState == 1)
  {
    if(currentState == ONHOOK){
      Serial.println("Waiting to play announcement");
      //We just went off hook, we now need to wait for the user to put the phone to their ear.
      currentState = WAITING;
      waitTicks = millis();
    }
    else if (currentState == WAITING){
      if(millis()-waitTicks > 1000)
        currentState = WELCOME;
    }
   else if(currentState == WELCOME){
      audioFile = SD.open("/WELCOME.mp3");

      // setup audiokit before SD!
      auto config = MP3kit.defaultConfig(TX_MODE);
      config.sd_active = true;
      MP3kit.setVolume(100);
      MP3kit.begin(config);

      // setup I2S based on sampling rate provided by decoder
      decoder.begin();

      // begin copy
      copier.setCheckAvailableForWrite(false);
      copier.begin(decoder, audioFile);

      currentState = WELCOME_PLAYING;
   }
   else if(currentState == BEEP){
      PlayBeep();
      currentState = BEEP_PLAYING;
   }
   else if (currentState == RECORD){
      currentState = RECORDING;

      //Configure the source.
      auto cfgIn = RAWkit.defaultConfig(RXTX_MODE);
      cfgIn.sd_active = true;
      cfgIn.sample_rate = sample_rate;
      cfgIn.channels = channels;
      cfgIn.input_device = AUDIO_HAL_ADC_INPUT_LINE2;
      RAWkit.setVolume(1.0);
      RAWkit.begin(cfgIn);

      //Configure to destination.
      //Generate a unique name so we never overwrite.
      sprintf(file_name, "/Recordings/File%04u.wav", millis());
      file = SD.open(file_name, FILE_WRITE);
      
      pEncoder = new EncodedAudioStream(&file, wavEncoder);

      auto cfgOut = pEncoder->defaultConfig();
      cfgOut.sample_rate = sample_rate;
      cfgOut.channels = channels;
      pEncoder->begin(cfgOut);

      copier.setCheckAvailableForWrite(false);
      copier.begin(*pEncoder, RAWkit);  
      recording = millis(); 
    }
    else if (currentState == TIMEOUT){
      if(recording > 0){
        EndAllStreams();
        PlayBeep();
      }
    }

    //We only do copying of the source to destination when the hook is off the handset.
    bool successful = copier.copy();

    //Check to see if we are over time for recording, if so, we will timeout.
    if ((recording > 0) && ((millis() - recording) > MAXRECORDINGTIME))
        currentState = TIMEOUT;

    if(!successful){
      EndAllStreams();
    }
  }
  else if (!wifiActive)
  {
    if(digitalRead(wifi5_pin) == 1)
    {
      Serial.println("Wifi starting");
      wifiActive = true;
      initWiFi();

      server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        GetFileList(request);
      });
      
      server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200);
      }, handleUpload);

      server.on("/recs", HTTP_GET, GetFileList);
      server.on("/changewelcome", HTTP_GET, PutFile);
      server.on("/deletefile", HTTP_GET, DeleteFile);
      server.on("/play", HTTP_GET, PlayFile);

      server.serveStatic("/", SD, "/");

      server.begin();
      delay(1000);
    }
 }
 else
 {
   if(deleteAllFiles)
   {
     deleteAllFiles = false;
     DeleteAllFiles();
   }
 }
}