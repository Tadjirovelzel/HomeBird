//logging
#ifdef CORE_DEBUG_LEVEL
#undef CORE_DEBUG_LEVEL
#endif

#define CORE_DEBUG_LEVEL 5
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp32-hal-log.h"

//*
#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <MQTT.h>

// Camera related
#include "esp_camera.h"
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "driver/rtc_io.h"

// camera model
#define CAMERA_MODEL_AI_THINKER_MODIFIED

// Pin definition for camera models
#if defined(CAMERA_MODEL_AI_THINKER)
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

#elif defined(CAMERA_MODEL_AI_THINKER_MODIFIED)
  #define PWDN_GPIO_NUM     32
  #define RESET_GPIO_NUM    -1
  #define XCLK_GPIO_NUM      0
  #define SIOD_GPIO_NUM     26
  #define SIOC_GPIO_NUM     27

  #define Y9_GPIO_NUM       33
  #define Y8_GPIO_NUM       34
  #define Y7_GPIO_NUM       13//13 or 39 for special board
  #define Y6_GPIO_NUM       15//15 or 36 for special board
  #define Y5_GPIO_NUM       21
  #define Y4_GPIO_NUM       19
  #define Y3_GPIO_NUM       18
  #define Y2_GPIO_NUM        5
  #define VSYNC_GPIO_NUM    25
  #define HREF_GPIO_NUM     23
  #define PCLK_GPIO_NUM     22

#else
  #error "Camera model not selected"
#endif

// WiFi parameters
const char* ssid = "dinges";
const char* pass = "dinges11";

// MQTT parameters
#define MQTT_USER "fnrfeqot"
#define MQTT_PASSWORD "P-1Ta8Utd1Pj"
#define MQTT_SERIAL_PUBLISH_CH "device/5/clip"
#define MQTT_CLIENT_ID "32DrhEK#7LQNk"
const char* mqtt_server = "excellent-engraver.cloudmqtt.com";

WiFiClientSecure net;
MQTTClient client;

void connect() 
{
  Serial.println("checking wifi...");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) 
  {
      Serial.print(".");
      delay(100);
  }

  Serial.print("\nconnecting...");
  net.setInsecure();
  Serial.println("test");
  
  while (!client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD))
  {
      Serial.print(".");
      delay(100);
  }

  Serial.println("\nconnected!");
}

void init_camera()
{
  // Camera pins
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
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 24000000;
  config.pixel_format = PIXFORMAT_RGB565;
  config.fb_location    = CAMERA_FB_IN_PSRAM; /*!< The location where the frame buffer will be allocated */
  config.grab_mode      = CAMERA_GRAB_LATEST;  /*!< When buffers should be filled */
  
  Serial.printf("PSRAM Total heap %d, PSRAM Free Heap %d\n",ESP.getPsramSize(),ESP.getFreePsram());
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    // config.jpeg_quality = 10;
    config.fb_count = 1;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    // config.jpeg_quality = 12;
    config.fb_count = 1;
    }
  
  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return;
  } else {
    Serial.printf("Camera init succes\n", err);
  }
}

void take_picture()
{
  camera_fb_t * pic = NULL;

  // Take Picture with Camera
  pic = esp_camera_fb_get();

  if(pic) {
    Serial.println("Camera capture succeeded");
    Serial.printf("Picture taken! Its size was: %zu bytes\n", pic->len);
  } else {
    Serial.println("Camera capture failed");
    //init_camera();
    return;
  }
  
  delay(2000);
  if (!client.connected()) connect();
  client.publish(MQTT_SERIAL_PUBLISH_CH, (const char *)pic->buf, pic->len);
  esp_camera_fb_return(pic);
}

void setup() {
  esp_log_level_set("*", ESP_LOG_DEBUG);
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

  Serial.begin(115200);
  Serial.setDebugOutput(true);

  init_camera();
}

void loop() 
{
  take_picture();
  delay(5000);
}
//*/