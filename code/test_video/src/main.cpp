/* test_video/src/main.cpp  -   Test code to record a .AVI clip on an sd card if present

  If a camera and sd card are present, JPEG pictures are taken and stuck together on the sd card, forming a .AVI clip. For a more
  detailed description of the functions and parameters, see test_video/lib/video/video.cpp

*/

#include <Arduino.h>
#include "video.h"

void setup() {
  Serial.begin(115200);
  Serial.println("\n\n---");

  video::initialize();
  video::init_sdcard();
}

void loop(){
  video::record("/sdcard/new_file.avi");
  //video::copy("/sdcard/new_file.avi", "/sdcard/new_file2.avi");

  // Read file to buffer in order to be able to send over mqtt
  int err;
  size_t f_size;
  char *f_data;

  f_data = video::c_read_file("/sdcard/new_file.avi", &err, &f_size);

  if (err) {
    Serial.printf("Error copying file: err%d \n",err);
    free(f_data);
  }
  else {
    Serial.println("File written to buffer");
    Serial.printf("Size = %d \n",f_size);
    free(f_data);
  }

  video::clear("/sdcard/new_file.avi");
  delay(5000);
}