#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>
//#include <Arduino_JSON.h> // not needed, now might bring back in future
#include <WiFiManager.h> //update to version 2.0.12 using manage libraries to fix compile errors.


/*

Camera example modified by Walter Schreppers to post IP address to a Fast-API server running on port 3000

ALSO add following extra board with board manager:

Click on the File menu on the top menu bar.
Click on the Preferences menu item. This will open a Preferences dialog box.
You should be on the Settings tab in the Preferences dialog box by default.
Look for the textbox labeled “Additional Boards Manager URLs”.
If there is already text in this box add a coma at the end of it, then follow the next step.
Paste the following link into the text box – https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/packahbjhb jmm7j5reriorn5b54 78     dhglpji';kl'\kl"|jk;o:jn/l n  ne_esp32_index.json


Choose board: AI Thinker ESP32-CAM and a serial port cu.usbmodem1410 (by plugging board in using usb data cable)

TODO: Use wifi manager as it also supplies ota capabilities. And someone already made a tutorial for this exact
usecase https://dronebotworkshop.com/wifimanager/
-> link a button this makes it go into access point mode where you can both update the firmware and also
set new wifi creds. This is ideal. We then have a button to both update the device firmware and setup wifi.

*/


//
// WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
//            Ensure ESP32 Wrover Module or other board with PSRAM is selected
//            Partial images will be transmitted if image exceeds buffer size
//
//            You must select partition scheme from the board menu that has at least 3MB APP space.
//            Face Recognition is DISABLED for ESP32 and ESP32-S2, because it takes up from 15 
//            seconds to process single frame. Face Detection is ENABLED if PSRAM is enabled as well

// ===================
// Select camera model
// ===================
//#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
//#define CAMERA_MODEL_ESP32S3_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
//#define CAMERA_MODEL_M5STACK_UNITCAM // No PSRAM
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM
// ** Espressif Internal Boards **
//#define CAMERA_MODEL_ESP32_CAM_BOARD
//#define CAMERA_MODEL_ESP32S2_CAM_BOARD
//#define CAMERA_MODEL_ESP32S3_CAM_LCD

#include "camera_pins.h"

#define FLASH_GPIO_NUM 4  // ESP32-CAM GPIO 4 (flashlight)

// ===========================
// Enter your WiFi credentials
// ===========================
const char* ssid = "telenet-1443323";
const char* password = "uu4xawdvYYnf";

// TODO: change this to an actual public running server in future, for now just connect to laptop
const char* serverName = "http://192.168.0.215:3000/camera_update";

// TODO: OTA https://lastminuteengineers.com/esp32-ota-web-updater-arduino-ide/


void startCameraServer();

String IpAddressToString(const IPAddress& ipAddress) {
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ;
}

void setup() {
  Serial.begin(115200);
  //Serial.begin(19200);

  Serial.setDebugOutput(true);
  Serial.println();

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
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;
  
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if(config.pixel_format == PIXFORMAT_JPEG){
    if(psramFound()){
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
      // Limit the frame size when PSRAM is not available
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  } else {
    // Best option for face detection/recognition
    config.frame_size = FRAMESIZE_240X240;
#if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 2;
#endif
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  if(config.pixel_format == PIXFORMAT_JPEG){
    s->set_framesize(s, FRAMESIZE_QVGA);
  }

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

#if defined(CAMERA_MODEL_ESP32S3_EYE)
  s->set_vflip(s, 1);
#endif

  // WiFi.begin(ssid, password);
  WiFiManager wm;
     
  // initialize digital pin ledPin as an output
  pinMode(FLASH_GPIO_NUM, OUTPUT);

  // wm.resetSettings(); //disable this in production, good for testing now
  bool res;
  res = wm.autoConnect("WalterCamera", "password");
  if(!res){
    Serial.println("WiFiManager connection failed");
    
    for(int i=0;i<3;i++){
      digitalWrite(FLASH_GPIO_NUM, HIGH);
      delay(500);
      digitalWrite(FLASH_GPIO_NUM, LOW); //turn flash back off, we know its ready now
      delay(100);
    }
  }
  
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");


  // Post ip to some server configured
  Serial.print("Posting IP to server ");
  Serial.print(serverName);
  Serial.println("...");
  HTTPClient http;
  http.begin(serverName);
  http.addHeader("Content-Type", "application/json");
  
  String json_data = "{\"api_key\":\"some_api_key_here\",\"IP\":\""+ IpAddressToString( WiFi.localIP() ) +"\"}";
  int response_code = http.POST(json_data);
  
  Serial.print("Server Response code: ");
  Serial.println(response_code);
  http.end();

  // also print ip to serial
  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");

  startCameraServer();

  for(int i=0;i<2;i++){
    digitalWrite(FLASH_GPIO_NUM, HIGH);
    delay(500);
    digitalWrite(FLASH_GPIO_NUM, LOW); //turn flash back off, we know its ready now
    delay(100);
  }

}

void loop() {
  // Do nothing. Everything is done in another task by the web server
  delay(10000);
}
