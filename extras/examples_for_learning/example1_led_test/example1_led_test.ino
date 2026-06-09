/*
 * example1_led_test.ino
 * ---------------------
 * LEARNING EXAMPLE 1 — Blink the built-in LED via WebSocket.
 *
 * What you will learn:
 *   - Starting ESPController in AP mode
 *   - Connecting from a browser or the app
 *   - Receiving commands with onCommand()
 *   - Responding to 'F' (LED ON) and 'S' (LED OFF)
 *
 * Board: Any ESP8266 or ESP32
 * No motor driver needed — just the built-in LED.
 *
 * ⚠️ Change WIFI_SSID and WIFI_PASSWORD before flashing!
 */

#include <ESPController.h>

// ── WiFi credentials (Access Point) ─────────────
// ⚠️ Change these to your own AP credentials before flashing
#define WIFI_SSID     "ESPController"
#define WIFI_PASSWORD "12345678"

#if defined(ESP32)
  #define LED_PIN 2   // GPIO2 on most ESP32 boards
#else
  #define LED_PIN LED_BUILTIN
#endif

ESPController controller;

void onCommand(char cmd, const String& payload) {
  switch (cmd) {
    case 'F':  // Forward → LED ON
      digitalWrite(LED_PIN, LOW);   // active-low on most boards
      Serial.println("LED ON");
      break;
    case 'S':  // Stop → LED OFF
      digitalWrite(LED_PIN, HIGH);
      Serial.println("LED OFF");
      break;
    default:
      Serial.printf("Unknown command: %c\n", cmd);
      break;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  // Start with LED off

  controller.onCommand(onCommand);
  controller.beginAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("=== LED Test ready ===");
  Serial.print("Open: http://");
  Serial.println(controller.localIPString());
  Serial.println("Press Forward to turn LED on, Stop to turn it off.");
}

void loop() {
  controller.handle();
}
