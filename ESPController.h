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
//  Supported boards: ESP32-CAM, ESP32, ESP32-S3, ESP8266
//
//  HOW TO USE:
//    In your .ino file, define your board BEFORE the include:
//
//      #define BOARD_ESP32CAM        // AI-Thinker ESP32-CAM
//      // #define BOARD_ESP32        // Generic ESP32
//      // #define BOARD_ESP32S3      // ESP32-S3
//      // #define BOARD_ESP8266      // ESP8266 / NodeMCU
//      #include <ESPController.h>
//
//    Optional extras (also in .ino, before include):
//      #define USE_PWM_MOTORS        // LEDC/analogWrite speed control
//      #define HAS_FLASH_LED
//      #define FLASH_LED_PIN  4
//      #define HAS_STATUS_LED
//      #define STATUS_LED_PIN 33
//      #define HAS_RGB_STATUS_LED    // addressable NeoPixel status LED (e.g. ESP32-S3 pin 48)
//      #define RGB_STATUS_LED_PIN 48
//
//    For ESP32-CAM you can override camera pins too:
//      #define CAM_PIN_PWDN  32
//      ... etc.
//
//    REQUIRED LIBRARIES (install via Library Manager before compiling):
//      - ESPAsyncWebServer
//      - AsyncTCP        (ESP32 family)  /  ESPAsyncTCP (ESP8266)
//      - Adafruit NeoPixel   <-- only needed if HAS_RGB_STATUS_LED is active
//                                (auto-enabled on BOARD_ESP32S3)
//      If you downloaded this library as a GitHub ZIP, these are NOT
//      bundled and must be installed separately, the same way you'd
//      install this library itself.
//
//  NEVER edit this file. All config belongs in your .ino.
// ============================================================
#pragma once

// ============================================================
//  Pull in Arduino's own headers FIRST, before any board
//  detection runs below.
//
//  WHY THIS MATTERS: this header is processed independently by
//  EVERY .cpp file that includes it — your sketch's .ino *and*
//  this library's own ESPController.cpp. Arduino's build system
//  automatically prepends "#include <Arduino.h>" to your .ino
//  before anything else, so by the time your .ino reaches this
//  header, chip-target macros like CONFIG_IDF_TARGET_ESP32S3 are
//  already visible. ESPController.cpp gets NO such automatic
//  prepend, though — so without this line, the board auto-
//  detection below could silently disagree between your .ino and
//  this library's .cpp, declaring functions in one file and never
//  defining them in the other (classic "undefined reference"
//  linker error). Including Arduino.h here guarantees both files
//  see an identical, correct board determination.
// ============================================================
#include <Arduino.h>

// ============================================================
//  GUARD: exactly one board must be defined
// ============================================================
#if defined(ARDUINO_ARCH_ESP8266)
  #define BOARD_ESP8266

#elif defined(CONFIG_IDF_TARGET_ESP32S3) || defined(ARDUINO_ESP32S3_DEV)
  // CONFIG_IDF_TARGET_ESP32S3 comes from sdkconfig.h (pulled in above
  // via Arduino.h). ARDUINO_ESP32S3_DEV is a belt-and-suspenders
  // backup: it's injected directly as a compiler -D flag for this
  // FQBN, so it stays visible even if a toolchain update ever changes
  // how/when sdkconfig.h gets included.
  #define BOARD_ESP32S3

#elif defined(BOARD_HAS_PSRAM)
  #define BOARD_ESP32CAM

#else
  #define BOARD_ESP32
#endif
// Only one board at a time
#if (defined(BOARD_ESP32CAM) && (defined(BOARD_ESP32)||defined(BOARD_ESP32S3)||defined(BOARD_ESP8266))) || \
    (defined(BOARD_ESP32)    && (defined(BOARD_ESP32S3)||defined(BOARD_ESP8266))) || \
    (defined(BOARD_ESP32S3)  && defined(BOARD_ESP8266))
  #error "ESPController: Multiple boards defined. Define exactly ONE board."
#endif

// ============================================================
//  BOARD CAPABILITY MAP
//
//  Feature              CAM   ESP32  S3    8266
//  ─────────────────    ───   ─────  ──    ────
//  Camera               YES   NO     NO    NO
//  LEDC PWM             YES   YES    YES   NO
//  analogWrite PWM      NO    NO     NO    YES
//  Flash LED (auto)     YES   NO     NO    NO
//  Status LED (auto)    YES   NO     NO    NO
//  mDNS                 YES   YES    YES   YES *
//  WiFi AP+STA          YES   YES    YES   YES
//  Battery ADC          YES   YES    YES   YES
//
//  * ESP8266 mDNS needs MDNS.update() in loop — handled internally
// ============================================================

