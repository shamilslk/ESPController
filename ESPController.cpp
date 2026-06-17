/*
 * ============================================================
 *  ESPController.cpp
 *  Part of ESPController Library by Shamil K, Vismaya P
 *  https://github.com/shamilslk/ESPController
 *  Version : 2.0.0
 * ============================================================
 */

#include "ESPController.h"

// ──────────────────────────────────────────────
//  Embedded HTML controller page (stored in flash)
// ──────────────────────────────────────────────
static const char CONTROLLER_HTML[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
<title>ESPController</title>
<style>
  :root{--bg:#0d0d0d;--surface:#1a1a2e;--accent:#e94560;--text:#eaeaea;--muted:#888;}
  *{box-sizing:border-box;margin:0;padding:0;touch-action:none;}
  body{background:var(--bg);color:var(--text);font-family:'Segoe UI',sans-serif;
       display:flex;flex-direction:column;height:100vh;overflow:hidden;}
  #status-bar{background:var(--surface);padding:6px 14px;display:flex;
              align-items:center;gap:10px;font-size:13px;border-bottom:1px solid #222;}
  #dot{width:10px;height:10px;border-radius:50%;background:#e94560;}
  #dot.connected{background:#00e676;}
  #dot.reconnecting{background:#ffab00;}
  #main{display:flex;flex:1;overflow:hidden;}
  #cam-panel{flex:1;background:#000;display:flex;align-items:center;justify-content:center;position:relative;}
  #cam-panel img{max-width:100%;max-height:100%;object-fit:contain;}
  #no-cam{color:var(--muted);font-size:14px;text-align:center;}
  #ctrl-panel{width:200px;background:var(--surface);display:flex;flex-direction:column;
              align-items:center;justify-content:space-evenly;padding:10px;gap:8px;
              border-left:1px solid #222;}
  .dpad{display:grid;grid-template-columns:repeat(3,52px);grid-template-rows:repeat(3,52px);gap:4px;}
  .dpad-btn{background:#252540;border:none;border-radius:8px;color:var(--text);
             font-size:20px;cursor:pointer;display:flex;align-items:center;justify-content:center;}
  .dpad-btn:active,.dpad-btn.pressed{background:var(--accent);}
  .dpad-empty{visibility:hidden;}
  #stop-btn{width:100%;padding:10px;background:#c0392b;border:none;border-radius:10px;
             color:#fff;font-size:16px;font-weight:700;cursor:pointer;letter-spacing:1px;}
  #stop-btn:active{background:#e74c3c;}
  #joystick-zone{width:130px;height:130px;background:#111;border-radius:50%;
                 border:2px solid #333;position:relative;touch-action:none;}
  #joystick-knob{width:50px;height:50px;background:var(--accent);border-radius:50%;
                 position:absolute;top:50%;left:50%;transform:translate(-50%,-50%);
                 transition:box-shadow .1s;}
  #speed-wrap{width:100%;font-size:12px;color:var(--muted);}
  #speed-wrap input{width:100%;accent-color:var(--accent);}
</style>
</head>
<body>
<div id="status-bar">
  <div id="dot"></div>
  <span id="status-text">Connecting…</span>
  <span style="margin-left:auto;color:var(--muted)" id="ip-label"></span>
</div>
<div id="main">
  <div id="cam-panel">
    <div id="no-cam">📷 Camera feed will appear here</div>
  </div>
  <div id="ctrl-panel">
    <div class="dpad">
      <div class="dpad-empty"></div>
      <button class="dpad-btn" data-cmd="F">▲</button>
      <div class="dpad-empty"></div>
      <button class="dpad-btn" data-cmd="L">◀</button>
      <button class="dpad-btn" data-cmd="S" style="font-size:14px">■</button>
      <button class="dpad-btn" data-cmd="R">▶</button>
      <div class="dpad-empty"></div>
      <button class="dpad-btn" data-cmd="B">▼</button>
      <div class="dpad-empty"></div>
    </div>
    <div id="joystick-zone"><div id="joystick-knob"></div></div>
    <button id="stop-btn">■ STOP</button>
    <div id="speed-wrap">
      <label>Speed: <span id="speed-val">200</span></label>
      <input type="range" id="speed-slider" min="50" max="255" value="200">
    </div>
  </div>
</div>
<script>
const dot=document.getElementById('dot');
const statusText=document.getElementById('status-text');
document.getElementById('ip-label').textContent=location.hostname;
let ws,reconnectTimer;
function connect(){
  ws=new WebSocket('ws://'+location.hostname+':81/');
  ws.onopen=()=>{dot.className='connected';statusText.textContent='Connected';};
  ws.onclose=()=>{dot.className='reconnecting';statusText.textContent='Reconnecting…';
                  reconnectTimer=setTimeout(connect,2000);};
  ws.onerror=()=>ws.close();
}
connect();
function send(msg){if(ws&&ws.readyState===1)ws.send(msg);}
document.querySelectorAll('.dpad-btn').forEach(btn=>{
  btn.addEventListener('pointerdown',e=>{e.preventDefault();btn.classList.add('pressed');send(btn.dataset.cmd);});
  btn.addEventListener('pointerup',  ()=>{btn.classList.remove('pressed');if(btn.dataset.cmd!=='S')send('S');});
  btn.addEventListener('pointerleave',()=>{btn.classList.remove('pressed');if(btn.dataset.cmd!=='S')send('S');});
});
document.getElementById('stop-btn').addEventListener('click',()=>send('E'));
const speedSlider=document.getElementById('speed-slider');
const speedVal=document.getElementById('speed-val');
speedSlider.addEventListener('input',()=>{speedVal.textContent=speedSlider.value;send('V'+speedSlider.value);});
const zone=document.getElementById('joystick-zone');
const knob=document.getElementById('joystick-knob');
const R=55;let jActive=false,jOrigin={x:0,y:0};
zone.addEventListener('pointerdown',e=>{
  jActive=true;zone.setPointerCapture(e.pointerId);
  const r=zone.getBoundingClientRect();
  jOrigin={x:r.left+r.width/2,y:r.top+r.height/2};
  moveKnob(e);
});
zone.addEventListener('pointermove',e=>{if(jActive)moveKnob(e);});
zone.addEventListener('pointerup',resetKnob);
zone.addEventListener('pointercancel',resetKnob);
function moveKnob(e){
  let dx=e.clientX-jOrigin.x,dy=e.clientY-jOrigin.y;
  const dist=Math.min(Math.hypot(dx,dy),R);
  const angle=Math.atan2(dy,dx);
  dx=Math.cos(angle)*dist;dy=Math.sin(angle)*dist;
  knob.style.left=(50+dx/R*45)+'%';
  knob.style.top=(50+dy/R*45)+'%';
  const jx=Math.round(dx/R*255),jy=-Math.round(dy/R*255);
  send('J'+jx+','+jy);
}
function resetKnob(){jActive=false;knob.style.left='50%';knob.style.top='50%';send('S');}
</script>
</body>
</html>
)rawhtml";

