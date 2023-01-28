//*
#include <Arduino.h>
#include <avr/sleep.h>

#include "Wire.h"
#include "SPI.h"
#include "SD.h"

#include <PMserial.h> 
#include <StateMachine.h>
#include <BlueDot_BME280.h>
#include <ArduinoJson.h>

#define INTERRUPT_OUTPUT_PIN 4
#define PMS_TX 8
#define PMS_RX 9
#define PIR_PIN 2

#define test_mode

#ifdef test_mode
  #define pms_interval 50000
  #define bme_interval 20000
  #define send_interval 120000
  #define pic_interval 20000
  #define cooldown 240000
#else
  #define pms_interval 1800000
  #define bme_interval 300000
  #define send_interval 3600000
  #define pic_interval 300000
  #define cooldown 108000000
#endif

// Functions
void s0_sleep();
void s1_measure_BME();
void s2_measure_PMS();
void s3_send();
void s4_time();
void s5_picture();
void s6_video();
void write_to_file(StaticJsonDocument<256> document, String filename);
void message_to_esp(String message_string);
void sleep(int sleep_time);
bool sleep_while(int sleep_time);

StateMachine machine = StateMachine();

// Define states
State* S0 = machine.addState(&s0_sleep); 
State* S1 = machine.addState(&s1_measure_BME); 
State* S2 = machine.addState(&s2_measure_PMS); 
State* S3 = machine.addState(&s3_send); 
State* S4 = machine.addState(&s4_time); 
State* S5 = machine.addState(&s5_picture); 
State* S6 = machine.addState(&s6_video); 

// PMS5003 Pins and serial connection
SerialPM pms(PMS5003, PMS_RX, PMS_TX);
bool pms_value_updated = false;

// BME280 objects
BlueDot_BME280 bme1;                                     
BlueDot_BME280 bme2;                                     

// Time variables
unsigned long myTimer = 0;
unsigned long timer_last = 0;
unsigned long pms_last = 0;
unsigned long bme_last = 0;
unsigned long send_last = 0;
unsigned long picture_last = 0;
unsigned long video_last = 0;
unsigned long time_last = 0;
unsigned long unix_time = 0;

// Time function
unsigned long currentTime()
{
  unsigned long current_time = myTimer + (millis() - timer_last);
  return current_time;
}

// Sleep functions

void RTC_init(void)
{
  while (RTC.STATUS > 0) ;     //Wait for all register to be synchronized 
 
  RTC.CLKSEL = RTC_CLKSEL_INT1K_gc;        // Run low power oscillator (OSCULP32K) at 1024Hz for long term sleep
  RTC.PITINTCTRL = RTC_PI_bm;              // PIT Interrupt: enabled 

  RTC.PITCTRLA = RTC_PERIOD_CYC1024_gc | RTC_PITEN_bm;     // Set period 8 seconds (8192 for eight sec) and enable PIC                      
}

ISR(RTC_PIT_vect)
{
  RTC.PITINTFLAGS = RTC_PI_bm;          // Clear interrupt flag by writing '1' (required) 
}



//======================================//
//=============== States ===============//
//======================================//


void s0_sleep()
{
  myTimer += (millis() - timer_last);
  sleep_cpu();
  myTimer += 1024;
  timer_last = millis();
  message_to_esp("Arduino just woke up; millis() = " + String(millis()) + "; time (ms) = " + String(currentTime()));
}

void s1_measure_BME()
{
  // Create json object
  StaticJsonDocument<256> doc;

  // Read pms values
  if(bme1.init() == 0x60)
  {
    doc["temperature1"] = bme1.readTempC();
    doc["humidity1"] = bme1.readHumidity();
    doc["pressure1"] = bme1.readPressure();
  } else
    message_to_esp("BME sensor 1 not detected");

  if (bme2.init() == 0x60)
  {
    doc["temperature2"] = bme2.readTempC();
    doc["humidity2"] = bme2.readHumidity();
    doc["pressure2"] = bme2.readPressure();
  } else
    message_to_esp("BME sensor 2 not detected");

  doc["time"] = unix_time + (int)((currentTime()-time_last)/1000);

  if(pms_value_updated)
  {
    doc["pm01"] = pms.pm01;
    doc["pm10"] = pms.pm10;
    doc["pm25"] = pms.pm25;
    pms_value_updated = false;
  }

  // Write to document
  message_to_esp("BME280 measured");
  write_to_file(doc, "data.txt");
  write_to_file(doc, "today.txt");
}

void s2_measure_PMS()
{
  // Power on pms sensor and refresh air
  pms.init();
  pms.wake();
  sleep(32); // warmup period 

  // Update values and global variables
  pms.read();
  pms.sleep();
  pms_value_updated = true;
  pms_last = currentTime();

  // Send message to esp
  message_to_esp("PMS5003 measured");
}

void s3_send()
{
  digitalWrite(INTERRUPT_OUTPUT_PIN, true);
  bool ready = sleep_while(120);

  // If ESP ready to receive
  if(ready)
  {
    if(SD.begin(6)){
    File file = SD.open("today.txt", FILE_READ);
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
      SD.remove("today.txt");
    } else
      message_to_esp("error opening file");
  } else
    message_to_esp("SD couldn't initialize");
  SD.end();
  }
}

