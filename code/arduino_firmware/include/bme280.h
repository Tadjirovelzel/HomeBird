#ifndef BME280_H
#define BME280_H

namespace BME280 {
    void init(int);
    void measure(int);
    void read_calibration(int, uint8_t*);
    void read_temperature(int, uint8_t*);
    void read_pressure(int, uint8_t*);
    void read_humidity(int, uint8_t*);
};

#endif