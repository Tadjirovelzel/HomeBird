#include <Arduino.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiManager.h>
#include "time.h"

// for camera:  // parts from: https://github.com/espressif/esp32-camera/blob/master/examples/main/take_picture.c and https://randomnerdtutorials.com/esp32-cam-take-photo-save-microsd-card/
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifndef portTICK_RATE_MS
#define portTICK_RATE_MS portTICK_PERIOD_MS
#endif

#include "esp_camera.h"


static void sendTime(void* arg);
String uint8_t_array_to_string_hex(uint8_t* array, int length);
String uint8_t_to_hex(uint8_t data);
void init_cam();
void clear_image_buf(camera_fb_t* pic);
camera_fb_t* take_image();
static esp_err_t init_camera();


// define the number of bytes you want to access
#define EEPROM_SIZE 1

// Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#define CONFIG_ESP32_SPIRAM_SUPPORT=y



#define INTERRUPT_INPUT_PIN GPIO_NUM_2

const char* ssid = "weatherstation";
const char* password = "weatherstation";
String serverName = "http://192.168.0.109:8000/";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

unsigned long count = 0;

WiFiManager wm;

bool camera_OK = false;
camera_fb_t *pic;


void setup() {
  Serial.begin(2000000);

  Serial2.begin(115200, SERIAL_8N1, 14, 12);
  Serial2.setTimeout(1000);

  pinMode(INTERRUPT_INPUT_PIN, INPUT_PULLUP);
  // attachInterrupt(INTERRUPT_INPUT_PIN, sendTime, RISING);
  gpio_isr_handler_add(INTERRUPT_INPUT_PIN, &sendTime, (void *) INTERRUPT_INPUT_PIN);
  gpio_set_intr_type(INTERRUPT_INPUT_PIN, GPIO_INTR_POSEDGE);

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
    Serial.printf("Failed to obtain time\n");
    return(0);
  }
  time(&now);
  return now;
}

void loop() {
  delay(10000);
  init_cam();
  if(camera_OK){
    pic = take_image();
  }

  count++;

  if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;


    String serverPath = serverName;
    String packetData = "";
    packetData += "count=" + String(count);

    if(Serial2.available() > 0){
      String data = Serial2.readStringUntil('\n');
      bool correct = true;
      for(int i = 0; i < data.length(); i++){
        if(!(((data[i] >= '0') && (data[i] <= '9')) || ((data[i] >= 'A') && (data[i] <= 'F')) || ((data[i] >= 'a') && (data[i] <= 'f')) || (data[i]) == ' ')){
          correct = false;
        }
      }
      if(correct)
        packetData += "\ndata=" + Serial2.readStringUntil('\n');
    }

    if(camera_OK){
      // packetData += "\npic_data=" + uint8_t_array_to_string_hex(pic->buf, pic->len);
      http.begin((serverPath + "/camera").c_str());
      String data = uint8_t_array_to_string_hex(pic->buf, pic->len);
      http.POST((uint8_t*)data.c_str(), data.length());
      packetData += "\npic_width=" + String(pic->width);
      packetData += "\npic_height=" + String(pic->height);
      packetData += "\npic_format=" + String(pic->format);
      clear_image_buf(pic);
    }

    struct tm timeinfo;
    time_t now;
    if(getLocalTime(&timeinfo)){
      time(&now);
      packetData += "\ntime=" + String(now);
    }
    packetData.replace(" ", "");
    packetData.replace("\r", "");

    // Your Domain name with URL path or IP address with path
    http.begin((serverPath + "/data").c_str());
    
    // Send HTTP GET request
    int httpResponseCode = http.POST((uint8_t *)packetData.c_str(), packetData.length());
    printf("%s\n", packetData.c_str());
    
    // Free resources
    http.end();
  }
}

static esp_err_t init_camera(){
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; 


  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else { //It does not have psram
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }


  //initialize the camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "Camera Init Failed");
    return err;
  }

  return ESP_OK;
}

camera_fb_t* take_image(){
  printf("Taking picture...\n");
  camera_fb_t *pic = esp_camera_fb_get();
  
  return pic;
}

void clear_image_buf(camera_fb_t* pic){
  esp_camera_fb_return(pic);
}

void init_cam(){
  if(!camera_OK){
    esp_err_t err = init_camera();
    if(err == ESP_OK){
      camera_OK = true;
    }else{
      printf("ESP camera error: %d\n", err);
    }
  }
}

static void sendTime(void* arg){
  struct tm timeinfo;
  time_t now;
  if(getLocalTime(&timeinfo)){
    time(&now);
    Serial2.printf("%s\n", String(now).c_str());
    Serial.printf("Time received: %s\n", String(now).c_str());
  }
  Serial.printf("sendTime() ran\n");
}


String uint8_t_to_hex(uint8_t data){
  return (String("0123456789ABCDEF"[data >> 4]) + String("0123456789ABCDEF"[data & 0x0F]));
}

String uint8_t_array_to_string_hex(uint8_t* array, int length){
  String s = "";
  for(int i = 0; i < length; i++){
    s += uint8_t_to_hex(*array); //write data pointer is pointing to to file
    array++; //pointer needs to point to next data address
  }
  return s;
}