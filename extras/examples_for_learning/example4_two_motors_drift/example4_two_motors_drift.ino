/*
 * example4_two_motors_drift.ino
 * -----------------------------
 * LEARNING EXAMPLE 4 — Two motors: forward, backward, drift, spin.
 *
 * What you will learn:
 *   - Configuring separate left and right motor pins
 *   - How Left/Right commands create tank-style turns
 *   - Adjusting speed mid-session with the speed slider
 *   - Broadcasting status messages back to the app
 *
 * Board: Any ESP8266 or ESP32
 *
 * Wiring (L298N dual channel):
 *   Left  motor → IN1=GPIO14, IN2=GPIO12, EN=GPIO13
 *   Right motor → IN1=GPIO27, IN2=GPIO26, EN=GPIO25
 *
 * ⚠️ Change WIFI_SSID and WIFI_PASSWORD before flashing!
 */

#include <ESPController.h>

// ── WiFi credentials (Access Point) ─────────────
// ⚠️ Change these to your own AP credentials before flashing
#define WIFI_SSID     "ESPController"
#define WIFI_PASSWORD "12345678"

// ── Motor pins ───────────────────────────────────
#define LEFT_IN1  14
#define LEFT_IN2  12
#define LEFT_EN   13
#define RIGHT_IN1 27
#define RIGHT_IN2 26
#define RIGHT_EN  25

ESPController controller;

String lastMove = "STOP";

void onCommand(char cmd, const String& payload) {
  switch (cmd) {
    case 'F': lastMove = "FORWARD";  break;
    case 'B': lastMove = "BACKWARD"; break;
    case 'L': lastMove = "DRIFT LEFT";  break;
    case 'R': lastMove = "DRIFT RIGHT"; break;
    case 'S': lastMove = "STOP";     break;
    case 'V': lastMove = "SPEED=" + payload; break;
    default: break;
  }
  Serial.println(lastMove);
  controller.broadcast(lastMove);  // Echo to all connected clients
}

void setup() {
  Serial.begin(115200);

  MotorPins left  = {LEFT_IN1,  LEFT_IN2,  LEFT_EN};
  MotorPins right = {RIGHT_IN1, RIGHT_IN2, RIGHT_EN};

  controller.setMotorPins(left, right);
  controller.setSpeed(200);
  controller.onCommand(onCommand);

  controller.beginAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("=== Two Motor Drift Test ready ===");
  Serial.print("Control: http://");
  Serial.println(controller.localIPString());
}

void loop() {
  controller.handle();
}
