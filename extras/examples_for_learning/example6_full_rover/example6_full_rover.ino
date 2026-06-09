/*
 * example6_full_rover.ino
 * -----------------------
 * LEARNING EXAMPLE 6 — Full-featured rover with all callbacks.
 *
 * What you will learn:
 *   - All ESPController features combined
 *   - onCommand, onClientConnect, onClientDisconnect callbacks
 *   - Speed control via slider
 *   - Broadcasting telemetry back to the app
 *   - Periodic status updates using millis()
 *   - Emergency stop on client disconnect
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

// ── Telemetry interval ───────────────────────────
#define TELEMETRY_INTERVAL_MS 2000

ESPController controller;

unsigned long lastTelemetry = 0;
char currentMove = 'S';

// ── Command callback ─────────────────────────────
void onCommand(char cmd, const String& payload) {
  currentMove = cmd;

  // Log to Serial
  switch (cmd) {
    case 'F': Serial.println("[ROVER] Forward");       break;
    case 'B': Serial.println("[ROVER] Backward");      break;
    case 'L': Serial.println("[ROVER] Turn Left");     break;
    case 'R': Serial.println("[ROVER] Turn Right");    break;
    case 'S': Serial.println("[ROVER] Stop");          break;
    case 'J': Serial.printf("[ROVER] Joystick %s\n", payload.c_str()); break;
    case 'V': Serial.printf("[ROVER] Speed → %s\n",  payload.c_str()); break;
    default:  Serial.printf("[ROVER] Unknown cmd: %c\n", cmd);          break;
  }
}

// ── Connection callbacks ─────────────────────────
void onConnect(uint8_t id) {
  Serial.printf("[WS] App connected  (client #%u, total=%u)\n",
                id, controller.connectedClients());

  // Send welcome + current state
  String welcome = "{\"event\":\"connected\",\"ip\":\"";
  welcome += controller.localIPString();
  welcome += "\",\"speed\":";
  welcome += controller.getSpeed();
  welcome += "}";
  controller.sendMessage(id, welcome);
}

void onDisconnect(uint8_t id) {
  Serial.printf("[WS] App disconnected (client #%u)\n", id);
  // Safety: stop motors if no clients remain
  if (controller.connectedClients() == 0) {
    controller.motorStop();
    Serial.println("[ROVER] No clients — motors stopped for safety");
  }
}

// ── Periodic telemetry ───────────────────────────
void sendTelemetry() {
  if (controller.connectedClients() == 0) return;

  String tele = "{\"event\":\"telemetry\",\"move\":\"";
  tele += currentMove;
  tele += "\",\"speed\":";
  tele += controller.getSpeed();
  tele += ",\"clients\":";
  tele += controller.connectedClients();
  tele += "}";

  controller.broadcast(tele);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== Full Rover Example ===");

  MotorPins left  = {LEFT_IN1,  LEFT_IN2,  LEFT_EN};
  MotorPins right = {RIGHT_IN1, RIGHT_IN2, RIGHT_EN};

  controller.setMotorPins(left, right);
  controller.setSpeed(200);

  controller.onCommand(onCommand);
  controller.onClientConnect(onConnect);
  controller.onClientDisconnect(onDisconnect);

  controller.beginAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connect to WiFi: ");
  Serial.println(WIFI_SSID);
  Serial.print("Control: http://");
  Serial.println(controller.localIPString());
  Serial.println("Ready — waiting for app connection.");
}

void loop() {
  controller.handle();

  // Periodic telemetry broadcast
  if (millis() - lastTelemetry >= TELEMETRY_INTERVAL_MS) {
    lastTelemetry = millis();
    sendTelemetry();
  }
}
