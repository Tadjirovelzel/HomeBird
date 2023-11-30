/*
#include <Arduino.h>
#include <HttpClient.h>
// SIM model
#define TINY_GSM_MODEM_SIM7600

// Set serial for debug console, AT commands and debug prints
#define SerialMon Serial
#define SerialAT Serial1
#define TINY_GSM_DEBUG SerialMon
#define TINY_GSM_YIELD_MS 2

// Modem pins
#define uS_TO_S_FACTOR          1000000ULL  // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP           60          // Time ESP32 will go to sleep (in seconds) 

#define PIN_TX                  45
#define PIN_RX                  46
#define UART_BAUD               115200
#define PWR_PIN                 48
#define LED_PIN                 21

// set GSM PIN and apn, http details
#define GSM_PIN "0000"

const char* apn = "m2m.tele2.com";
const char* gprsUser = "";
const char* gprsPass = "";
const char* serverAddress = "https://pleasework.free.beeceptor.com";
const int serverPort = 80;

// Declare TinyGSM, and HTTP client 
#include <TinyGsmClient.h>
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
HttpClient http = HttpClient(client, serverAddress, serverPort);

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
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      14
#define SIOD_GPIO_NUM     4
#define SIOC_GPIO_NUM     5

#define Y9_GPIO_NUM       15
#define Y8_GPIO_NUM       16
#define Y7_GPIO_NUM       17
#define Y6_GPIO_NUM       12
#define Y5_GPIO_NUM       10
#define Y4_GPIO_NUM       8
#define Y3_GPIO_NUM       9
#define Y2_GPIO_NUM       11 
#define VSYNC_GPIO_NUM    6
#define HREF_GPIO_NUM     7
#define PCLK_GPIO_NUM     13

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
    .frame_size = FRAMESIZE_QHD,
    .jpeg_quality = 50,
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
    SerialAT.begin(115200, SERIAL_8N1, PIN_RX, PIN_TX);
    delay(100);
    pinMode(PWR_PIN, OUTPUT);
    digitalWrite(PWR_PIN, 1);
    delay(500);
    digitalWrite(PWR_PIN, 0);
    delay(3000);
    
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



void connect()
{
    // Registrate network
    if (!modem.isGprsConnected()) {
        Serial.println("GPRS disconnected!");
        initModem(); connectGPRS();
    }
}


void upload_pic(uint8_t *pic_buf, size_t len)
{
    Serial.println(F("Start uploading pictures... "));
    http.connectionKeepAlive();
    Serial.println(F("Performing HTTP POST request... "));
    Serial.println(F("Wait for upload to complete..."));
    http.beginRequest();
    http.post(serverAddress);
    http.sendHeader("Content-Type", "image/jpg");
    http.sendHeader("picpath", "/todos");
    // http.sendHeader("Accept-Encoding", "gzip, deflate, br");
    http.sendHeader("Content-Length", String(len));
    http.beginBody();
    uint32_t j = 0;
    uint32_t shard = 1426;
    for (int32_t i = len; i > 0;) {
        if (i >= shard) {
            http.write((const uint8_t *)(pic_buf + shard * j), shard);
            i -= shard;
            j++;
        } else {
            http.write((const uint8_t *)(pic_buf + shard * j), i);
            break;
        }
    }
    http.endRequest();
    // read the status code and body of the response
    int statusCode = http.responseStatusCode();
    String response = http.responseBody();
    Serial.print("Status code: ");
    Serial.println(statusCode);
    Serial.print("Response: ");
    Serial.println(response);
    // Shutdown
    http.stop();
    Serial.println(F("Server disconnected"));

    delay(20000);
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

void httpPost(){
    Serial.println("Start HTTP post request");
    // Data to be sent in the POST request
    String postData = "key1=value1&key2=value2";

    // Perform HTTP POST request
    int statusCode = http.post("/todos", "application/x-www-form-urlencoded", postData);

    // Check if the request was successful
    if (statusCode == 200) {
    Serial.println("POST request successful!");
    Serial.print("Response: ");
    Serial.println(http.responseBody());
    } else {
    Serial.print("Error in POST request. Status code: ");
    Serial.println(statusCode);
    Serial.print("Error message: ");
    Serial.println(http.responseBody());
    }

    // Delay before the next iteration
    delay(5000);
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

        if(pic->format != 4){
            if(frame2jpg(pic, 80, &cnv_buf, &cnv_buf_len)){
                Serial.printf("Converted to JPEG, size = %d \n", cnv_buf_len);
            } else Serial.println("Failed to convert to JPEG");
        }
    }
    
    while (!modem.isGprsConnected()) connect();
    // Upload picture using http
    httpPost();
    //upload_pic(pic->buf, pic->len);
    Serial.printf("PSRAM Total heap %d, PSRAM Free Heap %d\n",ESP.getPsramSize(),ESP.getFreePsram());
    free(cnv_buf); esp_camera_fb_return(pic);
}


void setup()
{
    delay(5000);

    // Set console baud rate
    SerialMon.begin(115200);
    Serial2.begin(115200, SERIAL_8N1, 14, 4);
    Serial.println("Hello world!");
    delay(10);

    // Pin mode of modem pins
    pinMode(LED_PIN, OUTPUT);       // Onboard LED light, it can be used freely
    pinMode(PWR_PIN, OUTPUT);       // PWR_PIN ： This Pin is the PWR-KEY of the SIM7600

    // Initialize camera (enable to initialize camera before sim)
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
    init_camera();
    delay(1000);

    SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);
    connect();

    delay(10000);
}

void loop()
{
    while (SerialAT.available()) {
        Serial.write(SerialAT.read());
    }
    while (Serial.available()) {
        SerialAT.write(Serial.read());
    }

    take_picture();
    delay(50000);
}

//*/