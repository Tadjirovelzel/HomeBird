#include <Arduino.h>
#include "bme280.h"
#include "Wire.h"
#include "i2c.h"

#include "SD.h"
#include "SPI.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiManager.h>

#include <PMserial.h> // Arduino library for PM sensors with serial interface

SerialPM pms(PMS5003, 25, 26);

const char* ssid = "weatherstation";
const char* password = "weatherstation";
String serverName = "http://192.168.0.109:8000/";


File file;

unsigned long count = 0;

WiFiManager wm;


// BME280 sensor module defective

void setup() {
  I2C::init();
  BME280::init();
  pms.init();;

  if (!SD.begin(4)) {
    printf("SD card initialization failed!\n");
  }


  WiFi.mode(WIFI_STA);
  bool success = wm.autoConnect(ssid, password);
  wm.setConfigPortalTimeout(60*10); //if WiFi is not found keep portal active for 10 minutes before shut down because of no clients connecting
  if(success){
    printf("Connected to WiFi!\n");
  }else{
    printf("Not able to connect to WiFi, the device will not work over WiFi\n");
  }
  pms.sleep();
}

void loop() {
  pms.wake(); //wake pms
  delay(30000); //wait for some air to flow, datasheet says 30 seconds

  count++;
  printf("---------------------\n");
  BME280::measure();
  delay(1);
  // printf("%f %f %f\n", BME280::read_temperature(), BME280::read_pressure(), BME280::read_humidity());
  double temp = BME280::read_temperature();
  double press = BME280::read_pressure();
  
  printf("temp: %.2f C\n", temp);
  printf("pressure %.2f hPa\n", press);

  if(file = SD.open("/test.csv", FILE_APPEND)){
    file.printf("%ld,", count);
    file.printf("%.2f,", temp);
    file.printf("%.2f,", press);
    file.close();
  }

  pms.read();
  if (pms){
    printf("<1.0μm: %2d μg/m3\n<2.5μm: %2d μg/m3\n<10μm: %2d μg/m3\n",
                    pms.pm01, pms.pm25, pms.pm10);
      printf(">0.3μm: %4d #/100cm3\n>0.5μm: %3d #/100cm3\n>1.0μm: %2d #/100cm3\n>2.5μm: %2d #/100cm3\n>5.0μm: %2d #/100cm3\n>10.0μm: %2d #/100cm3\n",
            pms.n0p3, pms.n0p5, pms.n1p0, pms.n2p5, pms.n5p0, pms.n10p0);

    if(file = SD.open("/test.csv", FILE_APPEND)){
      file.printf("%2d,%2d,%2d,", pms.pm01, pms.pm25, pms.pm10);
      file.printf("%3d,%2d,%2d,%2d,%2d,%2d\n", pms.n0p3, pms.n0p5, pms.n1p0, pms.n2p5, pms.n5p0, pms.n10p0);
      file.close();
    }
  }else{
    // something went wrong
    switch (pms.status)
    {
    case pms.OK: // should never come here
      break;     // included to compile without warnings
    case pms.ERROR_TIMEOUT:
      printf("%s\n", F(PMS_ERROR_TIMEOUT));
      break;
    case pms.ERROR_MSG_UNKNOWN:
      printf("%s\n", F(PMS_ERROR_MSG_UNKNOWN));
      break;
    case pms.ERROR_MSG_HEADER:
      printf("%s\n", F(PMS_ERROR_MSG_HEADER));
      break;
    case pms.ERROR_MSG_BODY:
      printf("%s\n", F(PMS_ERROR_MSG_BODY));
      break;
    case pms.ERROR_MSG_START:
      printf("%s\n", F(PMS_ERROR_MSG_START));
      break;
    case pms.ERROR_MSG_LENGTH:
      printf("%s\n", F(PMS_ERROR_MSG_LENGTH));
      break;
    case pms.ERROR_MSG_CKSUM:
      printf("%s\n", F(PMS_ERROR_MSG_CKSUM));
      break;
    case pms.ERROR_PMS_TYPE:
      printf("%s\n", F(PMS_ERROR_PMS_TYPE));
      break;
    }
  }

  if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;

    String serverPath = serverName;
    serverPath += "?count=" + String(count);
    serverPath += "&temp=" + String(temp);
    serverPath += "&press=" + String(press);
    serverPath += "&pm01=" + String(pms.pm01);
    serverPath += "&pm25=" + String(pms.pm25);
    serverPath += "&pm10=" + String(pms.pm10);
    serverPath += "&n0p3=" + String(pms.n0p3);
    serverPath += "&n0p5=" + String(pms.n0p5);
    serverPath += "&n1p0=" + String(pms.n1p0);
    serverPath += "&n2p5=" + String(pms.n2p5);
    serverPath += "&n5p0=" + String(pms.n5p0);
    serverPath += "&n10p0=" + String(pms.n10p0);

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

  pms.sleep();
  delay(30000);
}