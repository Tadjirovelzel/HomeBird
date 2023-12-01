//*
// Modem pins
#define SerialMon Serial
#define uS_TO_S_FACTOR          1000000ULL  // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP           60          // Time ESP32 will go to sleep (in seconds) 

#define PIN_TX                  45
#define PIN_RX                  46
#define UART_BAUD               115200
#define PWR_PIN                 48
#define LED_PIN                 21


// Camera related
#include "esp_camera.h"
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "driver/rtc_io.h"


// Camera model
#define CAMERA_MODEL_AI_THINKER_MODIFIED

// Pin definition for camera models
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      14
#define SIOD_GPIO_NUM     4
#define SIOC_GPIO_NUM     5

#define Y9_GPIO_NUM       15
#define Y8_GPIO_NUM       16
#define Y7_GPIO_NUM       17
#define Y6_GPIO_NUM       12
#define Y5_GPIO_NUM       10
#define Y4_GPIO_NUM       8
#define Y3_GPIO_NUM       9
#define Y2_GPIO_NUM       11 
#define VSYNC_GPIO_NUM    6
#define HREF_GPIO_NUM     7
#define PCLK_GPIO_NUM     13

static camera_config_t camera_config = {
    .pin_pwdn = PWDN_GPIO_NUM,
    .pin_reset = RESET_GPIO_NUM,
    .pin_xclk = XCLK_GPIO_NUM,
    .pin_sccb_sda = SIOD_GPIO_NUM,
    .pin_sccb_scl = SIOC_GPIO_NUM,
    .pin_d7 = Y9_GPIO_NUM,
    .pin_d6 = Y8_GPIO_NUM,
    .pin_d5 = Y7_GPIO_NUM,
    .pin_d4 = Y6_GPIO_NUM,
    .pin_d3 = Y5_GPIO_NUM,
    .pin_d2 = Y4_GPIO_NUM,
    .pin_d1 = Y3_GPIO_NUM,
    .pin_d0 = Y2_GPIO_NUM,
    .pin_vsync = VSYNC_GPIO_NUM,
    .pin_href = HREF_GPIO_NUM,
    .pin_pclk = PCLK_GPIO_NUM,

    .xclk_freq_hz = 12000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,
    .pixel_format = PIXFORMAT_JPEG,
    //.pixel_format = PIXFORMAT_RGB565,
    .frame_size = FRAMESIZE_VGA,
    .jpeg_quality = 50,
    .fb_count = 1,
    .grab_mode = CAMERA_GRAB_LATEST // When buffers should be filled CAMERA_GRAB_WHEN_EMPTY
    //.fb_location    = CAMERA_FB_IN_PSRAM, // The location where the frame buffer will be allocated
};

//*/