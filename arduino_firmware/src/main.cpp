#include <Arduino.h>
#include "bme280.h"
#include "Wire.h"
#include "i2c.h"

#include "SPI.h"
#include "SD.h"

#include <LowPower.h>
#include <PMserial.h> // Arduino library for PM sensors with serial interface

File file;

SerialPM pms(PMS5003, 8, 9); //RX = 8, TX = 9

String file_path = "/sensor_data.txt";

unsigned long count = 0;

void sleep_for_milliseconds(long sleep_time);
void sleep_for_approx_milliseconds(long sleep_time);
void write_to_file(uint8_t* BME280_calibration, uint8_t* BME280_temperature, uint8_t* BME280_pressure);
void write_uint8_t_array_to_file_hex(File file, uint8_t* array, int length);
void write_uint16_t_array_to_file_hex(File file, uint16_t* array, int length);
String uint8_t_array_to_string_hex(uint8_t* array, int length);
String uint16_t_array_to_string_hex(uint16_t* array, int length);
String uint8_t_to_hex(uint8_t data);
String uint16_t_to_hex(uint16_t data);
String uint32_t_to_hex(uint32_t data);

void setup() {
  Serial.begin(115200);
  I2C::init();
  BME280::init();
  pms.init();
  pms.sleep();

  if (SD.begin(6)) {
    Serial.println("SD card initialized");
  }else{
    Serial.println("SD card initialization failed!");
  }
}

long startTime = 0;

void loop() {
  pms.init();
  pms.wake();
  sleep_for_milliseconds(32000);  //wait for some air to flow, datasheet says 30 seconds. but this function is a bit fast when pms is awake (the softwareserial interrupt stops a block early) so add a few seconds to be safe
  pms.read();
  pms.sleep();

  count++;
  BME280::measure();
  delay(1);
  uint8_t BME280_calibration[24];
  uint8_t BME280_temperature[3];
  uint8_t BME280_pressure[3];
  BME280::read_calibration(BME280_calibration);
  BME280::read_temperature(BME280_temperature);
  BME280::read_pressure(BME280_pressure);

  Serial.println(uint8_t_array_to_string_hex(BME280_calibration, 24) + " " + uint8_t_array_to_string_hex(BME280_temperature, 3) + " " + uint8_t_array_to_string_hex(BME280_pressure, 3) + " " + uint16_t_array_to_string_hex(pms.data, 9) + " " + uint32_t_to_hex(count));
  write_to_file(BME280_calibration, BME280_temperature, BME280_pressure);

  delay(100); //give the system time to finish communication
  sleep_for_approx_milliseconds(10000); //No interrupts should occur, which means using this function is fine
}

// Less efficient, but more accurate when interrupts may occur
// wake up every 15ms then go back to sleep untill time is up
void sleep_for_milliseconds(long sleep_time){ 
  for(int i = 0; i < (sleep_time/15); i++){ 
    LowPower.powerDown(SLEEP_15MS, ADC_OFF, BOD_ON);
  }
}

//more efficient, but WAY (!) less accurate when interrupts may occur
//wake up every second then go back to sleep untill time is up
void sleep_for_approx_milliseconds(long sleep_time){ 
  for(int i = 0; i < (sleep_time / 1000); i++){ 
    LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_ON);
  }
}

void write_to_file(uint8_t* BME280_calibration, uint8_t* BME280_temperature, uint8_t* BME280_pressure){
  if(file = SD.open(file_path, FILE_WRITE)){
    write_uint8_t_array_to_file_hex(file, BME280_calibration, 24);
    file.print(" ");
    write_uint8_t_array_to_file_hex(file, BME280_temperature, 3);
    file.print(" ");
    write_uint8_t_array_to_file_hex(file, BME280_pressure, 3);
    file.print(" ");
    write_uint16_t_array_to_file_hex(file, pms.data, 9);
    file.print(" ");
    file.println(uint32_t_to_hex(count));
    file.close();
  }
}

void write_uint8_t_array_to_file_hex(File file, uint8_t* array, int length){
  file.print(uint8_t_array_to_string_hex(array, length));
}

void write_uint16_t_array_to_file_hex(File file, uint16_t* array, int length){
  file.print(uint16_t_array_to_string_hex(array, length));
}

String uint8_t_array_to_string_hex(uint8_t* array, int length){
  String s = "";
  for(int i = 0; i < length; i++){
    s += uint8_t_to_hex(*array); //write data pointer is pointing to to file
    array++; //pointer needs to point to next data address
  }
  return s;
}

String uint16_t_array_to_string_hex(uint16_t* array, int length){
  String s = "";
  for(int i = 0; i < length; i++){
    s += uint16_t_to_hex(*array); //write data pointer is pointing to to file
    array++; //pointer needs to point to next data address
  }
  return s;
}

String uint8_t_to_hex(uint8_t data){
  return (String("0123456789ABCDEF"[data >> 4]) + String("0123456789ABCDEF"[data & 0x0F]));
}

String uint32_t_to_hex(uint32_t data){
  return uint8_t_to_hex(data >> 24) + uint8_t_to_hex(data >> 16) + uint8_t_to_hex(data >> 8) + uint8_t_to_hex(data);
}

String uint16_t_to_hex(uint16_t data){
  return uint8_t_to_hex(data >> 8) + uint8_t_to_hex(data);
}