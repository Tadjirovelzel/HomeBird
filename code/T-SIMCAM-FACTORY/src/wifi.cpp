/*
#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <HttpClient.h> 

const char* ssid = "Moto Lennard";
const char* password = "hotspotlennard";

// const char* serverAddress = "https://eoltyrafh9ilblr.m.pipedream.net";
const char* serverAddress = "https://pleasework.free.beeceptor.com";
const int serverPort = 80;

WiFiClient wifiClient;
HttpClient client = HttpClient(wifiClient, serverAddress, serverPort);

void setup() {
  delay(2000);
  Serial.println("Hello World!");
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
}

void loop() {
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


// void loop() {
//   // Perform HTTP GET request
//   int statusCode = httpClient.get("/todos");

//   // Check if the request was successful
//   if (statusCode == 200) {
//     Serial.println("GET request successful!");
//     Serial.print("Response: ");
//     Serial.println(httpClient.responseBody());
//   } else {
//     Serial.print("Error in GET request. Status code: ");
//     Serial.println(statusCode);
//     Serial.print("Error message: ");
//     Serial.println(httpClient.responseBody());
//   }

//   // Delay before the next iteration
//   delay(5000);  // Adjust as needed
// }
//*/