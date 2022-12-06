#include <Arduino.h>

#include "Wire.h"
#include "SPI.h"
#include "SD.h"

#include <LowPower.h>
#include <PMserial.h> 
#include <StateMachine.h>
#include <BlueDot_BME280.h>
#include <ArduinoJson.h>

#define INTERRUPT_OUTPUT_PIN 4
#define PMS_TX 8
#define PMS_RX 9

char incoming;

// Functions
void write_to_file(StaticJsonDocument<256> document, String filename);
void sleep(int sleep_time);
void message_to_esp(String message_string);
void s0_sleep();
void s1_measure_BME();
void s2_measure_PMS();
void s3_send();
void s4_time();

StateMachine machine = StateMachine();

// Define states
State* S0 = machine.addState(&s0_sleep); 
State* S1 = machine.addState(&s1_measure_BME); 
State* S2 = machine.addState(&s2_measure_PMS); 
State* S3 = machine.addState(&s3_send); 
State* S4 = machine.addState(&s4_time); 
//State* S5 = machine.addState(&s5_cooldown); 

// PMS5003 Pins and serial connection
SerialPM pms(PMS5003, PMS_RX, PMS_TX);
bool pms_value_updated = false;

// BME280 objects and availability
BlueDot_BME280 bme1;                                     
BlueDot_BME280 bme2;                                     
bool bme1Detected = false;                                    
bool bme2Detected = false;

// Interval settings
unsigned long pms_interval = 50000;//1800000;
unsigned long bme_interval = 20000;//300000;
unsigned long send_interval = 120000;//3600000;

// Time variables
unsigned long pms_last = 0;
unsigned long bme_last = 0;
unsigned long send_last = 0;
unsigned long time_last = 0;
unsigned long unix_time = 0;

//=======================================//
//============== States =================//
//=======================================//

void s0_sleep()
{
  LowPower.powerSave(SLEEP_8S, ADC_OFF, BOD_ON, TIMER2_OFF);
  String msg = "Arduino just woke up" + String(millis());
  message_to_esp(msg);
}

void s1_measure_BME()
{
  // Create json object
  StaticJsonDocument<256> doc;

  // Read pms values
  if (true)//bme1Detected)
  {
    doc["temperature1"] = (float)16.23;//bme1.readTempC();
    doc["humidity1"] = (float)52.71;//bme1.readHumidity();
    doc["pressure1"] = (float)1020.47;//bme1.readPressure();
  }

  if (true)//bme2Detected)
  {
    doc["temperature2"] = (float)16.23;//bme2.readTempC();
    doc["humidity2"] = (float)52.71;//bme2.readHumidity();
    doc["pressure2"] = (float)1020.47;//bme2.readPressure();
  }

  doc["time"] = unix_time + (int)((millis()-time_last)/1000);

  if(pms_value_updated)
  {
    doc["pm01"] = pms.pm01;
    doc["pm10"] = pms.pm10;
    doc["pm25"] = pms.pm25;
    pms_value_updated = false;
  }

  // Write to document
  String msg = "BME280 measured";
  message_to_esp(msg);
  write_to_file(doc, "data.jsonl");
  write_to_file(doc, "today.jsonl");
  bme_last = millis();
}

void s2_measure_PMS()
{
  // Power on pms sensor and refresh air
  pms.init();
  pms.wake();
  sleep(32);  // wait for some air to flow
  delay(50);  // give uart time to receive

  // Update values and global variables
  pms.read();
  pms.sleep();
  pms_value_updated = true;
  pms_last = millis();

  // Send message to esp
  String msg = "PMS5003 measured";
  message_to_esp(msg);
}

void s3_send()
{
  digitalWrite(INTERRUPT_OUTPUT_PIN, true);
  delay(2000);
  if(Serial.available() > 0)
  {
    Serial.write('s');
    delay(500);
    incoming = Serial.read();
  }

  // If ESP ready to receive
  if(incoming == 's')
  {
    if(SD.begin(6))
    {
    File file = SD.open("today.jsonl", FILE_READ);
    if(file)
    {
      while (true) 
      {
        // Create json object and buffer to write it to
        StaticJsonDocument<256> doc;
        char buffer[256];

        DeserializationError err = deserializeJson(doc, file);
        if (err) break;

        // Send message to ESP
        serializeJson(doc, buffer);
        Serial.println(buffer);
        delay(100);
      }
      file.close();
      SD.remove("today.jsonl");
    }
    else
    {
      String msg = "error opening file";
      message_to_esp(msg);
    }
  }
  else
  {
    String msg = "SD couldn't initialize";
    message_to_esp(msg);
  }
  SD.end();
  }
}

void s4_time()
{
  digitalWrite(INTERRUPT_OUTPUT_PIN, true);
  delay(2000);
  if(Serial.available() > 0)
  {
    // Connect
    Serial.write('c');
    delay(2000);

    // Obtain time
    Serial.write('t');
    delay(500);
    String time_string = Serial.readStringUntil('\n');
    unix_time = (unsigned long)time_string.toInt();
    
    // Send message to esp
    String msg = "Time received by Arduino";
    message_to_esp(msg);
  }
  digitalWrite(INTERRUPT_OUTPUT_PIN, false);
}

