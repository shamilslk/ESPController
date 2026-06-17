
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

// ============================================================
//  ESPController.h  —  Universal ESP controller library
//  Works on: ESP32-CAM, ESP32, ESP8266
//
//  STEP 1: Uncomment the flags that match your board/setup
//  STEP 2: If using ESP32-CAM, set your camera pins below
//  STEP 3: Call Controller.begin() and Controller.update()
// ============================================================
#pragma once

// ============================================================
//  >>>  BOARD CONFIGURATION — edit this section  <<<
// ============================================================

// -- Uncomment ONE board type --
#define BOARD_ESP32CAM       // AI Thinker ESP32-CAM (has camera + flash LED)
// #define BOARD_ESP32       // Any ESP32 without camera
// #define BOARD_ESP8266     // ESP8266 (NodeMCU, Wemos D1, etc.)

// -- Uncomment if your board has a flash/torch LED --
// (already enabled automatically for BOARD_ESP32CAM)
// #define HAS_FLASH_LED
// #define FLASH_LED_PIN  4   // change to your pin

// -- Uncomment if your board has a status LED --
// (already enabled automatically for BOARD_ESP32CAM)
// #define HAS_STATUS_LED
// #define STATUS_LED_PIN 33  // change to your pin

// -- Uncomment if you want PWM motor speed control --
// (uses LEDC on ESP32, analogWrite on ESP8266)
// On ESP8266 or basic ESP32 with simple HIGH/LOW motors,
// leave this commented — motors will use digitalWrite instead.
// #define USE_PWM_MOTORS

// ============================================================
//  Camera pins  (only needed for BOARD_ESP32CAM)
//  These are set to AI Thinker defaults. Change if different.
// ============================================================
#ifdef BOARD_ESP32CAM
  #ifndef CAM_PIN_PWDN
    #define CAM_PIN_PWDN  32
    #define CAM_PIN_RESET -1
    #define CAM_PIN_XCLK   0
    #define CAM_PIN_SIOD  26
    #define CAM_PIN_SIOC  27
    #define CAM_PIN_Y9    35
    #define CAM_PIN_Y8    34
    #define CAM_PIN_Y7    39
    #define CAM_PIN_Y6    36
    #define CAM_PIN_Y5    21
    #define CAM_PIN_Y4    19
    #define CAM_PIN_Y3    18
    #define CAM_PIN_Y2     5
    #define CAM_PIN_VSYNC 25
    #define CAM_PIN_HREF  23
    #define CAM_PIN_PCLK  22
  #endif
  // ESP32-CAM has flash + status LED built in
  #ifndef HAS_FLASH_LED
    #define HAS_FLASH_LED
    #define FLASH_LED_PIN  4
  #endif
  #ifndef HAS_STATUS_LED
    #define HAS_STATUS_LED
    #define STATUS_LED_PIN 33
  #endif
  // ESP32-CAM supports PWM via LEDC
  #ifndef USE_PWM_MOTORS
    #define USE_PWM_MOTORS
  #endif
#endif

// ============================================================
//  Board-specific includes
// ============================================================
#ifdef BOARD_ESP8266
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
  #include <ESP8266mDNS.h>
#else
  // ESP32 and ESP32-CAM
  #include <WiFi.h>
  #include <AsyncTCP.h>
  #include <ESPmDNS.h>
#endif

#ifdef BOARD_ESP32CAM
  #include "esp_camera.h"
#endif

#include <ESPAsyncWebServer.h>
#include <Arduino.h>

// ============================================================
//  Motor driver defaults
// ============================================================
#ifdef USE_PWM_MOTORS
  #ifdef BOARD_ESP32CAM
    #define MOTOR_LEDC_FREQ  1000
    #define MOTOR_LEDC_RES   8
  #endif
#endif
#define ACCEL_STEP_DEFAULT 15
#define ACCEL_TICK_MS      20

