#include <Arduino.h>
#include "bme280.h"
#include "Wire.h"
#include "i2c.h"

#include <PMserial.h> // Arduino library for PM sensors with serial interface

SerialPM pms(PMS5003, 25, 26);

// BME280 sensor module defective

void setup() {
  // I2C::init();
  // BME280::init();
  pms.init();
}

void loop() {
  // BME280::measure();
  // delay(100);
  // printf("%f %f %f\n", BME280::read_temperature(), BME280::read_pressure(), BME280::read_humidity());

  pms.read();
  if (pms){
    printf("PM1.0 %2d, PM2.5 %2d, PM10 %2d [ug/m3]\n",
                    pms.pm01, pms.pm25, pms.pm10);

    if (pms.has_number_concentration())
      printf("N0.3 %4d, N0.5 %3d, N1.0 %2d, N2.5 %2d, N5.0 %2d, N10 %2d [#/100cc]\n",
            pms.n0p3, pms.n0p5, pms.n1p0, pms.n2p5, pms.n5p0, pms.n10p0);
  }else{
    // something went wrong
    switch (pms.status)
    {
    case pms.OK: // should never come here
      break;     // included to compile without warnings
    case pms.ERROR_TIMEOUT:
      printf("%s\n", F(PMS_ERROR_TIMEOUT));
      break;
    case pms.ERROR_MSG_UNKNOWN:
      printf("%s\n", F(PMS_ERROR_MSG_UNKNOWN));
      break;
    case pms.ERROR_MSG_HEADER:
      printf("%s\n", F(PMS_ERROR_MSG_HEADER));
      break;
    case pms.ERROR_MSG_BODY:
      printf("%s\n", F(PMS_ERROR_MSG_BODY));
      break;
    case pms.ERROR_MSG_START:
      printf("%s\n", F(PMS_ERROR_MSG_START));
      break;
    case pms.ERROR_MSG_LENGTH:
      printf("%s\n", F(PMS_ERROR_MSG_LENGTH));
      break;
    case pms.ERROR_MSG_CKSUM:
      printf("%s\n", F(PMS_ERROR_MSG_CKSUM));
      break;
    case pms.ERROR_PMS_TYPE:
      printf("%s\n", F(PMS_ERROR_PMS_TYPE));
      break;
    }
  }

  // wait for 10 seconds
  delay(10000);
}