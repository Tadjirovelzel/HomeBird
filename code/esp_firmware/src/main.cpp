/*  esp_firmware/src/main.cpp  -   Main code designed to run on LilyGo ESP32

    This code consists of five main functions:
    - void init_modem() initializes the modem by calling modem_on() repeatedly until the modem powers on, and 
    subsequently acquires and prints modem information;
    - void connect() is used to connect the device to MQTT over 4G connection by calling connectGPRS() and connectMQTT();
    - void init_camera() initializes the camera using pins and variables as defined in the camera_config structure;
    - void take_picture() is used to take a picture and send it in JPEG format to the MQTT broker. As a result, 
    pictures taken in other formats are converted to JPEG first;
    - void sendCurrentTime() obtains the current timestamp and sends it to the Arduino Every.

    In void setup() the functions init_modem(), init_camera(), and connect() are called, in order to initialize modem and camera and 
    properly connect the device to internet. The functions that are called in void loop() are determined by the Arduino input and processed
    using a switch-case structure. The following Arduino input is possible:
    - '?' requests the state of the ESP32, which will respond with 'y' if connected. Otherwise connect() is called;
    - 't' calls sendCurrentTime() in order to communicate the current timestamp to Arduino;
    - 's' reads an incoming json message from Arduino and publishes it to MQTT;
    - 'm' is used for debugging purposes only and prints messages from Arduino to the Serial Monitor.;
    - 'p' calls take_picture() once and sends a JPG image to MQTT;
    - 'v' calls take_picture() repeatedly to send a series of JPG images to MQTT, which get merged to a MP4 clip on the server side.

    Note that the ESP32 default state is to be shut down. Only if the Arduino opens the FET, the ESP32 will get battery power.

*/

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <StreamUtils.h>
#include "time.h"

// SIM model
#define TINY_GSM_MODEM_SIM7600

// Set serial for debug console, AT commands and debug prints
#define Serial Serial
#define SerialAT Serial1
#define TINY_GSM_DEBUG Serial

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
const char* broker = "excellent-engraver.cloudmqtt.com";        // Public IP address or domain name
const char* mqttUsername = "fnrfeqot";                          // MQTT username
const char* mqttPassword = "P-1Ta8Utd1Pj";                      // MQTT password
const char* topicMeasure = "device/6/measurement";              // Topic measurement
const char* topicRecord = "device/6/rec";                       // Topic start recording
const char* topicImage = "device/6/img";                        // Topic image
const char* topicClip = "device/6/clip";                        // Topic clip

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

// Time parameters
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;
unsigned long lastMillis = 0;

// Structure containing all camera pins and variables
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
    .frame_size = FRAMESIZE_QCIF,
    .jpeg_quality = 50,
    .fb_count = 1,
    .grab_mode = CAMERA_GRAB_LATEST //CAMERA_GRAB_WHEN_EMPTY
};

// Power on SIM7600 modem
void modem_on()
{   
    // The time of active low level impulse of PWRKEY pin to power on module, typically 500 ms
    Serial.println("\nStarting Up Modem...");
    digitalWrite(LED_PIN, LOW);   
    digitalWrite(POWER_PIN, HIGH);
    digitalWrite(PWR_PIN, HIGH);
    delay(500);
    digitalWrite(PWR_PIN, LOW);
    delay(5000);
    
    // Test modem response
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

// Initialize modem
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

// Connect device to GPRS in order to establish internet connection
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

// Connect device to MQTT if an internet connection is already present
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

// Connect device to MQTT over 4G connection
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

// Get current time and send it to Arduino Every
void sendCurrentTime()
{
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time");
        return;
    }
    time_t timeSinceEpoch = mktime(&timeinfo);
    Serial.print("Time obtained: ");
    Serial.println(String(timeSinceEpoch));

    if(Serial2.available())
    {
        Serial2.print(String(timeSinceEpoch));
        Serial2.write("\n");
        Serial.println("Time sent");
    }
}

// Initalize camera
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