// ──────────────────────────────────────────────
//  Singleton instance
// ──────────────────────────────────────────────
ESPControllerClass Controller;

// ──────────────────────────────────────────────
//  Constructor
// ──────────────────────────────────────────────
ESPControllerClass::ESPControllerClass()
  : _ssid(nullptr), _password(nullptr),
    _wifiMode(ControllerWiFiMode::AP),
    _mdnsName(nullptr),
    _httpPort(80), _wsPort(81),
    _server(nullptr), _ws(nullptr),
    _maxSpeed(200),
    _targetA(0), _targetB(0),
    _currentA(0), _currentB(0),
    _accelStep(255), _accelTickMs(20),
    _lastAccelTick(0),
    _watchdogMs(0), _lastCmdTime(0),
    _lastRssiBroadcast(0),
    _connectedClients(0),
    _battPin(0), _battThresholdMv(0), _battTriggered(false),
    _cbUp(nullptr), _cbDown(nullptr),
    _cbLeft(nullptr), _cbRight(nullptr), _cbStop(nullptr),
    _cbJoystick(nullptr), _cbJoystickRaw(nullptr),
    _cbConnected(nullptr), _cbDisconnected(nullptr),
    _cbLowBattery(nullptr)
{
  _motorA = {0, 0, 0, false};
  _motorB = {0, 0, 0, false};
  for (int i = 0; i < 4; i++) { _btnPressTime[i] = 0; _btnActive[i] = false; }
}

// ──────────────────────────────────────────────
//  Configuration
// ──────────────────────────────────────────────
void ESPControllerClass::setWiFi(const char* ssid, const char* password,
                                   ControllerWiFiMode mode) {
  _ssid     = ssid;
  _password = password;
  _wifiMode = mode;
}

void ESPControllerClass::setMDNS(const char* hostname) {
  _mdnsName = hostname;
}

