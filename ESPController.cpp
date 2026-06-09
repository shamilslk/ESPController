/*
 * ESPController.cpp
 * -----------------
 * Implementation of the ESPController library.
 * See ESPController.h for full API documentation.
 *
 * License: MIT
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
  #settings-btn{position:absolute;top:8px;right:8px;background:#252540;border:none;
                border-radius:6px;color:var(--text);padding:4px 10px;cursor:pointer;font-size:13px;}
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
    <button id="settings-btn">⚙ Settings</button>
  </div>
  <div id="ctrl-panel">
    <div class="dpad">
      <div class="dpad-empty"></div>
      <button class="dpad-btn" data-cmd="F" id="btn-f">▲</button>
      <div class="dpad-empty"></div>
      <button class="dpad-btn" data-cmd="L" id="btn-l">◀</button>
      <div class="dpad-empty"></div>
      <button class="dpad-btn" data-cmd="R" id="btn-r">▶</button>
      <div class="dpad-empty"></div>
      <button class="dpad-btn" data-cmd="B" id="btn-b">▼</button>
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
const dot = document.getElementById('dot');
const statusText = document.getElementById('status-text');
const ipLabel = document.getElementById('ip-label');
ipLabel.textContent = location.hostname;
let ws, reconnectTimer;
function connect(){
  ws = new WebSocket('ws://'+location.hostname+':81/');
  ws.onopen=()=>{ dot.className='connected'; statusText.textContent='Connected'; };
  ws.onclose=()=>{ dot.className='reconnecting'; statusText.textContent='Reconnecting…';
                   reconnectTimer=setTimeout(connect,2000); };
  ws.onerror=()=>ws.close();
}
connect();
function send(msg){ if(ws&&ws.readyState===1) ws.send(msg); }
// D-Pad
document.querySelectorAll('.dpad-btn').forEach(btn=>{
  btn.addEventListener('pointerdown',e=>{e.preventDefault();btn.classList.add('pressed');send(btn.dataset.cmd);});
  btn.addEventListener('pointerup',  e=>{btn.classList.remove('pressed');send('S');});
  btn.addEventListener('pointerleave',e=>{btn.classList.remove('pressed');send('S');});
});
document.getElementById('stop-btn').addEventListener('click',()=>send('S'));
// Speed slider
const speedSlider=document.getElementById('speed-slider');
const speedVal=document.getElementById('speed-val');
speedSlider.addEventListener('input',()=>{
  speedVal.textContent=speedSlider.value;
  send('V'+speedSlider.value);
});
// Joystick
const zone=document.getElementById('joystick-zone');
const knob=document.getElementById('joystick-knob');
const R=55; let jActive=false, jOrigin={x:0,y:0};
zone.addEventListener('pointerdown',e=>{
  jActive=true; zone.setPointerCapture(e.pointerId);
  const r=zone.getBoundingClientRect();
  jOrigin={x:r.left+r.width/2,y:r.top+r.height/2};
  moveKnob(e);
});
zone.addEventListener('pointermove',e=>{ if(jActive) moveKnob(e); });
zone.addEventListener('pointerup', resetKnob);
zone.addEventListener('pointercancel', resetKnob);
function moveKnob(e){
  let dx=e.clientX-jOrigin.x, dy=e.clientY-jOrigin.y;
  const dist=Math.min(Math.hypot(dx,dy),R);
  const angle=Math.atan2(dy,dx);
  dx=Math.cos(angle)*dist; dy=Math.sin(angle)*dist;
  knob.style.left=(50+dx/R*45)+'%';
  knob.style.top=(50+dy/R*45)+'%';
  const jx=Math.round(dx/R*100), jy=-Math.round(dy/R*100);
  send('J'+jx+','+jy);
}
function resetKnob(){
  jActive=false;
  knob.style.left='50%'; knob.style.top='50%';
  send('S');
}
</script>
</body>
</html>
)rawhtml";

// ──────────────────────────────────────────────
//  Constructor
// ──────────────────────────────────────────────
ESPController::ESPController()
  : _server(nullptr), _ws(nullptr),
    _port(80), _wsPort(81),
    _motorsConfigured(false), _speed(200),
    _onCommandCb(nullptr), _onConnectCb(nullptr), _onDisconnectCb(nullptr),
    _connectedClients(0)
{
  _leftMotor  = {0, 0, 255};
  _rightMotor = {0, 0, 255};
}

// ──────────────────────────────────────────────
//  WiFi / server setup
// ──────────────────────────────────────────────
void ESPController::beginAP(const char* ssid, const char* password,
                             uint16_t port, uint16_t wsPort) {
  _port   = port;
  _wsPort = wsPort;

  if (strlen(password) >= 8) {
    WiFi.softAP(ssid, password);
  } else {
    WiFi.softAP(ssid);
  }

  Serial.print(F("[ESPController] AP IP: "));
  Serial.println(WiFi.softAPIP());

  _startServers();
}

bool ESPController::beginSTA(const char* ssid, const char* password,
                              uint32_t timeoutMs, uint16_t port, uint16_t wsPort) {
  _port   = port;
  _wsPort = wsPort;

  WiFi.begin(ssid, password);
  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - start > timeoutMs) {
      Serial.println(F("[ESPController] WiFi connection timed out"));
      return false;
    }
    delay(200);
    Serial.print('.');
  }
  Serial.print(F("\n[ESPController] STA IP: "));
  Serial.println(WiFi.localIP());

  _startServers();
  return true;
}

void ESPController::_startServers() {
#if defined(ESP8266)
  _server = new ESP8266WebServer(_port);
#else
  _server = new WebServer(_port);
#endif
  _ws = new WebSocketsServer(_wsPort);

  _setupRoutes();
  _server->begin();

  _ws->begin();
  _ws->onEvent([this](uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    _onWebSocketEvent(num, type, payload, length);
  });

  Serial.printf("[ESPController] HTTP on port %u, WS on port %u\n", _port, _wsPort);
}

// ──────────────────────────────────────────────
//  loop() handler
// ──────────────────────────────────────────────
void ESPController::handle() {
  if (_server) _server->handleClient();
  if (_ws)     _ws->loop();
}

// ──────────────────────────────────────────────
//  HTTP routes
// ──────────────────────────────────────────────
void ESPController::_setupRoutes() {
  _server->on("/", HTTP_GET, [this]() { _handleRoot(); });
  _server->on("/cmd", HTTP_GET, [this]() { _handleCommand(); });
  _server->on("/status", HTTP_GET, [this]() { _handleStatus(); });
  _server->onNotFound([this]() {
    _server->send(404, "text/plain", "Not found");
  });
}

void ESPController::_handleRoot() {
  _server->send_P(200, "text/html", CONTROLLER_HTML);
}

void ESPController::_handleCommand() {
  if (!_server->hasArg("c")) {
    _server->send(400, "text/plain", "Missing ?c=");
    return;
  }
  String raw = _server->arg("c");
  if (raw.length() == 0) {
    _server->send(400, "text/plain", "Empty command");
    return;
  }
  char cmd     = raw[0];
  String payload = raw.substring(1);
  _processCommand(cmd, payload);
  _server->send(200, "text/plain", "OK");
}

void ESPController::_handleStatus() {
  String json = "{\"clients\":";
  json += _connectedClients;
  json += ",\"speed\":";
  json += _speed;
  json += ",\"ip\":\"";
  json += localIPString();
  json += "\"}";
  _server->send(200, "application/json", json);
}

// ──────────────────────────────────────────────
//  WebSocket event handler
// ──────────────────────────────────────────────
void ESPController::_onWebSocketEvent(uint8_t num, WStype_t type,
                                       uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      _connectedClients++;
      Serial.printf("[WS] Client #%u connected\n", num);
      if (_onConnectCb) _onConnectCb(num);
      break;

    case WStype_DISCONNECTED:
      if (_connectedClients > 0) _connectedClients--;
      Serial.printf("[WS] Client #%u disconnected\n", num);
      motorStop();  // safety stop on disconnect
      if (_onDisconnectCb) _onDisconnectCb(num);
      break;

    case WStype_TEXT:
      if (length > 0) {
        char cmd = (char)payload[0];
        String data = "";
        if (length > 1) {
          payload[length] = '\0';
          data = String((char*)payload + 1);
        }
        _processCommand(cmd, data);
      }
      break;

    default:
      break;
  }
}

// ──────────────────────────────────────────────
//  Command dispatcher
// ──────────────────────────────────────────────
void ESPController::_processCommand(char cmd, const String& payload) {
  switch (cmd) {
    case CMD_FORWARD:   motorForward();  break;
    case CMD_BACKWARD:  motorBackward(); break;
    case CMD_LEFT:      motorLeft();     break;
    case CMD_RIGHT:     motorRight();    break;
    case CMD_STOP:      motorStop();     break;

    case CMD_JOYSTICK: {
      int comma = payload.indexOf(',');
      if (comma > 0) {
        int x = payload.substring(0, comma).toInt();
        int y = payload.substring(comma + 1).toInt();
        motorJoystick(x, y);
      }
      break;
    }

    case CMD_SPEED:
      _speed = (uint8_t)_clamp(payload.toInt(), 0, 255);
      break;

    default:
      break;
  }

  if (_onCommandCb) _onCommandCb(cmd, payload);
}

// ──────────────────────────────────────────────
//  Motor pin configuration
// ──────────────────────────────────────────────
void ESPController::setMotorPins(MotorPins left, MotorPins right) {
  _leftMotor  = left;
  _rightMotor = right;
  _motorsConfigured = true;

  // Configure pin modes
  auto setup = [](const MotorPins& m) {
    pinMode(m.in1, OUTPUT);
    pinMode(m.in2, OUTPUT);
    if (m.en != 255) pinMode(m.en, OUTPUT);
  };
  setup(_leftMotor);
  setup(_rightMotor);
}

void ESPController::setSpeed(uint8_t speed) {
  _speed = speed;
}

// ──────────────────────────────────────────────
//  Low-level motor driver
//  direction: +1 = forward, -1 = backward, 0 = coast
// ──────────────────────────────────────────────
void ESPController::_driveMotor(const MotorPins& m, int8_t direction, uint8_t pwm) {
  if (!_motorsConfigured) return;

  switch (direction) {
    case  1:
      digitalWrite(m.in1, HIGH);
      digitalWrite(m.in2, LOW);
      break;
    case -1:
      digitalWrite(m.in1, LOW);
      digitalWrite(m.in2, HIGH);
      break;
    default:
      digitalWrite(m.in1, LOW);
      digitalWrite(m.in2, LOW);
      pwm = 0;
      break;
  }

  if (m.en != 255) analogWrite(m.en, pwm);
}

// ──────────────────────────────────────────────
//  High-level motor commands
// ──────────────────────────────────────────────
void ESPController::motorForward() {
  _driveMotor(_leftMotor,  1, _speed);
  _driveMotor(_rightMotor, 1, _speed);
}

void ESPController::motorBackward() {
  _driveMotor(_leftMotor,  -1, _speed);
  _driveMotor(_rightMotor, -1, _speed);
}

void ESPController::motorLeft() {
  _driveMotor(_leftMotor,  -1, _speed);
  _driveMotor(_rightMotor,  1, _speed);
}

void ESPController::motorRight() {
  _driveMotor(_leftMotor,   1, _speed);
  _driveMotor(_rightMotor, -1, _speed);
}

void ESPController::motorStop() {
  _driveMotor(_leftMotor,  0, 0);
  _driveMotor(_rightMotor, 0, 0);
}

void ESPController::motorJoystick(int x, int y) {
  // Differential drive mixing
  // y = forward/backward  (-100 … +100)
  // x = left/right steer  (-100 … +100)
  int leftSpeed  = _clamp(y + x, -100, 100);
  int rightSpeed = _clamp(y - x, -100, 100);

  uint8_t lPwm = (uint8_t)(abs(leftSpeed)  * _speed / 100);
  uint8_t rPwm = (uint8_t)(abs(rightSpeed) * _speed / 100);

  _driveMotor(_leftMotor,  leftSpeed  > 0 ? 1 : (leftSpeed  < 0 ? -1 : 0), lPwm);
  _driveMotor(_rightMotor, rightSpeed > 0 ? 1 : (rightSpeed < 0 ? -1 : 0), rPwm);
}

// ──────────────────────────────────────────────
//  Callbacks
// ──────────────────────────────────────────────
void ESPController::onCommand(void (*cb)(char, const String&)) {
  _onCommandCb = cb;
}

void ESPController::onClientConnect(void (*cb)(uint8_t)) {
  _onConnectCb = cb;
}

void ESPController::onClientDisconnect(void (*cb)(uint8_t)) {
  _onDisconnectCb = cb;
}

// ──────────────────────────────────────────────
//  Status helpers
// ──────────────────────────────────────────────
bool ESPController::isConnected() const {
#if defined(ESP8266)
  return (WiFi.status() == WL_CONNECTED) || (WiFi.softAPgetStationNum() > 0);
#else
  return (WiFi.status() == WL_CONNECTED) || (WiFi.softAPgetStationNum() > 0);
#endif
}

IPAddress ESPController::localIP() const {
  if (WiFi.status() == WL_CONNECTED) return WiFi.localIP();
  return WiFi.softAPIP();
}

String ESPController::localIPString() const {
  return localIP().toString();
}

// ──────────────────────────────────────────────
//  WebSocket messaging
// ──────────────────────────────────────────────
void ESPController::sendMessage(uint8_t clientId, const String& msg) {
  if (_ws) _ws->sendTXT(clientId, msg);
}

void ESPController::broadcast(const String& msg) {
  if (_ws) _ws->broadcastTXT(msg);
}

// ──────────────────────────────────────────────
//  Utility
// ──────────────────────────────────────────────
int ESPController::_clamp(int v, int lo, int hi) {
  return v < lo ? lo : (v > hi ? hi : v);
}
