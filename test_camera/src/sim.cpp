//*
#include <Arduino.h>

// SIM model
#define TINY_GSM_MODEM_SIM7600

// Set serial for debug console, AT commands and debug prints
#define SerialMon Serial
#define SerialAT Serial1
#define TINY_GSM_DEBUG SerialMon
#define TINY_GSM_YIELD_MS 2

// Set TinyGsm en MQTT
#include <TinyGsmClient.h>
#include <PubSubClient.h>
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
PubSubClient mqtt(client);

// set GSM PIN and apn details
#define GSM_PIN "0000"

const char* apn = "m2m.tele2.com";
const char* gprsUser = "";
const char* gprsPass = "";

// MQTT details
const char* broker = "excellent-engraver.cloudmqtt.com"; // Public IP address or domain name
const char* mqttUsername = "fnrfeqot";                   // MQTT username
const char* mqttPassword = "P-1Ta8Utd1Pj";               // MQTT password
const char* topicMeasure = "device/5/measurement";       // topic measurement data
const char* topicImage = "device/5/img";                 // topic images

// Modem pins
#define uS_TO_S_FACTOR          1000000ULL  // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP           60          // Time ESP32 will go to sleep (in seconds) 

#define PIN_TX                  27
#define PIN_RX                  26
#define UART_BAUD               115200
#define PWR_PIN                 4
#define LED_PIN                 12
#define POWER_PIN               25

bool reply = false;
uint32_t lastReconnectAttempt = 0;
size_t cnv_buf_len;
uint8_t * cnv_buf = NULL;

// Camera related
#include "esp_camera.h"
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "driver/rtc_io.h"

int cam_pins[] = {35, 34, 39, 36, 21, 19, 18, 5, 22, 23, 26, 27};

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
  #define SIOD_GPIO_NUM     26
  #define SIOC_GPIO_NUM     27

  #define Y9_GPIO_NUM       33
  #define Y8_GPIO_NUM       34
  #define Y7_GPIO_NUM       13
  #define Y6_GPIO_NUM       15
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
    //.pixel_format = PIXFORMAT_JPEG,
    .pixel_format = PIXFORMAT_RGB565,
    .frame_size = FRAMESIZE_VGA,
    .jpeg_quality = 20,
    .fb_count = 1,
    .grab_mode = CAMERA_GRAB_LATEST // When buffers should be filled CAMERA_GRAB_WHEN_EMPTY
    //.fb_location    = CAMERA_FB_IN_PSRAM, // The location where the frame buffer will be allocated
};

void modem_on()
{
    // Disable camera
    // for(int i=0; i < 12; i++){
    //     digitalWrite(i, LOW);
    //     digitalWrite(32, HIGH);
    // }
    
    // The time of active low level impulse of PWRKEY pin to power on module, typically 500 ms
    Serial.println("\nStarting Up Modem...");
    digitalWrite(LED_PIN, LOW);   
    digitalWrite(POWER_PIN, HIGH);
    digitalWrite(PWR_PIN, HIGH);
    delay(500);
    digitalWrite(PWR_PIN, LOW);
    delay(5000);
    
    Serial.println("\nTesting Modem Response...\n");
    Serial.println("****");
    for(int i=0; i < 10; i++){
        SerialAT.println("AT");
        delay(500);
        if (SerialAT.available()) {
            String r = SerialAT.readString();
            Serial.println(r);
            if ( r.indexOf("OK") >= 0 ) {
                reply = true;
                break;
            }
        }
        delay(500);
    }
    Serial.println("****\n");
}

void initModem(){
    DBG("Wait...");
    int retry = 5;
    while (!reply && retry--){
        modem_on();
    }

    if (reply) {
        Serial.println(F("***********************************************************"));
        Serial.println(F(" You can now send AT commands"));
        Serial.println(F(" Enter \"AT\" (without quotes), and you should see \"OK\""));
        Serial.println(F(" If it doesn't work, select \"Both NL & CR\" in Serial Monitor"));
        Serial.println(F(" DISCLAIMER: Entering AT commands without knowing what they do"));
        Serial.println(F(" can have undesired consiquinces..."));
        Serial.println(F("***********************************************************\n"));
    } else {
        Serial.println(F("***********************************************************"));
        Serial.println(F(" Failed to connect to the modem! Check the baud and try again."));
        Serial.println(F("***********************************************************\n"));
        return;
    }

    // Modem info
    String ret;
    ret = modem.setNetworkMode(38);
    DBG("setNetworkMode:", ret);

    String name = modem.getModemName();
    DBG("Modem Name:", name);

    String modemInfo = modem.getModemInfo();
    DBG("Modem Info:", modemInfo);    
}