void ESPControllerClass::setMotorAPins(uint8_t in1, uint8_t in2) {
  _motorA.in1        = in1;
  _motorA.in2        = in2;
  _motorA.configured = true;
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
}

void ESPControllerClass::setMotorBPins(uint8_t in1, uint8_t in2) {
  _motorB.in1        = in1;
  _motorB.in2        = in2;
  _motorB.configured = true;
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
}

void ESPControllerClass::setMaxSpeed(uint8_t speed) {
  _maxSpeed = speed;
}

void ESPControllerClass::setMotorTrim(MotorSide side, int8_t trim) {
  if (side == MotorSide::LEFT)  _motorA.trim = _clamp(trim, -100, 100);
  else                          _motorB.trim = _clamp(trim, -100, 100);
}

void ESPControllerClass::setWatchdogTimeout(uint32_t ms) {
  _watchdogMs = ms;
}

void ESPControllerClass::setAcceleration(uint8_t step, uint16_t tickMs) {
  _accelStep   = step;
  _accelTickMs = tickMs;
}

// ──────────────────────────────────────────────
//  begin() — starts everything
// ──────────────────────────────────────────────
void ESPControllerClass::begin(uint16_t httpPort, uint16_t wsPort) {
  _httpPort = httpPort;
  _wsPort   = wsPort;

  Serial.println(F("[CTRL] Starting ESPController..."));

  // ── WiFi ────────────────────────────────────
  if (_wifiMode == ControllerWiFiMode::AP) {
    if (_password && strlen(_password) >= 8) {
      WiFi.softAP(_ssid, _password);
    } else {
      if (_password && strlen(_password) > 0) {
        Serial.println(F("[WIFI] Warning: password < 8 chars, starting open AP"));
      }
      WiFi.softAP(_ssid);
    }
    Serial.print(F("[WIFI] AP IP: "));
    Serial.println(WiFi.softAPIP());
  } else {
    WiFi.begin(_ssid, _password);
    Serial.print(F("[WIFI] Connecting to "));
    Serial.print(_ssid);
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED) {
      if (millis() - start > 15000) {
        Serial.println(F("\n[WIFI] Connection timed out"));
        break;
      }
      delay(250);
      Serial.print('.');
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print(F("\n[WIFI] STA IP: "));
      Serial.println(WiFi.localIP());
    }
  }

  // ── mDNS ────────────────────────────────────
  if (_mdnsName) {
    if (MDNS.begin(_mdnsName)) {
      Serial.print(F("[mDNS] Reachable at http://"));
      Serial.print(_mdnsName);
      Serial.println(F(".local"));
    }
  }

  // ── HTTP server ─────────────────────────────
#if defined(ESP8266)
  _server = new ESP8266WebServer(_httpPort);
#else
  _server = new WebServer(_httpPort);
#endif
  _setupRoutes();
  _server->begin();
  Serial.printf("[HTTP] Server ready on port %u\n", _httpPort);

  // ── WebSocket server ─────────────────────────
  _ws = new WebSocketsServer(_wsPort);
  _ws->begin();
  _ws->onEvent([this](uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    _onWsEvent(num, type, payload, length);
  });
  Serial.printf("[WS] Server ready on port %u\n", _wsPort);

  // ── Motor PWM ────────────────────────────────
  if (_motorA.configured || _motorB.configured) {
    Serial.println(F("[MOTORS] PWM ready"));
  }

  _lastCmdTime = millis();
  Serial.println(F("[CTRL] Ready"));
}

// ──────────────────────────────────────────────
//  update() — call every loop()
// ──────────────────────────────────────────────
void ESPControllerClass::update() {
  if (_server) _server->handleClient();
  if (_ws)     _ws->loop();

#if defined(ESP8266)
  if (_mdnsName) MDNS.update();
#endif

  _applyRamp();
  _checkWatchdog();
  _checkBtnRelease();
  _checkBattery();
  _broadcastRSSI();
}