//=======================================//
//=========== Transitions ===============//
//=======================================//


bool transitionS0S1()
{
  bool check = (millis() - bme_last >= bme_interval) ? true : false;
  return check;
}

bool transitionS0S2()
{
  bool check = (millis() - pms_last >= pms_interval) ? true : false;
  return check;
}

bool transitionS0S3()
{
  bool check = (millis() - send_last >= send_interval) ? true : false;
  return check;
}

bool transitionS0S4()
{
  bool check = (unix_time == 0) ? true : false;
  return check;
}

bool transitionS1S0(){return true;}
bool transitionS2S0(){return true;}
bool transitionS3S0(){return true;}
bool transitionS4S0(){return true;}

//=======================================

// Write json message to file (.jsonl)
void write_to_file(StaticJsonDocument<256> document, String filename)
{
  if(SD.begin(6))
  {
    File file = SD.open(filename, FILE_WRITE); //A name that is too long gives errors!
    if(file)
    {
      serializeJson(document, file);
      file.println();
      file.close();
    }
    else
    {
      String msg = "error opening file";
      message_to_esp(msg);
    }
  }
  else
  {
    String msg = "SD couldn't initialize";
    message_to_esp(msg);
  }
  SD.end();
}

// Write log to file 
/*
void log_to_file(String log_message)
{
  if(SD.begin(6))
  {
    long time_val = unix_time + (int)((millis()-time_last)/1000);
    File file = SD.open("log.txt", FILE_WRITE); //A name that is too long gives errors!
    if(file)
    {
      file.println(time_val);
      file.print(" ");
      file.print(log_message);
      file.close();
    }
    else
    {
      //Serial.println("error opening file");
    }
  }
  else
  {
    //Serial.println("SD couldn't initialize");
  }
  SD.end();  
}*/


// Sleep sleep_time seconds
void sleep(int sleep_time)
{ 
  for(int i = 0; i < sleep_time; i++)
  { 
    LowPower.powerSave(SLEEP_1S, ADC_OFF, BOD_ON, TIMER2_OFF);
  }
}

// Send
void message_to_esp(String message_string)
{
  digitalWrite(INTERRUPT_OUTPUT_PIN, true);
  delay(1000);
  if(true)//Serial.available())
  {
    Serial.write("m");
    delay(20);
    Serial.print(message_string);
    Serial.write("\n");
  }
  digitalWrite(INTERRUPT_OUTPUT_PIN, false);
  delay(100);
}

//=======================================

void setup() 
{
  // Serial communication
  Serial.begin(115200, SERIAL_8N1);
  Serial.setTimeout(1000);
  pinMode(INTERRUPT_OUTPUT_PIN, OUTPUT);
  digitalWrite(INTERRUPT_OUTPUT_PIN, true);

  // Initialize BME280 sensor
  bme1.parameter.communication = 0;                    //I2C communication for Sensor 1 (bme1)
  bme2.parameter.communication = 0;                    //I2C communication for Sensor 2 (bme2)
  bme1.parameter.I2CAddress = 0x77;                    //I2C Address for Sensor 1 (bme1)
  bme2.parameter.I2CAddress = 0x76;                    //I2C Address for Sensor 2 (bme2)
  bme1.parameter.sensorMode = 0b01;                    //Setup Sensor mode for Sensor 1
  bme2.parameter.sensorMode = 0b01;                    //Setup Sensor mode for Sensor 2 
  bme1.parameter.IIRfilter = 0b100;                    //IIR Filter for Sensor 1
  bme2.parameter.IIRfilter = 0b100;                    //IIR Filter for Sensor 2
  bme1.parameter.humidOversampling = 0b101;            //Humidity Oversampling for Sensor 1
  bme2.parameter.humidOversampling = 0b101;            //Humidity Oversampling for Sensor 2
  bme1.parameter.tempOversampling = 0b101;             //Temperature Oversampling for Sensor 1
  bme2.parameter.tempOversampling = 0b101;             //Temperature Oversampling for Sensor 2
  bme1.parameter.pressOversampling = 0b101;            //Pressure Oversampling for Sensor 1
  bme2.parameter.pressOversampling = 0b101;            //Pressure Oversampling for Sensor 2

  if (bme1.init() == 0x60)
  {    
    bme1Detected = true;
  }

  if (bme2.init() == 0x60)
  {    
    bme2Detected = true;
  }
 
  // Initialize PMS5003 sensor
  pms.init();
  pms.sleep();

  // Define state transitions
  S0->addTransition(&transitionS0S1,S1);
  S0->addTransition(&transitionS0S2,S2);
  S0->addTransition(&transitionS0S3,S3);
  S0->addTransition(&transitionS0S4,S4);
  S1->addTransition(&transitionS1S0,S0);
  S2->addTransition(&transitionS2S0,S0);
  S3->addTransition(&transitionS3S0,S0);
  S4->addTransition(&transitionS4S0,S0);

  // Wait for ESP to initialize and send message
  sleep(30);
  String msg = "Arduino initialized";
  message_to_esp(msg);
  digitalWrite(INTERRUPT_OUTPUT_PIN, false);
}

void loop() 
{
  machine.run();
  delay(100);
}