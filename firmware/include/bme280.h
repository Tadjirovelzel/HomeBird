#ifndef BME280_H
#define BME280_H

namespace BME280 {
    void init();
    void measure();
    double read_temperature();
    double read_pressure();
    double read_humidity();
};

#endif