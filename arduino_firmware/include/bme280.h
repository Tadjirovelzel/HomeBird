#ifndef BME280_H
#define BME280_H

namespace BME280 {
    void init();
    void measure();
    void read_calibration(uint8_t*);
    void read_temperature(uint8_t*);
    void read_pressure(uint8_t*);
    void read_humidity(uint8_t*);
};

#endif