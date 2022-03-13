#include <Arduino.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiManager.h>
#include "time.h"

#define INTERRUPT_INPUT_PIN 32

const char* ssid = "weatherstation";
const char* password = "weatherstation";
String serverName = "http://192.168.0.109:8000/";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

unsigned long count = 0;

WiFiManager wm;

void sendTime();

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(1000);

  attachInterrupt(INTERRUPT_INPUT_PIN, sendTime, RISING);

  WiFi.mode(WIFI_STA);
  bool success = wm.autoConnect(ssid, password);
  wm.setConfigPortalTimeout(60*10); //if WiFi is not found keep portal active for 10 minutes before shut down because of no clients connecting
  if(success){
    printf("Connected to WiFi!\n");
  }else{
    printf("Not able to connect to WiFi, the device will not work over WiFi\n");
  }

  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

void loop() {
  delay(10000);

  count++;

  if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;


    String serverPath = serverName;
    serverPath += "?count=" + String(count);

    if(Serial.available() > 0){
      serverPath += "?data=" + Serial.readStringUntil('\n');
    }

    struct tm timeinfo;
    time_t now;
    if(getLocalTime(&timeinfo)){
      time(&now);
      serverPath += "?time=" + String(now);
    }
    serverPath.replace(" ", "");
    serverPath.replace("\r", "");

    // Your Domain name with URL path or IP address with path
    http.begin(serverPath.c_str());
    
    // Send HTTP GET request
    int httpResponseCode = http.GET();
    
    // if (httpResponseCode>0) {
    //   Serial.print("HTTP Response code: ");
    //   Serial.println(httpResponseCode);
    //   String payload = http.getString();
    //   Serial.println(payload);
    // }
    // else {
    //   Serial.print("Error code: ");
    //   Serial.println(httpResponseCode);
    // }
    // Free resources
    http.end();
  }
}

void sendTime(){
  struct tm timeinfo;
  time_t now;
  if(getLocalTime(&timeinfo)){
    time(&now);
    Serial.println(now);
  }
}