void s4_time()
{
  digitalWrite(INTERRUPT_OUTPUT_PIN, true);
  bool ready = sleep_while(120);
  if(ready)
  {
    // Obtain time
    Serial1.write('t');
    delay(100);
    String time_string = Serial1.readStringUntil('\n');
    message_to_esp("Time received by Arduino (str): " + String(unix_time));

    unsigned long recv_time = (unsigned long)time_string.toInt();
    unix_time = (recv_time > unix_time) ? recv_time : unix_time;
    
    // Print the received time
    message_to_esp("Time received by Arduino: " + String(unix_time));
  }
  digitalWrite(INTERRUPT_OUTPUT_PIN, false);
}

void s5_picture()
{
  digitalWrite(INTERRUPT_OUTPUT_PIN, true);
  bool ready = sleep_while(120);
  if(ready)
  {    
    Serial1.write('p');
    bool done = sleep_while(60);
  }
  //digitalWrite(INTERRUPT_OUTPUT_PIN, false);
}

void s6_video()
{
  digitalWrite(INTERRUPT_OUTPUT_PIN, true);
  bool ready = sleep_while(120);
  if(ready)
  {    
    Serial1.write('v');
    bool done = sleep_while(60);
  }
  //digitalWrite(INTERRUPT_OUTPUT_PIN, false);
}


//=======================================//
//=========== Transitions ===============//
//=======================================//


bool transitionS0S1()
{
  bool check = (currentTime() - bme_last >= bme_interval) ? true : false;
  return check;
}

bool transitionS0S2()
{
  bool check = (currentTime() - pms_last >= pms_interval) ? true : false;
  return check;
}

bool transitionS0S3()
{
  bool check = (currentTime() - send_last >= send_interval) ? true : false;
  return check;
}

bool transitionS0S4()
{
  bool check = (unix_time == 0) ? true : false;
  return check;
}

bool transitionS0S5()
{
  bool check = (currentTime() - picture_last >= pic_interval) ? false : false;
  return check;
}

bool transitionS0S6()
{
  if (currentTime() - video_last < cooldown || (currentTime() / 3600) % 24 + 1 < 7 || (currentTime() / 3600) % 24 + 1 > 21) return false;
  
  bool check = (digitalRead(PIR_PIN) == true) ? false : false;
  return check;
}

bool transitionS1S0(){return true;}
bool transitionS2S0(){return true;}
bool transitionS3S0(){return true;}
bool transitionS4S0(){return true;}
bool transitionS5S4(){return true;}
bool transitionS6S4(){return true;}


//=======================================//
//=========== Other functions ===========//
//=======================================//


// Write json message to file
void write_to_file(StaticJsonDocument<256> document, String filename)
{
  if(!SD.begin(6))
  {
    message_to_esp("SD couldn't initialize");
    return;
  }

  SD.begin(6);
  File myfile = SD.open(filename, FILE_WRITE); // Name should respect 8.3 limit
  if(myfile)
  {
      serializeJson(document, myfile);
      myfile.println();
      myfile.close();

      message_to_esp("written to file");
      bme_last = currentTime();
  }
  else
  {
    message_to_esp("error opening file");
  }
  SD.end();
}

// Sleep sleep_time seconds
void sleep(int sleep_time)
{ 
  for(int i = 0; i < sleep_time; i++)  {sleep_cpu();}
}

// Sleep sleep_time seconds while ESP32 is not ready yet
bool sleep_while(int sleep_time)
{
  for(int i = 0; i < sleep_time; i++){
    myTimer += (millis() - timer_last);
    sleep_cpu();
    myTimer += 1024;
    timer_last = millis();
    if(Serial1.availableForWrite()) Serial1.write('?');
    delay(50);
    if(Serial1.read() == 'y') return true;
  }
  return false;
}


// Send
void message_to_esp(String message_string)
{
  digitalWrite(INTERRUPT_OUTPUT_PIN, true);
  delay(1000);
  if(true)//Serial.available())
  {
    Serial1.write("m");
    delay(20);
    Serial1.print(message_string);
    Serial.println(message_string);
    Serial1.write("\n");
  }
  digitalWrite(INTERRUPT_OUTPUT_PIN, false);
  delay(100);
}

//=======================================

void setup() 
{
  // Serial communication
  Serial.begin(115200, SERIAL_8N1);
  Serial1.begin(115200, SERIAL_8N1);
  Serial.setTimeout(1000);

  // Set pinModes
  pinMode(INTERRUPT_OUTPUT_PIN, OUTPUT);
  pinMode(PIR_PIN, OUTPUT);
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
 
  // Initialize PMS5003 sensor
  pms.init();
  pms.sleep();

  // Define state transitions
  S0->addTransition(&transitionS0S1,S1);
  S0->addTransition(&transitionS0S2,S2);
  S0->addTransition(&transitionS0S3,S3);
  S0->addTransition(&transitionS0S4,S4);
  S0->addTransition(&transitionS0S5,S5);
  S0->addTransition(&transitionS0S6,S6);
  S1->addTransition(&transitionS1S0,S0);
  S2->addTransition(&transitionS2S0,S0);
  S3->addTransition(&transitionS3S0,S0);
  S4->addTransition(&transitionS4S0,S0);
  S5->addTransition(&transitionS5S4,S4);
  S6->addTransition(&transitionS6S4,S4);
 
  // Initialize sleep mode
  RTC_init();   
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);  // Set sleep mode to POWER DOWN mode 
  sleep_enable(); 

  // Wait for ESP to initialize and send message
  sleep_while(120);
  message_to_esp("Arduino initialized");
  digitalWrite(INTERRUPT_OUTPUT_PIN, false);
}

void loop() 
{
  machine.run();
  delay(100);
}
//*/