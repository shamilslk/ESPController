/*
 * example_esp8266.ino
 * -------------------
 * ESP8266 rover controller.
 *   - WebSocket + HTTP control
 *   - D-Pad and joystick differential drive
 *   - mDNS: http://espcontroller.local
 *
 * Board: NodeMCU 1.0 (ESP-12E) / Wemos D1 Mini / any ESP8266
 *
 * Motor driver wiring (L298N / L293D):
 *   Left  motor → IN1=D1(GPIO5), IN2=D2(GPIO4), EN=D3(GPIO0)
 *   Right motor → IN1=D5(GPIO14), IN2=D6(GPIO12), EN=D7(GPIO13)
 *
 * ⚠️ Change WIFI_SSID and WIFI_PASSWORD before flashing!
 */

#include <ESPController.h>
#include <ESP8266mDNS.h>

// ── WiFi credentials (Access Point) ─────────────
// ⚠️ Change these to your own AP credentials before flashing
#define WIFI_SSID     "ESP8266Rover"
#define WIFI_PASSWORD "12345678"

// ── Motor pins (NodeMCU GPIO numbers) ────────────
#define LEFT_IN1   5   // D1
#define LEFT_IN2   4   // D2
#define LEFT_EN    0   // D3
#define RIGHT_IN1 14   // D5
#define RIGHT_IN2 12   // D6
#define RIGHT_EN  13   // D7

ESPController controller;

void onCommand(char cmd, const String& payload) {
  Serial.printf("[CMD] %c  payload='%s'\n", cmd, payload.c_str());
}

void setup() {
  Serial.begin(115200);

  MotorPins left  = {LEFT_IN1,  LEFT_IN2,  LEFT_EN};
  MotorPins right = {RIGHT_IN1, RIGHT_IN2, RIGHT_EN};

  controller.setMotorPins(left, right);
  controller.setSpeed(180);
  controller.onCommand(onCommand);

  controller.beginAP(WIFI_SSID, WIFI_PASSWORD);

  if (MDNS.begin("espcontroller")) {
    Serial.println("[mDNS] espcontroller.local active");
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("ws",   "tcp", 81);
  }

  Serial.println("=== ESP8266 Rover ready ===");
  Serial.print("Control: http://");
  Serial.println(controller.localIPString());
}

void loop() {
  controller.handle();
  MDNS.update();  // Required on ESP8266
}
