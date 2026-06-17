/*
 * ============================================================
 *  ESPController Library
 *  Universal ESP32-CAM / ESP32 / ESP8266 WebSocket Controller
 * ------------------------------------------------------------
 *  Authors : Shamil K, Vismaya P
 *  Version : 2.0.0
 *  GitHub  : https://github.com/shamilslk/ESPController
 *  License : MIT — free to use, modify and distribute
 *            with credit to the original authors.
 * ============================================================
 */

#ifndef ESPController_h
#define ESPController_h

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  #include <ESP8266mDNS.h>
  #include <WebSocketsServer.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <WebServer.h>
  #include <ESPmDNS.h>
  #include <WebSocketsServer.h>
#else
  #error "ESPController requires ESP8266 or ESP32"
#endif

#include <Arduino.h>

// ──────────────────────────────────────────────
//  WiFi mode
// ──────────────────────────────────────────────
enum class ControllerWiFiMode {
  AP,
  STA
};

// ──────────────────────────────────────────────
//  Motor side selector
// ──────────────────────────────────────────────
enum class MotorSide {
  LEFT,
  RIGHT
};

// ──────────────────────────────────────────────
//  Internal motor channel descriptor
// ──────────────────────────────────────────────
struct _MotorChannel {
  uint8_t in1;
  uint8_t in2;
  int8_t  trim;      // -100 … +100 drift correction
  bool    configured;
};

// ──────────────────────────────────────────────
//  ESPController class
// ──────────────────────────────────────────────
class ESPControllerClass {
public:
  ESPControllerClass();

  // ── WiFi / server setup ─────────────────────
  /**
   * Configure WiFi credentials.
   * Mode defaults to AP (creates hotspot).
   * Call before begin().
   */
  void setWiFi(const char* ssid,
               const char* password,
               ControllerWiFiMode mode = ControllerWiFiMode::AP);

  /**
   * Set mDNS hostname — device reachable at http://<name>.local
   * Call before begin().
   */
  void setMDNS(const char* hostname);

  /**
   * Start WiFi, WebSocket server and HTTP server.
   * Call at end of setup().
   */
  void begin(uint16_t httpPort = 80, uint16_t wsPort = 81);

  /**
   * Drive the network stack — call every loop().
   */
  void update();

  // ── Motor pin configuration ──────────────────
  /** Set GPIO pins for motor A (left). */
  void setMotorAPins(uint8_t in1, uint8_t in2);

  /** Set GPIO pins for motor B (right). */
  void setMotorBPins(uint8_t in1, uint8_t in2);

  /** Global speed cap 0–255. Default 200. */
  void setMaxSpeed(uint8_t speed);

  /**
   * Correct physical drift on one side.
   * @param side   MotorSide::LEFT or MotorSide::RIGHT
   * @param trim   -100 (slow down) … 0 (no correction) … +100 (speed up)
   */
  void setMotorTrim(MotorSide side, int8_t trim);

  /**
   * Motor watchdog — stop motors automatically if no command
   * is received for this many milliseconds. 0 = disabled.
   */
  void setWatchdogTimeout(uint32_t ms);

  /**
   * Soft acceleration ramp.
   * @param step    PWM units to change per tick (255 = instant)
   * @param tickMs  How often (ms) to advance the ramp
   */
  void setAcceleration(uint8_t step, uint16_t tickMs);

  // ── Direct motor control ─────────────────────
  /**
   * Set motor A speed directly.
   * @param speed  -255 (full reverse) … +255 (full forward)
   * Trim, maxSpeed and acceleration ramp are applied automatically.
   */
  void setMotorA(int speed);

  /**
   * Set motor B speed directly.
   * @param speed  -255 (full reverse) … +255 (full forward)
   */
  void setMotorB(int speed);

  /** Gradual stop — respects acceleration ramp. */
  void stopMotors();

  /** Instant cut — bypasses ramp. Also kills flash LED if present. */
  void emergencyStop();

