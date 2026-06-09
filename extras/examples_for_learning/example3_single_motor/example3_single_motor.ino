/*
 * example3_single_motor.ino
 * -------------------------
 * LEARNING EXAMPLE 3 — Drive one motor with the D-Pad.
 *
 * What you will learn:
 *   - Configuring motor pins with MotorPins struct
 *   - Forward / Backward / Stop on one motor
 *   - How setSpeed() affects PWM output
 *
 * Board: Any ESP8266 or ESP32
 *
 * Wiring (L298N single channel):
 *   IN1 → GPIO14
 *   IN2 → GPIO12
 *   EN  → GPIO13  (PWM)
 *   Motor power supply: 6–12 V to L298N Vs pin
 *
 * ⚠️ Change WIFI_SSID and WIFI_PASSWORD before flashing!
 */

#include <ESPController.h>

// ── WiFi credentials (Access Point) ─────────────
// ⚠️ Change these to your own AP credentials before flashing
#define WIFI_SSID     "ESPController"
#define WIFI_PASSWORD "12345678"

// ── Single motor pins ────────────────────────────
#define MOTOR_IN1 14
#define MOTOR_IN2 12
#define MOTOR_EN  13

ESPController controller;

void onCommand(char cmd, const String& payload) {
  Serial.printf("[CMD] %c\n", cmd);
}

void setup() {
  Serial.begin(115200);

  // Use the same pins for both "left" and "right" so the
  // library drives a single motor regardless of direction command.
  MotorPins motor = {MOTOR_IN1, MOTOR_IN2, MOTOR_EN};
  controller.setMotorPins(motor, motor);
  controller.setSpeed(200);
  controller.onCommand(onCommand);

  controller.beginAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("=== Single Motor Test ready ===");
  Serial.print("Control: http://");
  Serial.println(controller.localIPString());
  Serial.println("Forward/Backward to spin, Stop to halt.");
}

void loop() {
  controller.handle();
}