// ============================================================
//  Callback types
// ============================================================
// state = HIGH (pressed) or LOW (auto-released after timeout)
typedef void (*ButtonCallback)(uint8_t state);
typedef void (*EventCallback)();
typedef void (*JoystickCallback)(int x, int y);        // -255…+255
typedef void (*JoystickRawCallback)(float x, float y); // -1.0…+1.0

// ============================================================
//  Enums
// ============================================================
#ifdef BOARD_ESP8266
  enum class ControllerWiFiMode { AP, STA };
#else
  enum class ControllerWiFiMode { AP, STA };
#endif
enum class MotorSide { LEFT, RIGHT };

// ============================================================
//  ESPController class
// ============================================================
class ESPController {
public:
  static ESPController& instance();
  ESPController(const ESPController&)            = delete;
  ESPController& operator=(const ESPController&) = delete;

  // ════════════════════════════════════════════════════════
  //  CONFIGURATION  — call before begin()
  // ════════════════════════════════════════════════════════

  /** WiFi SSID + password.
   *  AP mode  = creates its own hotspot (default).
   *  STA mode = joins existing router.
   */
  void setWiFi(const char* ssid, const char* password,
               ControllerWiFiMode mode = ControllerWiFiMode::AP);

  /** Reach device at http://<hostname>.local */
  void setMDNS(const char* hostname = "espcontroller");

  /** Motor A pins (e.g. left motor IN1, IN2) */
  void setMotorAPins(uint8_t in1, uint8_t in2);

  /** Motor B pins (e.g. right motor IN3, IN4) */
  void setMotorBPins(uint8_t in1, uint8_t in2);

  /** Stop motors if no command received within ms. 0 = disabled. */
  void setWatchdogTimeout(unsigned long ms);

  /** Global speed cap 0-255. */
  void setMaxSpeed(uint8_t speed);

  /** Per-motor drift correction -100…+100 percent. */
  void setMotorTrim(MotorSide side, int8_t trimPercent);

  /** Soft acceleration. step=255 for instant response. */
  void setAcceleration(uint8_t accelStep, uint16_t tickMs = ACCEL_TICK_MS);

  // ════════════════════════════════════════════════════════
  //  CALLBACKS
  //  state = HIGH when button pressed, LOW when released.
  //  LOW is fired automatically after timeout since the
  //  app only sends press, not release.
  // ════════════════════════════════════════════════════════
  void onUp   (ButtonCallback cb);
  void onDown (ButtonCallback cb);
  void onLeft (ButtonCallback cb);
  void onRight(ButtonCallback cb);
  void onStop (ButtonCallback cb);   // always HIGH, no release

  void onJoystick   (JoystickCallback    cb);   // x,y  -255…+255
  void onJoystickRaw(JoystickRawCallback cb);   // x,y  -1.0…+1.0

  void onControllerConnected   (EventCallback cb);
  void onControllerDisconnected(EventCallback cb);

#ifdef BOARD_ESP32CAM
  void onCameraConnected   (EventCallback cb);
  void onCameraDisconnected(EventCallback cb);
#endif

  /** Battery monitor via ADC pin. Fires once below thresholdMv. */
  void onLowBattery(uint8_t adcPin, uint32_t thresholdMv, EventCallback cb);

  // ════════════════════════════════════════════════════════
  //  MOTOR HELPERS
  //  Only available after setMotorAPins / setMotorBPins.
  //  speed: -255 (full reverse) … +255 (full forward)
  // ════════════════════════════════════════════════════════
  void setMotorA(int speed);
  void setMotorB(int speed);
  void stopMotors();
  void emergencyStop();   // instant cut, bypasses ramp, kills flash

  // ════════════════════════════════════════════════════════
  //  CONNECTIVITY
  // ════════════════════════════════════════════════════════
  int     getRSSI();
  uint8_t getRSSIQuality();   // 0-100
  String  getStatusJSON();