// ──────────────────────────────────────────────
//  HTTP routes
// ──────────────────────────────────────────────
void ESPControllerClass::_setupRoutes() {
  _server->on("/",        HTTP_GET, [this]() { _handleRoot(); });
  _server->on("/cmd",     HTTP_GET, [this]() { _handleHTTPCmd(); });
  _server->on("/status",  HTTP_GET, [this]() { _handleStatus(); });
  // Convenience REST endpoints
  _server->on("/up",      HTTP_GET, [this]() { _processCommand("F"); _server->send(200, "text/plain", "OK"); });
  _server->on("/down",    HTTP_GET, [this]() { _processCommand("B"); _server->send(200, "text/plain", "OK"); });
  _server->on("/left",    HTTP_GET, [this]() { _processCommand("L"); _server->send(200, "text/plain", "OK"); });
  _server->on("/right",   HTTP_GET, [this]() { _processCommand("R"); _server->send(200, "text/plain", "OK"); });
  _server->on("/stop",    HTTP_GET, [this]() { _processCommand("S"); _server->send(200, "text/plain", "OK"); });
  _server->onNotFound([this]() { _server->send(404, "text/plain", "Not found"); });
}

void ESPControllerClass::_handleRoot() {
  _server->send_P(200, "text/html", CONTROLLER_HTML);
}

void ESPControllerClass::_handleHTTPCmd() {
  if (!_server->hasArg("c")) {
    _server->send(400, "text/plain", "Missing ?c=");
    return;
  }
  String raw = _server->arg("c");
  if (raw.length() == 0) {
    _server->send(400, "text/plain", "Empty command");
    return;
  }
  _processCommand(raw);
  _server->send(200, "text/plain", "OK");
}

void ESPControllerClass::_handleStatus() {
  _server->send(200, "application/json", getStatusJSON());
}

// ──────────────────────────────────────────────
//  WebSocket event handler
// ──────────────────────────────────────────────
void ESPControllerClass::_onWsEvent(uint8_t num, WStype_t type,
                                     uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      _connectedClients++;
      Serial.printf("[CTRL] App connected! (client #%u)\n", num);
      if (_cbConnected) _cbConnected();
      break;

    case WStype_DISCONNECTED:
      if (_connectedClients > 0) _connectedClients--;
      Serial.printf("[CTRL] App disconnected (client #%u)\n", num);
      emergencyStop();                           // safety: cut motors immediately
      if (_cbDisconnected) _cbDisconnected();
      break;

    case WStype_TEXT:
      if (length > 0) {
        // Use String constructor with explicit length — no buffer overflow
        String raw = String((char*)payload, length);
        _processCommand(raw);
      }
      break;

    default:
      break;
  }
}

// ──────────────────────────────────────────────
//  Command dispatcher
// ──────────────────────────────────────────────
void ESPControllerClass::_processCommand(const String& raw) {
  if (raw.length() == 0) return;

  _lastCmdTime = millis();   // feed the watchdog

  char    cmd     = raw[0];
  String  payload = raw.length() > 1 ? raw.substring(1) : String("");

  Serial.printf("[CMD] %c  payload='%s'\n", cmd, payload.c_str());

  switch (cmd) {
    // ── D-Pad ──────────────────────────────────
    case 'F':
      _fireDPad(0, _cbUp);
      break;

    case 'B':
      _fireDPad(1, _cbDown);
      break;

    case 'L':
      _fireDPad(2, _cbLeft);
      break;

    case 'R':
      _fireDPad(3, _cbRight);
      break;

    case 'S':
      // Release all active buttons
      for (int i = 0; i < 4; i++) _btnActive[i] = false;
      if (_cbStop) _cbStop(HIGH);
      break;

    // ── Emergency stop ─────────────────────────
    case 'E':
      emergencyStop();
      break;

    // ── Joystick ───────────────────────────────
    case 'J': {
      int comma = payload.indexOf(',');
      if (comma > 0) {
        int x = payload.substring(0, comma).toInt();
        int y = payload.substring(comma + 1).toInt();
        x = _clamp(x, -255, 255);
        y = _clamp(y, -255, 255);
        if (_cbJoystick)    _cbJoystick(x, y);
        if (_cbJoystickRaw) _cbJoystickRaw(x / 255.0f, y / 255.0f);
      }
      break;
    }

    // ── Speed ──────────────────────────────────
    case 'V':
      _maxSpeed = (uint8_t)_clamp(payload.toInt(), 0, 255);
      break;

    default:
      break;
  }
}

// ──────────────────────────────────────────────
//  D-Pad button press with auto-release tracking
// ──────────────────────────────────────────────
void ESPControllerClass::_fireDPad(uint8_t index, void (*cb)(uint8_t)) {
  _btnActive[index]    = true;
  _btnPressTime[index] = millis();
  if (cb) cb(HIGH);
}

