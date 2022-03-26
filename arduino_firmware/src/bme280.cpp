#include <Arduino.h>

#include "bme280.h"
#include "i2c.h"

namespace BME280 {

    void init(int bme280_address){
        I2C::writeRegister(bme280_address, 0xF2, 0b00000111); //oversampling humidity = 16
        I2C::writeRegister(bme280_address, 0xF4, 0b10110100); //oversampling temp, press = 16, mode = forced
        I2C::writeRegister(bme280_address, 0xF5, 0b00000000); //turn off iir
    }

    void read_calibration(int bme280_address, uint8_t* data){
        I2C::getRegister(bme280_address, 0x88, data, 24);
        data += 24;
        I2C::getRegister(bme280_address, 0xA1, data, 1);
        data += 1;
        I2C::getRegister(bme280_address, 0xE1, data, 7);

    }

    void measure(int bme280_address){
        I2C::writeRegister(bme280_address, 0xF4, 0b10110101); //start measurement
    }

    void read_temperature(int bme280_address, uint8_t* data){
        I2C::getRegister(bme280_address, 0xFA, data, 3);
    }

    void read_pressure(int bme280_address, uint8_t* data){
        I2C::getRegister(bme280_address, 0xF7, data, 3);
    }

    void read_humidity(int bme280_address, uint8_t* data){
        I2C::getRegister(bme280_address, 0xFD, data, 2);
    }
}