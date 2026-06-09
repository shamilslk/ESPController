#ifndef ESPController_h
#define ESPController_h

/*
 * ESPController Library
 * ---------------------
 * A WebSocket + HTTP controller library for ESP8266 / ESP32 rover robots.
 *
 * Features:
 *  - WebSocket control channel (low-latency commands)
 *  - HTTP fallback command endpoint
 *  - Motor control helpers (forward, backward, left, right, stop)
 *  - Joystick X/Y → differential drive mapping
 *  - Dual-motor PWM with adjustable speed
 *  - Built-in HTML controller page served from flash
 *
 * Supported boards:
 *  - ESP8266 (NodeMCU, Wemos D1 Mini, etc.)
 *  - ESP32 / ESP32-CAM
 *
 * Author : ESPController Project
 * License: MIT
 */

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  #include <WebSocketsServer.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <WebServer.h>
  #include <WebSocketsServer.h>
#else
  #error "ESPController requires ESP8266 or ESP32"
#endif

#include <Arduino.h>

// ──────────────────────────────────────────────
//  Motor channel descriptor
// ──────────────────────────────────────────────
struct MotorPins {
  uint8_t in1;   // Direction pin A
  uint8_t in2;   // Direction pin B
  uint8_t en;    // PWM enable pin (255 = not used / always-on)
};

// ──────────────────────────────────────────────
//  Command IDs (must match the Android app)
// ──────────────────────────────────────────────
#define CMD_STOP      'S'
#define CMD_FORWARD   'F'
#define CMD_BACKWARD  'B'
#define CMD_LEFT      'L'
#define CMD_RIGHT     'R'
#define CMD_JOYSTICK  'J'   // payload: "J<x>,<y>"  (-100…+100 each)
#define CMD_SPEED     'V'   // payload: "V<0-255>"

// ──────────────────────────────────────────────
//  ESPController class
// ──────────────────────────────────────────────
class ESPController {
public:
  // ── Construction ────────────────────────────
  ESPController();

  // ── WiFi / server setup ─────────────────────
  /**
   * Start in Access Point mode.
   * @param ssid       AP network name
   * @param password   AP password (empty = open network)
   * @param port       HTTP port (default 80)
   * @param wsPort     WebSocket port (default 81)
   */
  void beginAP(const char* ssid,
               const char* password = "",
               uint16_t    port     = 80,
               uint16_t    wsPort   = 81);

  /**
   * Join an existing WiFi network (STA mode).
   * Blocks until connected or timeout (ms).
   */
  bool beginSTA(const char* ssid,
                const char* password,
                uint32_t    timeoutMs = 10000,
                uint16_t    port      = 80,
                uint16_t    wsPort    = 81);

  /** Call once in loop() to process network events. */
  void handle();

  // ── Motor configuration ──────────────────────
  /**
   * Configure left and right motor pins.
   * Call before begin*().
   */
  void setMotorPins(MotorPins left, MotorPins right);

  /** Set a global speed multiplier (0–255, default 200). */
  void setSpeed(uint8_t speed);
  uint8_t getSpeed() const { return _speed; }

  // ── Direct motor commands ────────────────────
  void motorForward();
  void motorBackward();
  void motorLeft();
  void motorRight();
  void motorStop();

  /**
   * Joystick differential drive.
   * @param x  Horizontal axis -100 … +100  (right = positive)
   * @param y  Vertical axis   -100 … +100  (forward = positive)
   */
  void motorJoystick(int x, int y);

  // ── Callbacks ───────────────────────────────
  /** Called whenever a command is received (char cmd, String payload). */
  void onCommand(void (*callback)(char cmd, const String& payload));

  /** Called on WebSocket connect / disconnect. */
  void onClientConnect(void (*callback)(uint8_t clientId));
  void onClientDisconnect(void (*callback)(uint8_t clientId));

  // ── Status ──────────────────────────────────
  bool     isConnected()    const;
  uint8_t  connectedClients() const { return _connectedClients; }
  IPAddress localIP()       const;
  String    localIPString() const;

  // ── Utility ─────────────────────────────────
  /** Send a string message to a specific WebSocket client. */
  void sendMessage(uint8_t clientId, const String& msg);
  /** Broadcast a string message to all connected WebSocket clients. */
  void broadcast(const String& msg);

private:
  // Internal helpers
  void _setupRoutes();
  void _handleRoot();
  void _handleCommand();
  void _handleStatus();
  void _onWebSocketEvent(uint8_t num, WStype_t type,
                         uint8_t* payload, size_t length);
  void _processCommand(char cmd, const String& payload);
  void _driveMotor(const MotorPins& m, int8_t direction, uint8_t pwm);
  int  _clamp(int v, int lo, int hi);

  // Server objects
#if defined(ESP8266)
  ESP8266WebServer* _server;
#else
  WebServer*        _server;
#endif
  WebSocketsServer* _ws;

  uint16_t _port;
  uint16_t _wsPort;

  // Motor state
  MotorPins _leftMotor;
  MotorPins _rightMotor;
  bool      _motorsConfigured;
  uint8_t   _speed;

  // Callbacks
  void (*_onCommandCb)(char, const String&);
  void (*_onConnectCb)(uint8_t);
  void (*_onDisconnectCb)(uint8_t);

  uint8_t _connectedClients;
};

#endif // ESPController_h
