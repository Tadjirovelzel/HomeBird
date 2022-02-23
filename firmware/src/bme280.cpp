#include <Arduino.h>

#include "bme280.h"
#include "i2c.h"

namespace BME280 {
    // #define BME280_ADDRESS 0b11101110 //or could be 0b11101100
    #define BME280_ADDRESS 0b1110110

    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;
    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;
    uint8_t dig_H1;
    int16_t dig_H2;
    uint8_t dig_H3;
    int16_t dig_H4;
    int16_t dig_H5;
    int8_t  dig_H6;

    int32_t t_fine;

    void init(){
        // I2C::writeRegister(BME280_ADDRESS, 0xF2, 0b00000111); //oversampling humidity = 16
        I2C::writeRegister(BME280_ADDRESS, 0xF4, 0b10110100); //oversampling temp, press = 16, mode = forced
        I2C::writeRegister(BME280_ADDRESS, 0xF5, 0b00000000); //turn off iir

        uint8_t data[24];
        I2C::getRegister(BME280_ADDRESS, 0x88, data, 24);
        dig_T1 = data[0]  | (data[1]  << 8);
        dig_T2 = data[2]  | (data[3]  << 8);
        dig_T3 = data[4]  | (data[5]  << 8);
        dig_P1 = data[6]  | (data[7]  << 8);
        dig_P2 = data[8]  | (data[9]  << 8);
        dig_P3 = data[10] | (data[11] << 8);
        dig_P4 = data[12] | (data[13] << 8);
        dig_P5 = data[14] | (data[15] << 8);
        dig_P6 = data[16] | (data[17] << 8);
        dig_P7 = data[18] | (data[19] << 8);
        dig_P8 = data[20] | (data[21] << 8);
        dig_P9 = data[22] | (data[23] << 8);

        // dig_H1 = I2C::getRegister(BME280_ADDRESS, 0xA1);

        // I2C::getRegister(BME280_ADDRESS, 0xE1, data, 7);
        // dig_H2 = data[0]  | (data[1]  << 8);
        // dig_H3 = data[2];
        // dig_H4 = (data[3] << 4) | (data[4] & 0b1111);
        // dig_H5 = ((data[4] & 0b11110000) << 4) | data[5];
        // dig_H6 = data[6];

    }

    void measure(){
        I2C::writeRegister(BME280_ADDRESS, 0xF4, 0b10110101); //start measurement
    }

    double read_temperature(){
        uint8_t data[3];
        I2C::getRegister(BME280_ADDRESS, 0xFA, data, 3);
        int32_t adc_T = (data[0] << 12) | (data[1] << 4) | ((data[2] & 0b11110000) >> 4);

        int32_t var1, var2;
        var1 = ((((adc_T>>3) - ((int32_t)dig_T1<<1))) * ((int32_t)dig_T2)) >> 11;
        var2 = (((((adc_T>>4) - ((int32_t)dig_T1)) * ((adc_T>>4) - ((int32_t)dig_T1))) >> 12) *
        ((int32_t)dig_T3)) >> 14;
        t_fine = var1 + var2;
        double T = (t_fine * 5 + 128) >> 8;
        return T / 100.0;
    }

    double read_pressure(){
        uint8_t data[3];
        I2C::getRegister(BME280_ADDRESS, 0xF7, data, 3);
        int32_t adc_P = (data[0] << 12) | (data[1] << 4) | ((data[2] & 0b11110000) >> 4);

        int32_t var1, var2;
        uint32_t p;
        var1 = (((int32_t)t_fine)>>1) - (int32_t)64000;
        var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((int32_t)dig_P6);
        var2 = var2 + ((var1*((int32_t)dig_P5))<<1);
        var2 = (var2>>2)+(((int32_t)dig_P4)<<16);
        var1 = (((dig_P3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((int32_t)dig_P2) * var1)>>1))>>18;
        var1 =((((32768+var1))*((int32_t)dig_P1))>>15);
        if (var1 == 0)
        {
        return 0; // avoid exception caused by division by zero
        }
        p = (((uint32_t)(((int32_t)1048576)-adc_P)-(var2>>12)))*3125;
        if (p < 0x80000000)
        {
        p = (p << 1) / ((uint32_t)var1);
        }
        else
        {
        p = (p / (uint32_t)var1) * 2;
        }
        var1 = (((int32_t)dig_P9) * ((int32_t)(((p>>3) * (p>>3))>>13)))>>12;
        var2 = (((int32_t)(p>>2)) * ((int32_t)dig_P8))>>13;
        p = (uint32_t)((int32_t)p + ((var1 + var2 + dig_P7) >> 4));
        return p / 100.0;
    }

    double read_humidity(){
        uint8_t data[2];
        I2C::getRegister(BME280_ADDRESS, 0xFD, data, 2);
        int32_t adc_H = (data[0] << 8) | data[1];

        int32_t v_x1_u32r;
        v_x1_u32r = (t_fine - ((int32_t)76800));
        v_x1_u32r = (((((adc_H << 14) - (((int32_t)dig_H4) << 20) - (((int32_t)dig_H5) *
            v_x1_u32r)) + ((int32_t)16384)) >> 15) * (((((((v_x1_u32r *
            ((int32_t)dig_H6)) >> 10) * (((v_x1_u32r * ((int32_t)dig_H3)) >> 11) +
            ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)dig_H2) +
            8192) >> 14));
        v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
            ((int32_t)dig_H1)) >> 4));
        v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
        v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
        return (int32_t)(v_x1_u32r>>12) / 1024.0;
    }
}