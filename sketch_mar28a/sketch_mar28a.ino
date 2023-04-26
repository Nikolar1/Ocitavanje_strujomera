#include "esp_camera.h"
#include "Arduino.h"
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include "mbedtls/base64.h"
#include <FastCRC.h>

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

struct packet{
  unsigned int packetNumber;
  unsigned int numberOfPackets;
  uint16_t crc;
  uint8_t *payload;
};

FastCRC16 CRC16;
int timer = 0; 
camera_fb_t * fb = NULL;

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  Serial.begin(9600);

    camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; 
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}

packet* packetIt(uint8_t data[], size_t len){
  if(len == 0) return NULL;
  unsigned int numberOfPackets = 1 + (( len - 1) / 256); //Racuna broj paketa zaokruzen na gore;
  packet *packets = (packet *)malloc(sizeof(packet)*numberOfPackets);
  for(int i = 0; i<numberOfPackets; i++){
       uint8_t payload[256];
       for(int j = i*256; j<(i+1)*256; j++){
          if(j>len){
              payload[j-i*256] = 0;
              continue;
          }
          payload[j-i*256] = data[j]; 
       }
       packets[i].packetNumber = i;
       packets[i].numberOfPackets =  numberOfPackets;
       packets[i].payload = payload;
       packets[i].crc = CRC16.xmodem(payload, len);
  }
  return packets;
}

void loop() {

  if(Serial.available()) {
      String command = Serial.readStringUntil('\n');
      if (command == "1"){
        delay(1000);
        fb = esp_camera_fb_get();  
        if(!fb) {
          Serial.println("Camera capture failed");
          return;
        }
        packet *packets = packetIt(fb->buf, fb->len);

        //Test print
        /*for(int i =0; i<fb->len; i++){
          Serial.print(fb->buf[i], HEX);
        }*/
        Serial.println("");
        for(int i =0; i<256; i++){
          Serial.print(packets[0].payload[i], HEX);
        }
        Serial.println("");
        Serial.println(packets[0].crc, HEX);
        Serial.println("");
        //-----------------------------

        
        free(packets);
        esp_camera_fb_return(fb); 
        // Turns off the ESP32-CAM white on-board LED (flash) connected to GPIO 4
        pinMode(4, OUTPUT);
        digitalWrite(4, LOW);
        rtc_gpio_hold_en(GPIO_NUM_4);
        
      }
  }
}