// ============================================================
//  ESP32-CAM: auto-enable camera features and built-in pins
// ============================================================
#ifdef BOARD_ESP32CAM
  // Default AI-Thinker camera pin mapping — override in .ino if needed
  #ifndef CAM_PIN_PWDN
    #define CAM_PIN_PWDN   32
    #define CAM_PIN_RESET  -1
    #define CAM_PIN_XCLK    0
    #define CAM_PIN_SIOD   26
    #define CAM_PIN_SIOC   27
    #define CAM_PIN_Y9     35
    #define CAM_PIN_Y8     34
    #define CAM_PIN_Y7     39
    #define CAM_PIN_Y6     36
    #define CAM_PIN_Y5     21
    #define CAM_PIN_Y4     19
    #define CAM_PIN_Y3     18
    #define CAM_PIN_Y2      5
    #define CAM_PIN_VSYNC  25
    #define CAM_PIN_HREF   23
    #define CAM_PIN_PCLK   22
  #endif

  // Built-in flash LED on pin 4
  #ifndef HAS_FLASH_LED
    #define HAS_FLASH_LED
    #define FLASH_LED_PIN  4
  #endif

  // Built-in red status LED on pin 33 (active LOW on most modules)
  #ifndef HAS_STATUS_LED
    #define HAS_STATUS_LED
    #define STATUS_LED_PIN 33
  #endif

  // ESP32-CAM uses LEDC for PWM
  #ifndef USE_PWM_MOTORS
    #define USE_PWM_MOTORS
  #endif
#endif // BOARD_ESP32CAM

// ============================================================
//  ESP32 / ESP32-S3: LEDC available for PWM if requested
// ============================================================
#if defined(BOARD_ESP32) || defined(BOARD_ESP32S3)
  // USE_PWM_MOTORS may be defined by user in .ino — we just honour it
  // LEDC constants are set below in Motor driver section
#endif

// ============================================================
//  ESP32-S3: built-in addressable RGB status LED on GPIO48
// ============================================================
#ifdef BOARD_ESP32S3
  #define HAS_RGB_STATUS_LED
  #ifndef RGB_STATUS_LED_PIN
    #define RGB_STATUS_LED_PIN 48
  #endif
#endif

// ============================================================
//  ESP8266: no LEDC — analogWrite only (10-bit 0-1023 range)
// ============================================================
#ifdef BOARD_ESP8266
  // USE_PWM_MOTORS may be defined by user — will use analogWrite
  // Reminder: LEDC calls must not appear in this path
#endif

// ============================================================
//  Motor driver constants
// ============================================================
#ifdef USE_PWM_MOTORS
  #if defined(BOARD_ESP32CAM) || defined(BOARD_ESP32) || defined(BOARD_ESP32S3)
    #define MOTOR_LEDC_FREQ   1000
    #define MOTOR_LEDC_RES       8    // 8-bit → 0-255
  #endif
  // ESP8266: analogWrite uses 0-1023; the library maps 0-255 → 0-1023
#endif

#define ACCEL_STEP_DEFAULT  15
#define ACCEL_TICK_MS       20

// ============================================================
//  Board-specific includes — no user action needed
// ============================================================
#ifdef BOARD_ESP8266
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
  #include <ESP8266mDNS.h>
#else
  // ESP32 / ESP32-S3 / ESP32-CAM
  #include <WiFi.h>
  #include <AsyncTCP.h>
  #include <ESPmDNS.h>
#endif

#ifdef BOARD_ESP32CAM
  #include "esp_camera.h"
#endif

#ifdef HAS_RGB_STATUS_LED
  #include <Adafruit_NeoPixel.h>
#endif

#include <ESPAsyncWebServer.h>

// ============================================================
//  Callback types
// ============================================================
typedef void (*ButtonCallback)(uint8_t state);   // HIGH = pressed, LOW = released
typedef void (*EventCallback)();
typedef void (*JoystickCallback)(int x, int y);        // -255 … +255
typedef void (*JoystickRawCallback)(float x, float y); // -1.0 … +1.0

// ============================================================
//  Enums
// ============================================================
enum class ControllerWiFiMode { AP, STA };
enum class MotorSide { LEFT, RIGHT };

// ============================================================
//  ESPController — singleton
// ============================================================
class ESPController {
public:
  static ESPController& instance();
  ESPController(const ESPController&)            = delete;
  ESPController& operator=(const ESPController&) = delete;

  // ══════════════════════════════════════════════════════════
  //  CONFIGURATION  — call before begin()
  // ══════════════════════════════════════════════════════════

  /** WiFi credentials + mode.
   *  AP  = creates its own hotspot (default — works everywhere, no router needed).
   *  STA = joins an existing router (falls back to AP on failure).
   */
  void setWiFi(const char* ssid, const char* password,
               ControllerWiFiMode mode = ControllerWiFiMode::AP);

