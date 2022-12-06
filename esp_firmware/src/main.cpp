#include <WiFiClientSecure.h>
#include <MQTT.h>
#include <ArduinoJson.h>
#include "time.h"

// WiFi parameters
const char ssid[] = "H369AEA4CE8";
const char pass[] = "FA9694C7FEC3";

// MQTT parameters
#define MQTT_USER "nestwachtdevlennard"
#define MQTT_PASSWORD "nVy5@2KUKJpH3"
#define MQTT_SERIAL_PUBLISH_CH "device/5/measurement"
const char* mqtt_server = "16c8ca6fc79543699af71365bfeed7bf.s2.eu.hivemq.cloud";

// Time parameters
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

#define wakeUpPin GPIO_NUM_2
//void wakeUp(){}

WiFiClientSecure net;
MQTTClient client;

void connect() 
{
    Serial.print("checking wifi...");
    while (WiFi.status() != WL_CONNECTED) 
    {
        Serial.print(".");
        delay(1000);
    }

    Serial.print("\nconnecting...");
    net.setInsecure();
    while (!client.connect("32DrhEK#7LQNk", MQTT_USER, MQTT_PASSWORD))
    {
        Serial.print(".");
        delay(1000);
    }

    Serial.println("\nconnected!");
}

//void messageReceived(String &topic, String &payload) 
//{
//    Serial.println("incoming: " + topic + " - " + payload);
//}

void sendCurrentTime()
{
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time");
        return;
    }
    time_t timeSinceEpoch = mktime(&timeinfo);
    Serial.println(timeSinceEpoch);

    if(Serial2.available())
    {
        Serial2.write(timeSinceEpoch);
        Serial2.write("\n");
        Serial.println("Time sent");
    }
}

void sendMqtt()
{
    // Make sure to be connected to MQTT
    client.loop();
    delay(10);  // <- fixes some issues with WiFi stability
    if (!client.connected()){connect();}       
    Serial2.write('s');

    // Read data from Arduino and write to MQTT
    static StaticJsonDocument<256> doc;
    char buffer[256];

    if(Serial2.available() > 0)
    {
        const auto deser_err = deserializeJson(doc, Serial2);
        
        if (deser_err)
        {
            Serial.print(F("Failed to deserialize, reason: \""));
            Serial.print(deser_err.c_str());
            Serial.println('"');
        } 
        else
        {
            Serial.print(F("Recevied valid json document with "));
            Serial.print(doc.size());
            Serial.println(F(" elements."));
            serializeJsonPretty(doc, Serial);
            Serial.println();

            size_t n = serializeJson(doc, buffer);
            client.publish(MQTT_SERIAL_PUBLISH_CH, buffer, n);
        }
    }    
}


void setup() 
{
    Serial.begin(115200);
    Serial2.begin(115200, SERIAL_8N1, 14, 4);
  
    // Connect to WiFi and MQTT
    WiFi.begin(ssid, pass);
    client.begin(mqtt_server, 8883, net);

    // connect to NTP server and send time to Arduino
    connect();
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    sendCurrentTime();
}

void loop() 
{
    char incoming;
    if(Serial2.available() > 0)
    {
        //Serial.println("Connected");
        incoming = Serial2.read();
    }
  
    switch(incoming)
    {
    // time
    case 't':
        Serial.println("Incoming: t");
        sendCurrentTime();
        break;

    // connect
    case 'c':
        Serial.println("Incoming: c");
        connect();
        break;

    // send
    case 's':
        Serial.println("Incoming: s");
        sendMqtt();
        break;

    // message
    case 'm':
        Serial.println("Incoming: m");
        delay(50);
        if(Serial2.available())
        {
            String message_arduino = Serial2.readStringUntil('\n');
            Serial.println(message_arduino);
        }
        break;

    default:
        //Serial.println("Error: no connection");
        break;
    }
    incoming = '0';

    // Sleep with external wake up
    if (!digitalRead(wakeUpPin))
    {
        esp_sleep_enable_ext0_wakeup(wakeUpPin, 1);
    }
}