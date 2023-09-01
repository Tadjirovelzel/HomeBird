/*
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Camera related
#include "esp_camera.h"
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "driver/rtc_io.h"

// Camera model
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
  #define SIOD_GPIO_NUM     21
  #define SIOC_GPIO_NUM     22

  #define Y9_GPIO_NUM       33
  #define Y8_GPIO_NUM       34
  #define Y7_GPIO_NUM       13
  #define Y6_GPIO_NUM       15
  #define Y5_GPIO_NUM       5
  #define Y4_GPIO_NUM       19
  #define Y3_GPIO_NUM       39
  #define Y2_GPIO_NUM       36 
  #define VSYNC_GPIO_NUM    18
  #define HREF_GPIO_NUM     23
  #define PCLK_GPIO_NUM     12

#else
  #error "Camera model not selected"
#endif

static camera_config_t camera_config = {
    .pin_pwdn = PWDN_GPIO_NUM,
    .pin_reset = RESET_GPIO_NUM,
    .pin_xclk = XCLK_GPIO_NUM,
    .pin_sccb_sda = SIOD_GPIO_NUM,
    .pin_sccb_scl = SIOC_GPIO_NUM,
    .pin_d7 = Y9_GPIO_NUM,
    .pin_d6 = Y8_GPIO_NUM,
    .pin_d5 = Y7_GPIO_NUM,
    .pin_d4 = Y6_GPIO_NUM,
    .pin_d3 = Y5_GPIO_NUM,
    .pin_d2 = Y4_GPIO_NUM,
    .pin_d1 = Y3_GPIO_NUM,
    .pin_d0 = Y2_GPIO_NUM,
    .pin_vsync = VSYNC_GPIO_NUM,
    .pin_href = HREF_GPIO_NUM,
    .pin_pclk = PCLK_GPIO_NUM,

    .xclk_freq_hz = 12000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,
    .pixel_format = PIXFORMAT_JPEG,
    //.pixel_format = PIXFORMAT_RGB565,
    .frame_size = FRAMESIZE_VGA,
    .jpeg_quality = 20,
    .fb_count = 1,
    .grab_mode = CAMERA_GRAB_LATEST // When buffers should be filled CAMERA_GRAB_WHEN_EMPTY
    //.fb_location    = CAMERA_FB_IN_PSRAM, // The location where the frame buffer will be allocated
};

// WiFi credentials
// const char ssid[] = "H369AEA4CE8";
// const char pass[] = "FA9694C7FEC3";
const char ssid[] = "Moto Lennard";
const char pass[] = "hotspotlennard";

// MQTT details
const char* broker = "excellent-engraver.cloudmqtt.com";        // Public IP address or domain name
const char* mqttUsername = "fnrfeqot";                          // MQTT username
const char* mqttPassword = "P-1Ta8Utd1Pj";                      // MQTT password
const char* topicMeasure = "device/5/measurement";              // Topic measurement
const char* topicImage = "device/5/img";                        // Topic image
const char* topicClip = "device/5/clip";                        // Topic clip

WiFiClient net;
PubSubClient client(net);

size_t cnv_buf_len;
uint8_t * cnv_buf = NULL;

void connect() {
    int i = 0;
    Serial.print("checking wifi...");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000); i++;
        if (i > 30) ESP.restart();
    }

    Serial.print("\nconnecting mqtt...");
    while (!client.connected()){
        // Create a random client ID
        String clientId = "ESP32Client-";
        clientId += String(random(0xffff), HEX);

        // Attempt to connect
        if (client.connect(clientId.c_str(), mqttUsername, mqttPassword)) {
            Serial.println("\nconnected!");
        } else Serial.print(".");

        delay(1000); i++;
        if (i > 60) ESP.restart();
    }
}

void init_camera()
{
    Serial.printf("PSRAM Total heap %d, PSRAM Free Heap %d\n",ESP.getPsramSize(),ESP.getFreePsram());
    
    // Power up the camera if PWDN pin is defined
    if(PWDN_GPIO_NUM != -1){
        pinMode(PWDN_GPIO_NUM, OUTPUT);
        digitalWrite(PWDN_GPIO_NUM, LOW);
    }
    
    // Init Camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return;
    } else {
        Serial.printf("Camera init succes\n", err);
        Serial.printf("PSRAM Total heap %d, PSRAM Free Heap %d\n",ESP.getPsramSize(),ESP.getFreePsram());
    }
}

void take_picture()
{
    camera_fb_t * pic = NULL;

    // Take Picture with Camera
    pic = esp_camera_fb_get();  
    if(!pic) {
        Serial.println("Camera capture failed");
        return;
    } else{
        Serial.println("Camera capture succesful");
        Serial.println("Format: " + String(pic->format) + "; size: " + String(pic->len));
        bool sent;

        if(pic->format != 4){
            bool isConverted = frame2jpg(pic, 60, &cnv_buf, &cnv_buf_len);
            if(isConverted){
                Serial.printf("Converted to JPEG, size = %d \n", cnv_buf_len);
            } else{
                Serial.println("Failed to convert to JPEG");
            } 
            if (!client.connected()) connect();
            sent = client.publish(topicImage, cnv_buf, cnv_buf_len);
        } else{
            if (!client.connected()) connect();
            sent = client.publish(topicImage, pic->buf, pic->len);
        }

        if (!sent){
            int err = client.state();
            Serial.printf("Upload failed with error %d \n", err);
        } else Serial.println("Upload succesfull");
    }

    free(cnv_buf); esp_camera_fb_return(pic);
}

void setup() {
    Serial.begin(115200);

    // Connect to mqtt and test connection
    WiFi.begin(ssid, pass);
    client.setServer(broker, 1883);
    connect();

    // MQTT buffer size
    Serial.printf("Buffer size: %d \n", client.getBufferSize());
    if(client.setBufferSize(65535)) Serial.printf("New buffer size set successfully: %d \n", client.getBufferSize());
    if(client.publish(topicMeasure, "{\"temperature\":1,\"humidity\":2,\"pressure\":3,\"temperature2\":4,\"humidity2\":5,\"pressure2\":6}")) Serial.println("Test data sent succesfully");

    // Initialize camera
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
    init_camera();
}

void loop() {
    client.loop();
    take_picture();
    delay(5000);
}

//*/