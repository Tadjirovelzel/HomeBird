//*
#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <HttpClient.h> 
#include "Cameraconfig.h"

const char* ssid = "Moto Lennard";
const char* password = "hotspotlennard";

//const char* serverAddress = "pleasework.free.beeceptor.com";
const char* serverAddress = "http://z7y6g.wiremockapi.cloud";//pleasework.free.beeceptor.com";
const int serverPort = 80;

WiFiClient wifiClient;
HttpClient client = HttpClient(wifiClient, serverAddress, serverPort);

bool reply = false;
uint32_t lastReconnectAttempt = 0;
size_t cnv_buf_len;
uint8_t * cnv_buf = NULL;

// rlc::Console console(Serial);
// HardwareSerial SerialAT(1);
// rlc::AtCommand command_helper(SerialAT, console, false);

void upload_pic(uint8_t *pic_buf, size_t len)
{
    Serial.printf("Start uploading pictures with length %d \n",len);
    //client.connectionKeepAlive();
    Serial.println(F("Performing HTTP POST request... "));
    Serial.println(F("Wait for upload to complete..."));
    client.beginRequest();
    client.post(serverAddress);
    client.sendHeader("Content-Type", "image/jpg");
    client.sendHeader("picpath", "/img");
    // client.sendHeader("Accept-Encoding", "gzip, deflate, br");
    client.sendHeader("Content-Length", String(len));
    client.beginBody();
    uint32_t j = 0;
    uint32_t shard = 4096; //1426;
    for (int32_t i = len; i > 0;) {
        if (i >= shard) {
            size_t num_bytes_sent = client.write((const uint8_t *)(pic_buf + shard * j), shard);
            Serial.printf("%d bytes sent \n", num_bytes_sent);
            i -= shard;
            j++;
        } else {
            client.write((const uint8_t *)(pic_buf + shard * j), i);
            break;
        }
    }
    
    // size_t max_chunk_size = 1426; //4096;
    // uint8_t chunk_buf[max_chunk_size];
    // size_t current_position = 0;
    // while (current_position < len)
    // {
    //     size_t chunk_size = 0;
    //     for(size_t i=0;i<max_chunk_size;i++)
    //     {
    //         if(current_position + i < len)
    //         {
    //             chunk_size += 1;
    //             chunk_buf[i] = pic_buf[current_position + i];
    //         }
    //     }
    //     size_t num_bytes_sent = client.write(chunk_buf, chunk_size);

    //     Serial.printf("%d bytes sent \n",num_bytes_sent);

    //     current_position += max_chunk_size;
    // }
    
    client.endRequest();
    // read the status code and body of the response
    int statusCode = client.responseStatusCode();
    String response = client.responseBody();
    Serial.print("Status code: ");
    Serial.println(statusCode);
    Serial.print("Response: ");
    Serial.println(response);
    // Shutdown
    client.stop();
    Serial.println(F("Server disconnected"));

    delay(20000);
}

// {
//     _command_helper.write(head, 1000);

//     size_t max_chunk_size = 4096;
//     uint8_t chunk_buf[max_chunk_size];
//     size_t current_position = 0;
//     while (current_position < size)
//     {
//         size_t chunk_size = 0;
//         for(size_t i=0;i<max_chunk_size;i++)
//         {
//             if(current_position + i < size)
//             {
//                 chunk_size += 1;
//                 chunk_buf[i] = buf[current_position + i];
//             }
//         }
//         size_t num_bytes_sent = _command_helper.write(chunk_buf, chunk_size, 2000);

//         current_position += max_chunk_size;
//     }

//     _command_helper.write(tail, 1000);
// }


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
    int statusCode = client.post("/todos", "application/x-www-form-urlencoded", postData);

    // Check if the request was successful
    if (statusCode == 200) {
    Serial.println("POST request successful!");
    Serial.print("Response: ");
    Serial.println(client.responseBody());
    } else {
    Serial.print("Error in POST request. Status code: ");
    Serial.println(statusCode);
    Serial.print("Error message: ");
    Serial.println(client.responseBody());
    }

    // Delay before the next iteration
    delay(5000);
}

void httpGet(){
  // Perform HTTP GET request
  int statusCode = client.get("/todos");

  // Check if the request was successful
  if (statusCode == 200) {
    Serial.println("GET request successful!");
    Serial.print("Response: ");
    Serial.println(client.responseBody());
  } else {
    Serial.print("Error in GET request. Status code: ");
    Serial.println(statusCode);
    Serial.print("Error message: ");
    Serial.println(client.responseBody());
  }

  // Delay before the next iteration
  delay(5000);  // Adjust as needed
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
  
    // Upload picture using http
    httpPost();
    //rlc::Http::post_file_buffer("pleasework.free.beeceptor.com", pic->buf, pic->len);
    upload_pic(pic->buf, pic->len);
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
  pinMode(PWR_PIN, OUTPUT);       // PWR_PIN ： This Pin is the PWR-KEY of the SIM7600

  // Initialize camera (enable to initialize camera before sim)
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  init_camera();
  delay(1000);

  // SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
}

void loop()
{
  // while (SerialAT.available()) {
  //     Serial.write(SerialAT.read());
  // }
  // while (Serial.available()) {
  //     SerialAT.write(Serial.read());
  // }

  take_picture();
  delay(50000);
}
//*/