void ESPControllerClass::_checkBtnRelease() {
  const uint32_t AUTO_RELEASE_MS = 300;
  uint32_t now = millis();
  void (*cbs[4])(uint8_t) = { _cbUp, _cbDown, _cbLeft, _cbRight };
  for (int i = 0; i < 4; i++) {
    if (_btnActive[i] && (now - _btnPressTime[i] >= AUTO_RELEASE_MS)) {
      _btnActive[i] = false;
      if (cbs[i]) cbs[i](LOW);
    }
  }
}

// ──────────────────────────────────────────────
//  Direct motor control (public API)
// ──────────────────────────────────────────────
void ESPControllerClass::setMotorA(int speed) {
  _targetA = _clamp(speed, -255, 255);
}

void ESPControllerClass::setMotorB(int speed) {
  _targetB = _clamp(speed, -255, 255);
}

void ESPControllerClass::stopMotors() {
  _targetA = 0;
  _targetB = 0;
}

void ESPControllerClass::emergencyStop() {
  _targetA  = 0; _targetB  = 0;
  _currentA = 0; _currentB = 0;
  _driveMotorHW(_motorA, 0, 0);
  _driveMotorHW(_motorB, 0, 0);
}

// ──────────────────────────────────────────────
//  Acceleration ramp
// ──────────────────────────────────────────────
void ESPControllerClass::_applyRamp() {
  uint32_t now = millis();
  if (now - _lastAccelTick < _accelTickMs) return;
  _lastAccelTick = now;

  auto step = [this](int current, int target) -> int {
    if (_accelStep >= 255) return target;
    int diff = target - current;
    if (abs(diff) <= _accelStep) return target;
    return current + (_accelStep * (diff > 0 ? 1 : -1));
  };

  _currentA = step(_currentA, _targetA);
  _currentB = step(_currentB, _targetB);

  // Apply maxSpeed cap and trim, then drive hardware
  int8_t  dirA = (_currentA > 0) ? 1 : (_currentA < 0 ? -1 : 0);
  int8_t  dirB = (_currentB > 0) ? 1 : (_currentB < 0 ? -1 : 0);
  uint8_t pwmA = _scalePwm(abs(_currentA), _motorA.trim);
  uint8_t pwmB = _scalePwm(abs(_currentB), _motorB.trim);

  _driveMotorHW(_motorA, dirA, pwmA);
  _driveMotorHW(_motorB, dirB, pwmB);
}

// Scale raw speed (0–255) by maxSpeed and trim
uint8_t ESPControllerClass::_scalePwm(int speed, int8_t trim) {
  float scaled = (float)speed * _maxSpeed / 255.0f;
  scaled *= (1.0f + trim / 100.0f);
  return (uint8_t)_clamp((int)scaled, 0, 255);
}

// ──────────────────────────────────────────────
//  Low-level motor hardware driver
//  direction: +1=forward, -1=backward, 0=coast
// ──────────────────────────────────────────────
void ESPControllerClass::_driveMotorHW(const _MotorChannel& m, int8_t direction, uint8_t pwm) {
  if (!m.configured) return;
  switch (direction) {
    case  1: digitalWrite(m.in1, HIGH); digitalWrite(m.in2, LOW);  break;
    case -1: digitalWrite(m.in1, LOW);  digitalWrite(m.in2, HIGH); break;
    default: digitalWrite(m.in1, LOW);  digitalWrite(m.in2, LOW);  pwm = 0; break;
  }
  // PWM is handled via setMotorA/B target; the EN pin is always-on here.
  // If you have a separate EN pin, call analogWrite(enPin, pwm) here.
  (void)pwm;
}

// ──────────────────────────────────────────────
//  Watchdog — stop motors if app goes silent
// ──────────────────────────────────────────────
void ESPControllerClass::_checkWatchdog() {
  if (_watchdogMs == 0) return;
  if (millis() - _lastCmdTime > _watchdogMs) {
    if (_targetA != 0 || _targetB != 0) {
      Serial.println(F("[CTRL] Watchdog timeout — stopping motors"));
      stopMotors();
    }
  }
}

// ──────────────────────────────────────────────
//  Battery monitor
// ──────────────────────────────────────────────
void ESPControllerClass::onLowBattery(uint8_t pin, uint16_t thresholdMv, void (*cb)()) {
  _battPin         = pin;
  _battThresholdMv = thresholdMv;
  _cbLowBattery    = cb;
  _battTriggered   = false;
}

