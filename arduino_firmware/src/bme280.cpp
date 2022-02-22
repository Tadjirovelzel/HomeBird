#include <Arduino.h>

#include "bme280.h"
#include "i2c.h"

namespace BME280 {
    // #define BME280_ADDRESS 0b11101110 //or could be 0b11101100
    #define BME280_ADDRESS 0b1110110

    void init(){
        // I2C::writeRegister(BME280_ADDRESS, 0xF2, 0b00000111); //oversampling humidity = 16
        I2C::writeRegister(BME280_ADDRESS, 0xF4, 0b10110100); //oversampling temp, press = 16, mode = forced
        I2C::writeRegister(BME280_ADDRESS, 0xF5, 0b00000000); //turn off iir
    }

    void read_calibration(uint8_t* data){
        I2C::getRegister(BME280_ADDRESS, 0x88, data, 24);

        // dig_H1 = I2C::getRegister(BME280_ADDRESS, 0xA1);

        // I2C::getRegister(BME280_ADDRESS, 0xE1, data, 7);

    }

    void measure(){
        I2C::writeRegister(BME280_ADDRESS, 0xF4, 0b10110101); //start measurement
    }

    void read_temperature(uint8_t* data){
        I2C::getRegister(BME280_ADDRESS, 0xFA, data, 3);
    }

    void read_pressure(uint8_t* data){
        I2C::getRegister(BME280_ADDRESS, 0xF7, data, 3);
    }

    void read_humidity(uint8_t* data){
        I2C::getRegister(BME280_ADDRESS, 0xFD, data, 2);
    }
}