  /** Reach the board at http://<hostname>.local on the same network. */
  void setMDNS(const char* hostname = "espcontroller");

  /** Motor A (left) driver pins — typically IN1, IN2 of an L298N / L293D. */
  void setMotorAPins(uint8_t in1, uint8_t in2);

  /** Motor B (right) driver pins — typically IN3, IN4. */
  void setMotorBPins(uint8_t in1, uint8_t in2);

  /** Stop motors if no command received within ms. 0 = disabled. */
  void setWatchdogTimeout(unsigned long ms);

  /** Hard speed cap 0-255, applied after trim. */
  void setMaxSpeed(uint8_t speed);

  /** Per-motor drift correction -100 … +100 percent.
   *  Positive trims the motor faster; negative slower. */
  void setMotorTrim(MotorSide side, int8_t trimPercent);

  /** Soft-start ramp. step=255 → instant; lower values = gentler ramp. */
  void setAcceleration(uint8_t accelStep, uint16_t tickMs = ACCEL_TICK_MS);

  // ══════════════════════════════════════════════════════════
  //  CALLBACKS — all optional
  // ══════════════════════════════════════════════════════════

  /** D-pad / button events. Callback receives HIGH (press) or LOW (release). */
  void onUp   (ButtonCallback cb);
  void onDown (ButtonCallback cb);
  void onLeft (ButtonCallback cb);
  void onRight(ButtonCallback cb);
  void onStop (ButtonCallback cb);

  /** Joystick events. Both fire on every joy: message. */
  void onJoystick   (JoystickCallback    cb);  // x/y in -255 … +255
  void onJoystickRaw(JoystickRawCallback cb);  // x/y in -1.0 … +1.0

  /** App connection state. */
  void onControllerConnected   (EventCallback cb);
  void onControllerDisconnected(EventCallback cb);

  // ── Camera callbacks (ESP32-CAM only) ──────────────────
#ifdef BOARD_ESP32CAM
  /** Fires when the Flutter app opens / closes the /camera WebSocket. */
  void onCameraConnected   (EventCallback cb);
  void onCameraDisconnected(EventCallback cb);
#endif

  /** Battery monitor via ADC pin. Fires once per boot when voltage drops
   *  below thresholdMv. adcPin must be an ADC-capable pin. */
  void onLowBattery(uint8_t adcPin, uint32_t thresholdMv, EventCallback cb);

  // ══════════════════════════════════════════════════════════
  //  MOTOR CONTROL HELPERS
  // ══════════════════════════════════════════════════════════

  /** Set Motor A speed  -255 (full reverse) … +255 (full forward). */
  void setMotorA(int speed);

  /** Set Motor B speed  -255 … +255. */
  void setMotorB(int speed);

  /** Ramp both motors to zero (respects acceleration). */
  void stopMotors();

  /** Instantly kill motors + flash LED (no ramp). Called on disconnect. */
  void emergencyStop();

  // ══════════════════════════════════════════════════════════
  //  STATUS / TELEMETRY
  // ══════════════════════════════════════════════════════════

  /** Raw RSSI dBm (STA mode useful; returns 0 in AP mode). */
  int     getRSSI();

  /** RSSI as 0-100 quality percentage. */
  uint8_t getRSSIQuality();

  /** JSON status blob sent to /status HTTP endpoint.
   *  Includes ip, rssi, rssi_quality, ctrl_clients, max_speed.
   *  ESP32-CAM also includes cam_clients, fps.
   *  Boards with HAS_FLASH_LED include flash brightness.
   */
  String  getStatusJSON();

  // ══════════════════════════════════════════════════════════
  //  LIFECYCLE — call in setup() and loop()
  // ══════════════════════════════════════════════════════════

  /** Initialises WiFi, mDNS, motors, camera (ESP32-CAM), WebSocket, HTTP.
   *  Returns false only if ESP32-CAM camera init fails (board halts). */
  bool begin();

  /** Must be called in every loop() iteration. Handles:
   *  motor ramp, watchdog, camera frames, battery check, RSSI broadcast,
   *  ESP8266 mDNS poll, button auto-release. */
  void update();

// ════════════════════════════════════════════════════════════
private:
// ════════════════════════════════════════════════════════════
  ESPController();

  // ── WiFi ────────────────────────────────────────────────
  const char*        _ssid     = "ESP_CTRL";
  const char*        _pass     = "12345678";
  ControllerWiFiMode _wifiMode = ControllerWiFiMode::AP;
  bool               _mdnsOn   = false;
  const char*        _mdnsHost = "espcontroller";

  // ── Motors ──────────────────────────────────────────────
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