// Take picture
void take_picture()
{
    camera_fb_t * pic = NULL;

    // Take picture with camera
    pic = esp_camera_fb_get();  
    if(!pic) {
        Serial.println("Camera capture failed");
        return;
    } else{
        Serial.println("Camera capture succesful");
        Serial.println("Format: " + String(pic->format) + "; size: " + String(pic->len));
        Serial.printf("PSRAM Total heap %d, PSRAM Free Heap %d\n",ESP.getPsramSize(),ESP.getFreePsram());

        // Format picture as JPG and send to MQTT broker
        if(pic->format != 4){
            if(frame2jpg(pic, 80, &cnv_buf, &cnv_buf_len)){
                Serial.printf("Converted to JPEG, size = %d \n", cnv_buf_len);
            } else Serial.println("Failed to convert to JPEG");

            if(mqtt.state()) connect();
            if(mqtt.publish(topicImage, cnv_buf, cnv_buf_len)){
                Serial.println("Upload succesfull");
            } else Serial.printf("Upload failed with error %d \n", mqtt.state());
            free(cnv_buf);
        } else{
            if(mqtt.state()) connect();
            if(mqtt.publish(topicImage, pic->buf, pic->len)){
                Serial.println("Upload succesfull");
            } else Serial.printf("Upload failed with error %d \n", mqtt.state());
            delay(100);
        }
    }

    esp_camera_fb_return(pic);
}

void setup() 
{

    // Set console baud rate
    Serial.begin(115200);
    Serial2.begin(115200, SERIAL_8N1, 14, 4);
    Serial.println("Hello world!");
    delay(10);

    // Onboard LED light, it can be used freely
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // POWER_PIN : This pin controls the power supply of the SIM7600
    pinMode(POWER_PIN, OUTPUT);
    digitalWrite(POWER_PIN, HIGH);

    // PWR_PIN ： This Pin is the PWR-KEY of the SIM7600
    // The time of active low level impulse of PWRKEY pin to power on module , type 500 ms
    pinMode(PWR_PIN, OUTPUT);
    digitalWrite(PWR_PIN, HIGH);
    delay(500);
    digitalWrite(PWR_PIN, LOW);
    delay(1000);

    SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);

    // Start modem and connect GPRS and MQTT
    mqtt.setServer(broker, 1883);
    if(mqtt.state()) connect();
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    // MQTT buffer size
    Serial.printf("Buffer size: %d \n", mqtt.getBufferSize());
    if(mqtt.setBufferSize(65535)) Serial.printf("New buffer size set successfully: %d \n", mqtt.getBufferSize());
    if(mqtt.publish(topicMeasure, "{\"temperature\":16,\"humidity\":53}")) Serial.println("Test data sent succesfully");

    // Initialize camera
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
    init_camera();
}

void loop() 
{  
    char incoming = 0;
    if(Serial2.available() > 0) incoming = Serial2.read();

    switch(incoming)
    {
    // Obtain current time and send to arduino
    case 't':
        Serial.println("Incoming: t");
        sendCurrentTime();
        break;

    // Publish incoming json message to MQTT
    case 's':
        Serial.println("Incoming: s");
        delay(50);
        if(Serial2.available())
        {
            // Read and convert incoming data
            String incoming_string = Serial2.readStringUntil('\n');
            Serial.println(incoming_string);
            char buffer[256];
            incoming_string.toCharArray(buffer, incoming_string.length()+1);

            // Connect to MQTT and send
            if (!mqtt.connected()) connect();
            if (mqtt.connected())
            {
                mqtt.publish(topicMeasure, buffer, incoming_string.length());
                Serial.print("Data sent to MQTT");
            }
        }
        break;

    // Take a picture and publish to MQTT
    case 'p':
        Serial.println("Incoming: p");
        //take_picture(topicImage);
        break;

    // Record a video and publish to MQTT
    case 'v':
        Serial.println("Incoming: v");
        // for(int i=0; i < 75; i++){
        //     take_picture(topicClip);
        //     if(i == 5) mqtt.publish(topicRecord, "{hello: world}");
        //     delay(175);
        // }
        break;

    // Receive incoming message from arduino and print it
    case 'm':
        Serial.println("Incoming: m");
        delay(50);
        if(Serial2.available())
        {
            String message_arduino = Serial2.readStringUntil('\n');
            Serial.println(message_arduino);
        }
        break;
    
    // Check connection to MQTT broker
    case '?':
        Serial.println("Incoming: ?");
        if (mqtt.connected()){
            Serial.println("Connection request Arduino: connected");
            for(int i=0; i<50; i++){
                Serial2.print('y');
                delay(2);
            }
        } 
        else{
            Serial.println("ESP32 disconnected");
            connect();
        } 
        break;

    case 0:
        break;

    default:
        Serial.print("Error: unknown char: '");
        Serial.print(incoming);
        Serial.println("'");
        break;
    }
}
//*/
