#ifndef I2C_H
#define I2C_H

#include <stdint.h>

namespace I2C {
    void init();
    uint8_t getRegister(uint8_t device, uint8_t reg);
    bool getRegister(uint8_t device, uint8_t reg, uint8_t * receiveData, int amount);
    void writeRegister(uint8_t device, uint8_t reg, uint8_t data);
    void writeRegister(uint8_t device, uint8_t reg, uint8_t dataBits, uint8_t data);
};

#endif