  // ── D-Pad callbacks ──────────────────────────
  // state = HIGH on press, LOW auto-fired after 300 ms
  void onUp   (void (*cb)(uint8_t state));
  void onDown (void (*cb)(uint8_t state));
  void onLeft (void (*cb)(uint8_t state));
  void onRight(void (*cb)(uint8_t state));
  // state = HIGH on press only
  void onStop (void (*cb)(uint8_t state));

  // ── Joystick callbacks ───────────────────────
  /** Integer joystick: x,y each −255…+255 */
  void onJoystick   (void (*cb)(int x, int y));
  /** Raw float joystick: x,y each −1.0…+1.0 */
  void onJoystickRaw(void (*cb)(float x, float y));

  // ── Connection callbacks ─────────────────────
  void onControllerConnected   (void (*cb)());
  void onControllerDisconnected(void (*cb)());

  // ── Battery monitor ──────────────────────────
  /**
   * Fire cb when ADC pin voltage drops below threshold.
   * @param pin        Analog pin number
   * @param thresholdMv  Threshold in millivolts
   * @param cb           Callback fired once per crossing
   */
  void onLowBattery(uint8_t pin, uint16_t thresholdMv, void (*cb)());

  // ── Status / connectivity ────────────────────
  int     getRSSI()        const;
  uint8_t getRSSIQuality() const;
  String  getStatusJSON()  const;
  bool    isConnected()    const;
  IPAddress localIP()      const;
  String    localIPString()const;

  // ── Messaging ────────────────────────────────
  void sendMessage(uint8_t clientId, const String& msg);
  void broadcast  (const String& msg);

private:
  // WiFi config
  const char*        _ssid;
  const char*        _password;
  ControllerWiFiMode _wifiMode;
  const char*        _mdnsName;
  uint16_t           _httpPort;
  uint16_t           _wsPort;

  // Server objects
#if defined(ESP8266)
  ESP8266WebServer* _server;
#else
  WebServer*        _server;
#endif
  WebSocketsServer* _ws;

  // Motor state
  _MotorChannel _motorA;   // left
  _MotorChannel _motorB;   // right
  uint8_t  _maxSpeed;
  int      _targetA;       // desired speed -255…+255
  int      _targetB;
  int      _currentA;      // ramped speed
  int      _currentB;
  uint8_t  _accelStep;
  uint16_t _accelTickMs;
  uint32_t _lastAccelTick;

  // Watchdog
  uint32_t _watchdogMs;
  uint32_t _lastCmdTime;

  // RSSI broadcast
  uint32_t _lastRssiBroadcast;

  // Button auto-release tracking
  uint32_t _btnPressTime[4];   // 0=Up 1=Down 2=Left 3=Right
  bool     _btnActive[4];

  // Connected clients
  uint8_t _connectedClients;

  // Battery monitor
  uint8_t  _battPin;
  uint16_t _battThresholdMv;
  bool     _battTriggered;

  // Callbacks
  void (*_cbUp)   (uint8_t);
  void (*_cbDown) (uint8_t);
  void (*_cbLeft) (uint8_t);
  void (*_cbRight)(uint8_t);
  void (*_cbStop) (uint8_t);
  void (*_cbJoystick)   (int, int);
  void (*_cbJoystickRaw)(float, float);
  void (*_cbConnected)  ();
  void (*_cbDisconnected)();
  void (*_cbLowBattery) ();

  // Internal helpers
  void _setupRoutes();
  void _handleRoot();
  void _handleHTTPCmd();
  void _handleStatus();
  void _onWsEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);
  void _processCommand(const String& raw);
  void _fireDPad(uint8_t index, void (*cb)(uint8_t));
  void _driveMotorHW(const _MotorChannel& m, int8_t dir, uint8_t pwm);
  void _applyRamp();
  void _checkWatchdog();
  void _checkBattery();
  void _checkBtnRelease();
  void _broadcastRSSI();
  int  _clamp(int v, int lo, int hi);
  uint8_t _scalePwm(int speed, int8_t trim);
};

// Global singleton — use as "Controller.xxx()"
extern ESPControllerClass Controller;

#endif // ESPController_h
