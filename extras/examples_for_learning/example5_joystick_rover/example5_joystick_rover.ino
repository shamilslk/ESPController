/*
 * example5_joystick_rover.ino
 * ---------------------------
 * LEARNING EXAMPLE 5 — Joystick-only differential drive.
 *
 * What you will learn:
 *   - How the joystick X/Y values map to left/right motor speeds
 *   - Differential drive mixing (motorJoystick)
 *   - Printing live joystick values to Serial Monitor
 *   - Ignoring D-Pad commands and using only the joystick
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

void onCommand(char cmd, const String& payload) {
  if (cmd == 'J') {
    // Parse "x,y" payload
    int comma = payload.indexOf(',');
    if (comma > 0) {
      int x = payload.substring(0, comma).toInt();
      int y = payload.substring(comma + 1).toInt();
      Serial.printf("[JOY] x=%4d  y=%4d\n", x, y);
      // motorJoystick is already called internally by the library,
      // but we can also call it manually after custom logic:
      // controller.motorJoystick(x, y);
    }
  } else if (cmd == 'S') {
    Serial.println("[JOY] Released — STOP");
  }
  // D-Pad commands (F/B/L/R) are intentionally ignored here.
  // The library still processes them internally; override
  // motorForward() etc. if you need different behaviour.
}

void setup() {
  Serial.begin(115200);

  MotorPins left  = {LEFT_IN1,  LEFT_IN2,  LEFT_EN};
  MotorPins right = {RIGHT_IN1, RIGHT_IN2, RIGHT_EN};

  controller.setMotorPins(left, right);
  controller.setSpeed(220);
  controller.onCommand(onCommand);

  controller.beginAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("=== Joystick Rover ready ===");
  Serial.print("Control: http://");
  Serial.println(controller.localIPString());
  Serial.println("Use the virtual joystick — values print here.");
}

void loop() {
  controller.handle();
}
