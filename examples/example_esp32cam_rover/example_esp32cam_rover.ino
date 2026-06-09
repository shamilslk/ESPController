/*
 * example_esp32cam_rover.ino
 * --------------------------
 * Full ESP32-CAM rover with:
 *   - WebSocket motor control
 *   - Live MJPEG camera streaming
 *   - WebSocket video transport
 *   - Camera settings (resolution, FPS, quality, flash)
 *   - mDNS: http://espcontroller.local
 *
 * Board: AI-Thinker ESP32-CAM
 * Partition scheme: "Huge APP (3MB No OTA)"
 *
 * Motor driver wiring (L298N example):
 *   Left  motor → IN1=GPIO14, IN2=GPIO15, EN=GPIO13
 *   Right motor → IN1=GPIO12, IN2=GPIO2,  EN=GPIO3
 *   (Adjust pins to match your actual wiring)
 *
 * Flash LED: GPIO4 (built-in)
 */

#include <ESPController.h>
#include "esp_camera.h"
#include <ESPmDNS.h>
#include <WebSocketsServer.h>

// ── WiFi credentials ─────────────────────────────
// ⚠️ Change these to your own AP credentials before flashing
#define WIFI_SSID     "MyRover"
#define WIFI_PASSWORD "12345678"

// ── Camera model (AI-Thinker ESP32-CAM) ─────────
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

// ── Motor pins ───────────────────────────────────
#define MOTOR_LEFT_IN1  14
#define MOTOR_LEFT_IN2  15
#define MOTOR_LEFT_EN   13
#define MOTOR_RIGHT_IN1 12
#define MOTOR_RIGHT_IN2  2
#define MOTOR_RIGHT_EN   3

#define FLASH_PIN        4

// ── Camera streaming WebSocket (port 82) ─────────
WebSocketsServer camWS(82);
bool camStreamActive = false;
uint8_t camClientId  = 0;

ESPController controller;

// ── Camera init ──────────────────────────────────
bool initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size   = FRAMESIZE_VGA;
  config.jpeg_quality = 12;
  config.fb_count     = 2;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("[CAM] Init failed: 0x%x\n", err);
    return false;
  }
  Serial.println("[CAM] Initialised");
  return true;
}

// ── Apply camera settings sent from the app ──────
// Payload format: "RES=VGA&FPS=20&Q=12&FLASH=50"
void applyCameraSettings(const String& settings) {
  sensor_t* s = esp_camera_sensor_get();
  if (!s) return;

  if (settings.indexOf("RES=QVGA") >= 0)  s->set_framesize(s, FRAMESIZE_QVGA);
  else if (settings.indexOf("RES=VGA") >= 0)  s->set_framesize(s, FRAMESIZE_VGA);
  else if (settings.indexOf("RES=SVGA") >= 0) s->set_framesize(s, FRAMESIZE_SVGA);

  int qIdx = settings.indexOf("Q=");
  if (qIdx >= 0) {
    int q = settings.substring(qIdx + 2).toInt();
    s->set_quality(s, constrain(q, 4, 63));
  }

  int flashIdx = settings.indexOf("FLASH=");
  if (flashIdx >= 0) {
    int brightness = settings.substring(flashIdx + 6).toInt();
    analogWrite(FLASH_PIN, map(brightness, 0, 100, 0, 255));
  }
}

// ── Camera WebSocket event ────────────────────────
void onCamWSEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      Serial.printf("[CAM-WS] Client #%u connected\n", num);
      camStreamActive = true;
      camClientId     = num;
      break;
    case WStype_DISCONNECTED:
      Serial.printf("[CAM-WS] Client #%u disconnected\n", num);
      camStreamActive = false;
      break;
    case WStype_TEXT:
      if (length > 0) {
        applyCameraSettings(String((char*)payload));
      }
      break;
    default:
      break;
  }
}

// ── ESPController command callback ───────────────
void onCommand(char cmd, const String& payload) {
  Serial.printf("[CMD] %c %s\n", cmd, payload.c_str());
}

void setup() {
  Serial.begin(115200);
  pinMode(FLASH_PIN, OUTPUT);
  analogWrite(FLASH_PIN, 0);

  if (!initCamera()) {
    Serial.println("[FATAL] Camera init failed — halting");
    while (true) { delay(1000); }
  }

  MotorPins left  = {MOTOR_LEFT_IN1,  MOTOR_LEFT_IN2,  MOTOR_LEFT_EN};
  MotorPins right = {MOTOR_RIGHT_IN1, MOTOR_RIGHT_IN2, MOTOR_RIGHT_EN};
  controller.setMotorPins(left, right);
  controller.setSpeed(200);
  controller.onCommand(onCommand);

  controller.beginAP(WIFI_SSID, WIFI_PASSWORD);

  // mDNS: espcontroller.local
  if (MDNS.begin("espcontroller")) {
    Serial.println("[mDNS] espcontroller.local");
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("ws",   "tcp", 81);
    MDNS.addService("ws",   "tcp", 82);
  }

  // Camera streaming WebSocket
  camWS.begin();
  camWS.onEvent(onCamWSEvent);

  Serial.println("=== ESP32-CAM Rover ready ===");
  Serial.print("Control: http://");
  Serial.println(controller.localIPString());
  Serial.println("mDNS:    http://espcontroller.local");
}

void loop() {
  controller.handle();
  camWS.loop();

  // Stream camera frame to connected client
  if (camStreamActive) {
    camera_fb_t* fb = esp_camera_fb_get();
    if (fb) {
      camWS.sendBIN(camClientId, fb->buf, fb->len);
      esp_camera_fb_return(fb);
    }
  }
}
