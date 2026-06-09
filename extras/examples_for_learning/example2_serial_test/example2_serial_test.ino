/*
 * example2_serial_test.ino
 * ------------------------
 * LEARNING EXAMPLE 2 — Echo every command to Serial Monitor.
 *
 * What you will learn:
 *   - How commands arrive from the app/browser
 *   - The command character + payload format
 *   - Using the onCommand callback for debugging
 *   - Checking connection status
 *
 * Board: Any ESP8266 or ESP32
 * No motor driver needed — open Serial Monitor at 115200 baud.
 *
 * ⚠️ Change WIFI_SSID and WIFI_PASSWORD before flashing!
 */

#include <ESPController.h>

// ── WiFi credentials (Access Point) ─────────────
// ⚠️ Change these to your own AP credentials before flashing
#define WIFI_SSID     "ESPController"
#define WIFI_PASSWORD "12345678"

ESPController controller;

// Map command char to human-readable name
String cmdName(char cmd) {
  switch (cmd) {
    case 'F': return "FORWARD";
    case 'B': return "BACKWARD";
    case 'L': return "LEFT";
    case 'R': return "RIGHT";
    case 'S': return "STOP";
    case 'J': return "JOYSTICK";
    case 'V': return "SPEED";
    default:  return String("UNKNOWN(") + cmd + ")";
  }
}

void onCommand(char cmd, const String& payload) {
  Serial.print("[CMD] ");
  Serial.print(cmdName(cmd));
  if (payload.length() > 0) {
    Serial.print("  payload=");
    Serial.print(payload);
  }
  Serial.println();
}

void onConnect(uint8_t id) {
  Serial.printf("[WS]  Client #%u CONNECTED  (total: %u)\n",
                id, controller.connectedClients());
  controller.sendMessage(id, "Serial test ready!");
}

void onDisconnect(uint8_t id) {
  Serial.printf("[WS]  Client #%u DISCONNECTED\n", id);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== Serial Echo Test ===");

  controller.onCommand(onCommand);
  controller.onClientConnect(onConnect);
  controller.onClientDisconnect(onDisconnect);

  controller.beginAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connect to WiFi: ");
  Serial.println(WIFI_SSID);
  Serial.print("Open: http://");
  Serial.println(controller.localIPString());
  Serial.println("All commands will be printed here.");
}

void loop() {
  controller.handle();
}
