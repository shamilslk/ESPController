/*
 * example_esp32_rover.ino
 * -----------------------
 * ESP32 rover without camera.
 *   - WebSocket + HTTP control
 *   - D-Pad and joystick
 *   - mDNS: http://espcontroller.local
 *
 * Board: ESP32 Dev Module (or any ESP32 variant)
 *
 * Motor driver wiring (L298N / L293D):
 *   Left  motor → IN1=GPIO14, IN2=GPIO27, EN=GPIO26
 *   Right motor → IN1=GPIO25, IN2=GPIO33, EN=GPIO32
 */

#include <ESPController.h>
#include <ESPmDNS.h>

// ── WiFi (Access Point) ──────────────────────────
// ⚠️ Change these to your own AP credentials before flashing
#define WIFI_SSID     "ESP32Rover"
#define WIFI_PASSWORD "12345678"

// ── Motor pins ───────────────────────────────────
#define LEFT_IN1  14
#define LEFT_IN2  27
#define LEFT_EN   26
#define RIGHT_IN1 25
#define RIGHT_IN2 33
#define RIGHT_EN  32

ESPController controller;

void onCommand(char cmd, const String& payload) {
  // React to commands here, e.g. trigger a sensor reading
  Serial.printf("[CMD] %c  payload='%s'\n", cmd, payload.c_str());
}

void onConnect(uint8_t id) {
  Serial.printf("[WS] App connected (client %u)\n", id);
  controller.sendMessage(id, "Hello from ESP32 rover!");
}

void onDisconnect(uint8_t id) {
  Serial.printf("[WS] App disconnected (client %u)\n", id);
}

void setup() {
  Serial.begin(115200);

  MotorPins left  = {LEFT_IN1,  LEFT_IN2,  LEFT_EN};
  MotorPins right = {RIGHT_IN1, RIGHT_IN2, RIGHT_EN};

  controller.setMotorPins(left, right);
  controller.setSpeed(180);
  controller.onCommand(onCommand);
  controller.onClientConnect(onConnect);
  controller.onClientDisconnect(onDisconnect);

  controller.beginAP(WIFI_SSID, WIFI_PASSWORD);

  if (MDNS.begin("espcontroller")) {
    Serial.println("[mDNS] espcontroller.local active");
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("ws",   "tcp", 81);
  }

  Serial.println("=== ESP32 Rover ready ===");
  Serial.print("Control: http://");
  Serial.println(controller.localIPString());
}

void loop() {
  controller.handle();
}
