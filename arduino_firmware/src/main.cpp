#include <Arduino.h>
#include "bme280.h"
#include "Wire.h"
#include "i2c.h"

#include "SPI.h"
#include "SD.h"

File file;

unsigned long count = 0;

void write_to_file(uint8_t* BME280_calibration, uint8_t* BME280_temperature, uint8_t* BME280_pressure);
void write_array_to_file_hex(File file, uint8_t* array, int length);
String array_to_string_hex(uint8_t* array, int length);
String uint8_t_to_hex(uint8_t data);
String uint32_t_to_hex(uint32_t data);

void setup() {
  Serial.begin(115200);

  delay(1000);
  I2C::init();
  BME280::init();

  if (!SD.begin(6)) {
    Serial.println("SD card initialization failed!");
  }else{
    Serial.println("SD card initialized");
  }
}

void loop() {
  delay(1000);

  count++;
  BME280::measure();
  delay(1);
  uint8_t BME280_calibration[24];
  uint8_t BME280_temperature[3];
  uint8_t BME280_pressure[3];
  BME280::read_calibration(BME280_calibration);
  BME280::read_temperature(BME280_temperature);
  BME280::read_pressure(BME280_pressure);

  Serial.println(array_to_string_hex(BME280_calibration, 24) + " " + array_to_string_hex(BME280_temperature, 3) + " " + array_to_string_hex(BME280_pressure, 3) + " " + uint32_t_to_hex(count));

  write_to_file(BME280_calibration, BME280_temperature, BME280_pressure);
}

void write_to_file(uint8_t* BME280_calibration, uint8_t* BME280_temperature, uint8_t* BME280_pressure){
  if(file = SD.open("/test.csv", FILE_WRITE)){
    write_array_to_file_hex(file, BME280_calibration, 24);
    file.print(" ");
    write_array_to_file_hex(file, BME280_temperature, 3);
    file.print(" ");
    write_array_to_file_hex(file, BME280_pressure, 3);
    file.print(" ");
    file.println(uint32_t_to_hex(count));
    file.close();
  }
}

void write_array_to_file_hex(File file, uint8_t* array, int length){
  file.print(array_to_string_hex(array, length));
}

String array_to_string_hex(uint8_t* array, int length){
  String s = "";
  for(int i = 0; i < length; i++){
    s += uint8_t_to_hex(*array); //write data pointer is pointing to to file
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