void connectGPRS(){
    // Check if the modem is active or not.
    if (modem.testAT()) {
        Serial.println("Modem is active.");
    } else {
        Serial.println("Modem is not active.");
        if(!modem.restart()){
            Serial.println("Restart failed");
            return;
        }
    }

    //     // Check if the modem is active or not.
    //     if (modem.testAT()) {
    //       Serial.println("Modem is active.");
    //     } else {
    //       Serial.println("Modem is not active.");
    //       initModem();
    //     }   

    // Unlock SIM card with a PIN if needed
    if (GSM_PIN && modem.getSimStatus() != 3) modem.simUnlock(GSM_PIN);

    // Registrate network
    Serial.print("Waiting for network...");
    if (!modem.waitForNetwork()) {
        Serial.println("fail");
        delay(10000);
        return;
    }
    Serial.println("Success!");

    if (modem.isNetworkConnected()) Serial.println("Network connected");

    // GPRS connection parameters are usually set after network registration
    Serial.print(F("Connecting to "));
    Serial.print(apn);
    if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
        Serial.println(" fail");
        delay(10000);
        return;
    }
    Serial.println(" success");

    // Check connection
    if (modem.isGprsConnected()) Serial.println("GPRS connected");
}

void connectMQTT()
{
    Serial.print("Connecting to "); Serial.print(broker);

    for (int i = 0; i < 30; i++){
        // Create a random client ID
        String clientId = "ESP32Client-";
        clientId += String(random(0xffff), HEX);

        // Attempt to connect
        if (mqtt.connect(clientId.c_str(), mqttUsername, mqttPassword)) {
            Serial.println("\nconnected!");
            break;
        } else Serial.print(".");
        delay(1000);
    }

    if(mqtt.connected()){
        Serial.println("MQTT Connected!");
    } else ESP.restart();
}

void connect()
{
    // Registrate network
    if (!modem.isGprsConnected()) {
        Serial.println("GPRS disconnected!");
        initModem(); connectGPRS();
    }

    // Connect MQTT
    if (modem.isGprsConnected() && !mqtt.connected()) {
        Serial.println("=== MQTT NOT CONNECTED ===");
        
        // Reconnect every 10 seconds
        uint32_t t = millis();
        if (t - lastReconnectAttempt > 100L) {
            lastReconnectAttempt = t;
            connectMQTT();
        }
        delay(100);
        return;
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
        Serial.printf("PSRAM Total heap %d, PSRAM Free Heap %d\n",ESP.getPsramSize(),ESP.getFreePsram());
        bool sent;

        if(pic->format != 4){
            if(frame2jpg(pic, 80, &cnv_buf, &cnv_buf_len)){
                Serial.printf("Converted to JPEG, size = %d \n", cnv_buf_len);
            } else Serial.println("Failed to convert to JPEG");

            connect();
            if(mqtt.publish(topicImage, cnv_buf, cnv_buf_len)){
                Serial.println("Upload succesfull");
            } else{
                int err = mqtt.state();
                Serial.printf("Upload failed with error %d \n", err);
            }
        } else{
            connect();
            if(mqtt.publish(topicImage, pic->buf, pic->len)){
                Serial.println("Upload succesfull");
            } else Serial.printf("Upload failed with error %d \n", mqtt.state());            
        }
    }

    Serial.printf("PSRAM Total heap %d, PSRAM Free Heap %d\n",ESP.getPsramSize(),ESP.getFreePsram());
    free(cnv_buf); esp_camera_fb_return(pic);
}

void setup()
{
    // Set console baud rate
    SerialMon.begin(115200);
    Serial2.begin(115200, SERIAL_8N1, 14, 4);
    Serial.println("Hello world!");
    delay(10);

    // Pin mode of modem pins
    pinMode(LED_PIN, OUTPUT);       // Onboard LED light, it can be used freely
    pinMode(POWER_PIN, OUTPUT);     // POWER_PIN : This pin controls the power supply of the SIM7600
    pinMode(PWR_PIN, OUTPUT);       // PWR_PIN ： This Pin is the PWR-KEY of the SIM7600

    // Initialize camera (enable to initialize camera before sim)
    // WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
    // init_camera();
    delay(1000);

    SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);

    // Start modem and connect GPRS and MQTT
    mqtt.setServer(broker, 1883);
    connect();

    // MQTT buffer size
    Serial.printf("Buffer size: %d \n", mqtt.getBufferSize());
    bool buff = mqtt.setBufferSize(65535);
    if(buff) Serial.printf("New buffer size set successfully: %d \n", mqtt.getBufferSize());
    Serial.printf("PSRAM Total heap %d, PSRAM Free Heap %d\n",ESP.getPsramSize(),ESP.getFreePsram());

    delay(10000);

    // Initialize camera (disable to initialize camera after sim)
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
    init_camera();

}

void loop()
{
    while (SerialAT.available()) {
        Serial.write(SerialAT.read());
    }
    while (Serial.available()) {
        SerialAT.write(Serial.read());
    }

    //mqtt.publish(topicMeasure, "{\"temperature\":16,\"humidity\":53}");
    //Serial2.print("Data sent"); SerialMon.println("Data sent");
    //take_picture();
    mqtt.loop();
    if(mqtt.publish(topicMeasure, "{\"temperature\":1,\"humidity\":2,\"pressure\":3,\"temperature2\":4,\"humidity2\":5,\"pressure2\":6}")) Serial.println("Test data sent succesfully");
    delay(1000);
}

//*/