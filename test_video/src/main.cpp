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
  //video::copy("/sdcard/new_file.avi", "/spiffs/new_file.avi");
  video::clear("/sdcard/new_file.avi");
  delay(5000);
}