  // ── Watchdog ────────────────────────────────────────────
  unsigned long _watchdogMs  = 500;
  unsigned long _lastCmdTime = 0;

  // ── Flash LED ───────────────────────────────────────────
  // Available on: BOARD_ESP32CAM (always), any board with HAS_FLASH_LED defined
#ifdef HAS_FLASH_LED
  int  _flashBrightness = 0;
  void _setFlash(int brightness);
#endif

  // ── Status LED ──────────────────────────────────────────
  // Available on: BOARD_ESP32CAM (always), any board with HAS_STATUS_LED defined
#ifdef HAS_STATUS_LED
  void _blinkStatus(int times, int ms = 200);
#endif

  // ── RGB Status LED (e.g. ESP32-S3 onboard NeoPixel, GPIO48) ──
#ifdef HAS_RGB_STATUS_LED
  Adafruit_NeoPixel _rgbStatus = Adafruit_NeoPixel(1, RGB_STATUS_LED_PIN, NEO_GRB + NEO_KHZ800);
  bool _rgbInited = false;   // tracks whether _rgbStatus.begin() has run yet,
                             // so setRGB() works even if called before begin()
#endif

public:
#ifdef HAS_RGB_STATUS_LED
  /** Set the onboard RGB status LED to an explicit colour.
   *  r/g/b are 0-255 each. Call from your .ino at any time after begin(). */
  void setRGB(uint8_t r, uint8_t g, uint8_t b);

  /** Convenience: turn the RGB LED off. */
  void setRGBOff();
#endif

private:

  // ── Web server ──────────────────────────────────────────
  AsyncWebServer* _server  = nullptr;
  AsyncWebSocket* _wsCtrl  = nullptr;

  // ── Camera (ESP32-CAM only) ─────────────────────────────
#ifdef BOARD_ESP32CAM
  AsyncWebSocket* _wsCamera      = nullptr;
  sensor_t*       _sensor        = nullptr;
  unsigned long   _frameInterval = 66;   // ms between frames (~15 fps default)
  unsigned long   _lastFrameTime = 0;
  bool _initCamera();
  void _sendFrame();
#endif

  // ── Battery ─────────────────────────────────────────────
  uint8_t       _batPin       = 0;
  uint32_t      _batThreshMv  = 0;
  bool          _batEnabled   = false;
  bool          _batAlertFired= false;
  unsigned long _lastBatCheck = 0;
  void _checkBattery();

  // ── RSSI broadcast ──────────────────────────────────────
  unsigned long _lastRssiTime = 0;

  // ── Button auto-release ─────────────────────────────────
  // If no new press arrives within BUTTON_RELEASE_MS, the library fires
  // the release callback automatically (mimics a physical button release).
  static const unsigned long BUTTON_RELEASE_MS = 300;
  bool          _upActive    = false; unsigned long _upLastMs    = 0;
  bool          _downActive  = false; unsigned long _downLastMs  = 0;
  bool          _leftActive  = false; unsigned long _leftLastMs  = 0;
  bool          _rightActive = false; unsigned long _rightLastMs = 0;
  void _checkButtonReleases();

  // ── Callbacks ───────────────────────────────────────────
  ButtonCallback      _cbUp      = nullptr;
  ButtonCallback      _cbDown    = nullptr;
  ButtonCallback      _cbLeft    = nullptr;
  ButtonCallback      _cbRight   = nullptr;
  ButtonCallback      _cbStop    = nullptr;
  JoystickCallback    _cbJoy     = nullptr;
  JoystickRawCallback _cbJoyRaw  = nullptr;
  EventCallback _cbCtrlConn      = nullptr;
  EventCallback _cbCtrlDisconn   = nullptr;
#ifdef BOARD_ESP32CAM
  EventCallback _cbCamConn       = nullptr;
  EventCallback _cbCamDisconn    = nullptr;
#endif
  EventCallback _cbLowBat        = nullptr;

  // ── Internal helpers ────────────────────────────────────
  void _handleCommand(const String& msg);

  /** Write speed to one motor. Handles PWM vs digitalWrite and trim. */
  void _writeMotor(uint8_t pin1, uint8_t pin2, int speed, int8_t trim);

  /** WebSocket event handler — /control endpoint. */
  static void _onCtrlEvent(AsyncWebSocket*, AsyncWebSocketClient*,
                            AwsEventType, void*, uint8_t*, size_t);

#ifdef BOARD_ESP32CAM
  /** WebSocket event handler — /camera endpoint (ESP32-CAM only). */
  static void _onCamEvent(AsyncWebSocket*, AsyncWebSocketClient*,
                           AwsEventType, void*, uint8_t*, size_t);
#endif
};

// Convenience alias — use Controller.begin() instead of ESPController::instance().begin()
#define Controller ESPController::instance()