  // ════════════════════════════════════════════════════════
  //  LIFECYCLE
  // ════════════════════════════════════════════════════════
  bool begin();    // call at end of setup()
  void update();   // must be called every loop()

private:
  ESPController();

  // WiFi
  const char*        _ssid     = "ESP_CTRL";
  const char*        _pass     = "12345678";
  ControllerWiFiMode _wifiMode = ControllerWiFiMode::AP;
  bool               _mdnsOn   = false;
  const char*        _mdnsHost = "espcontroller";

  // Motors
  uint8_t _aIn1 = 12, _aIn2 = 13;
  uint8_t _bIn1 = 14, _bIn2 = 15;
  bool    _motorsEnabled = false;

  uint8_t  _maxSpeed    = 255;
  int8_t   _trimA = 0,  _trimB = 0;
  uint8_t  _accelStep   = ACCEL_STEP_DEFAULT;
  uint16_t _accelTickMs = ACCEL_TICK_MS;
  int      _targetA = 0, _currentA = 0;
  int      _targetB = 0, _currentB = 0;
  unsigned long _lastAccelTick = 0;

  // Watchdog
  unsigned long _watchdogMs  = 500;
  unsigned long _lastCmdTime = 0;

  // Flash LED
#ifdef HAS_FLASH_LED
  int  _flashBrightness = 0;
  void _setFlash(int brightness);
#endif

  // Status LED
#ifdef HAS_STATUS_LED
  void _blinkStatus(int times, int ms = 200);
#endif

  // Server
  AsyncWebServer* _server   = nullptr;
  AsyncWebSocket* _wsCtrl   = nullptr;
#ifdef BOARD_ESP32CAM
  AsyncWebSocket* _wsCamera = nullptr;
  sensor_t*       _sensor   = nullptr;
  unsigned long   _frameInterval = 66;
  unsigned long   _lastFrameTime = 0;
  bool _initCamera();
  void _sendFrame();
#endif

  // Battery
  uint8_t       _batPin      = 0;
  uint32_t      _batThreshMv = 0;
  bool          _batEnabled  = false, _batAlertFired = false;
  unsigned long _lastBatCheck = 0;
  void _checkBattery();

  // RSSI
  unsigned long _lastRssiTime = 0;

  // Button auto-release (app only sends press, not release)
  static const unsigned long BUTTON_RELEASE_MS = 300;
  bool          _upActive    = false; unsigned long _upLastMs    = 0;
  bool          _downActive  = false; unsigned long _downLastMs  = 0;
  bool          _leftActive  = false; unsigned long _leftLastMs  = 0;
  bool          _rightActive = false; unsigned long _rightLastMs = 0;
  void _checkButtonReleases();

  // Callbacks
  ButtonCallback      _cbUp     = nullptr;
  ButtonCallback      _cbDown   = nullptr;
  ButtonCallback      _cbLeft   = nullptr;
  ButtonCallback      _cbRight  = nullptr;
  ButtonCallback      _cbStop   = nullptr;
  JoystickCallback    _cbJoy    = nullptr;
  JoystickRawCallback _cbJoyRaw = nullptr;
  EventCallback _cbCtrlConn     = nullptr, _cbCtrlDisconn = nullptr;
#ifdef BOARD_ESP32CAM
  EventCallback _cbCamConn      = nullptr, _cbCamDisconn  = nullptr;
#endif
  EventCallback _cbLowBat       = nullptr;

  // Internal
  void _handleCommand(const String& msg);
  void _writeMotor(uint8_t pin1, uint8_t pin2, int speed, int8_t trim);

  static void _onCtrlEvent(AsyncWebSocket*, AsyncWebSocketClient*,
                           AwsEventType, void*, uint8_t*, size_t);
#ifdef BOARD_ESP32CAM
  static void _onCamEvent(AsyncWebSocket*, AsyncWebSocketClient*,
                          AwsEventType, void*, uint8_t*, size_t);
#endif
};

#define Controller ESPController::instance()