void ESPControllerClass::_checkBattery() {
  if (!_cbLowBattery || _battThresholdMv == 0) return;
  // Only check every 5 seconds to avoid ADC noise
  static uint32_t lastCheck = 0;
  if (millis() - lastCheck < 5000) return;
  lastCheck = millis();

  int raw = analogRead(_battPin);
  // ESP32 ADC: 12-bit 0–4095 = 0–3300mV (3.3V ref)
  // ESP8266 ADC: 10-bit 0–1023 = 0–3300mV
#if defined(ESP32)
  uint16_t mv = (uint16_t)((uint32_t)raw * 3300 / 4095);
#else
  uint16_t mv = (uint16_t)((uint32_t)raw * 3300 / 1023);
#endif

  if (mv < _battThresholdMv) {
    if (!_battTriggered) {
      _battTriggered = true;
      Serial.printf("[BATT] Low battery! %u mV\n", mv);
      _cbLowBattery();
    }
  } else {
    _battTriggered = false;   // re-arm once voltage recovers
  }
}

// ──────────────────────────────────────────────
//  RSSI broadcast every 3 seconds
// ──────────────────────────────────────────────
void ESPControllerClass::_broadcastRSSI() {
  if (_connectedClients == 0) return;
  if (millis() - _lastRssiBroadcast < 3000) return;
  _lastRssiBroadcast = millis();

  int     rssi    = getRSSI();
  uint8_t quality = getRSSIQuality();
  String  msg     = "rssi:" + String(rssi) + ",q:" + String(quality);
  broadcast(msg);
}

// ──────────────────────────────────────────────
//  Callbacks
// ──────────────────────────────────────────────
void ESPControllerClass::onUp   (void (*cb)(uint8_t)) { _cbUp    = cb; }
void ESPControllerClass::onDown (void (*cb)(uint8_t)) { _cbDown  = cb; }
void ESPControllerClass::onLeft (void (*cb)(uint8_t)) { _cbLeft  = cb; }
void ESPControllerClass::onRight(void (*cb)(uint8_t)) { _cbRight = cb; }
void ESPControllerClass::onStop (void (*cb)(uint8_t)) { _cbStop  = cb; }

void ESPControllerClass::onJoystick(void (*cb)(int, int)) {
  _cbJoystick = cb;
}
void ESPControllerClass::onJoystickRaw(void (*cb)(float, float)) {
  _cbJoystickRaw = cb;
}
void ESPControllerClass::onControllerConnected(void (*cb)()) {
  _cbConnected = cb;
}
void ESPControllerClass::onControllerDisconnected(void (*cb)()) {
  _cbDisconnected = cb;
}

// ──────────────────────────────────────────────
//  Status / connectivity
// ──────────────────────────────────────────────
bool ESPControllerClass::isConnected() const {
  return (WiFi.status() == WL_CONNECTED) || (WiFi.softAPgetStationNum() > 0);
}

IPAddress ESPControllerClass::localIP() const {
  if (WiFi.status() == WL_CONNECTED) return WiFi.localIP();
  return WiFi.softAPIP();
}

String ESPControllerClass::localIPString() const {
  return localIP().toString();
}

int ESPControllerClass::getRSSI() const {
  return WiFi.RSSI();
}

uint8_t ESPControllerClass::getRSSIQuality() const {
  int rssi = getRSSI();
  if (rssi <= -100) return 0;
  if (rssi >= -50)  return 100;
  return (uint8_t)(2 * (rssi + 100));
}

String ESPControllerClass::getStatusJSON() const {
  String json = "{";
  json += "\"clients\":"  + String(_connectedClients);
  json += ",\"speed\":"   + String(_maxSpeed);
  json += ",\"ip\":\""    + localIPString() + "\"";
  json += ",\"rssi\":"    + String(getRSSI());
  json += ",\"rssiQ\":"   + String(getRSSIQuality());
  json += "}";
  return json;
}

// ──────────────────────────────────────────────
//  Messaging
// ──────────────────────────────────────────────
void ESPControllerClass::sendMessage(uint8_t clientId, const String& msg) {
  if (_ws) _ws->sendTXT(clientId, msg);
}

void ESPControllerClass::broadcast(const String& msg) {
  if (_ws) _ws->broadcastTXT(msg);
}

// ──────────────────────────────────────────────
//  Utility
// ──────────────────────────────────────────────
int ESPControllerClass::_clamp(int v, int lo, int hi) {
  return v < lo ? lo : (v > hi ? hi : v);
}
