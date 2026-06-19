<div align="center">

# ESPController

**Universal ESP32-CAM / ESP32 / ESP32-S3 / ESP8266 WebSocket controller library**

Control any ESP-based device wirelessly via WebSocket — rovers, camera turrets, anything.  
Comes with a Flutter Android app featuring live camera feed, virtual joystick, D-Pad and camera settings.

[![Download APK](https://img.shields.io/badge/Download-APK-green?style=for-the-badge&logo=android)](https://github.com/shamilslk/ESPController/releases/latest)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue?style=for-the-badge)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-ESP32%20%7C%20ESP32--CAM%20%7C%20ESP32--S3%20%7C%20ESP8266-orange?style=for-the-badge)](.)

</div>

---

## 📸 Screenshots

| D-Pad + Joystick + Camera | Settings | Camera Settings | Flash |
|---|---|---|---|
| ![](assets/screenshots/d_pad_joystick_camera_feed.jpeg) | ![](assets/screenshots/ip_joystick_settings.jpeg) | ![](assets/screenshots/camera_settings.jpeg) | ![](assets/screenshots/flash_working_ss.jpeg) |

---

## ✨ Features

- 🎮 **Virtual joystick** — smooth arcade-drive mixing
- 🕹️ **D-Pad** — Forward / Backward / Left / Right + Emergency Stop
- 📷 **Live camera stream** — real-time JPEG over WebSocket (ESP32-CAM)
- 💡 **Flash LED control** — brightness slider in app (flash LED or RGB NeoPixel)
- 🌈 **RGB NeoPixel status LED** — addressable LED support (auto-enabled on ESP32-S3)
- ⚙️ **Camera settings from app** — FPS, resolution (QVGA/VGA/SVGA), JPEG quality
- 🔄 **Auto reconnect** — app reconnects automatically on drop
- 📶 **RSSI broadcast** — signal quality shown in app every 3 seconds
- 🛡️ **Motor watchdog** — motors stop automatically if app goes silent
- 🚀 **Soft acceleration** — smooth speed ramp instead of jerky starts
- 🌐 **mDNS support** — reach device at `http://devicename.local`
- 🔋 **Battery monitor** — fires callback when voltage drops below threshold
- 🔍 **Auto board detection** — no manual board selection needed for standard setups

---

## 📱 App

Download the Android app from the [Releases page](https://github.com/shamilslk/ESPController/releases/latest).

The app connects to your ESP device over WiFi and gives you:
- Landscape split-screen layout: Joystick | Camera feed | D-Pad
- Connection status indicator (green/amber/red)
- Settings: change IP address, joystick sensitivity
- Camera settings: resolution, FPS, quality, flash brightness

---

## 🔧 Supported Boards

| Board | Camera | PWM Motors | Flash LED | RGB LED |
|---|---|---|---|---|
| AI Thinker ESP32-CAM | ✅ | ✅ LEDC | ✅ GPIO4 | — |
| ESP32 DevKit / WROOM | ❌ | ✅ LEDC | optional | optional |
| ESP32-S3 DevKitC | ❌ | ✅ LEDC | optional | ✅ GPIO48 (auto) |
| ESP8266 NodeMCU / Wemos D1 | ❌ | ✅ analogWrite | optional | — |

---

## 📦 Installation

### Option A — Manual (ZIP)
1. Click the green **Code** button → **Download ZIP**
2. Arduino IDE → **Sketch → Include Library → Add .ZIP Library**
3. Select the downloaded ZIP

### Option B — Clone
```bash
git clone https://github.com/shamilslk/ESPController.git
```
Copy the `ESPController` folder into your Arduino `libraries/` directory.

---

## 📚 Dependencies

### Board Packages — install via Arduino Boards Manager

| Board | Package | URL |
|---|---|---|
| ESP32 / ESP32-S3 / ESP32-CAM | `esp32` by Espressif | `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json` |
| ESP8266 | `esp8266` by ESP8266 Community | `https://arduino.esp8266.com/stable/package_esp8266com_index.json` |

### Libraries — install via Arduino Library Manager

| Library | Required for |
|---|---|
| **AsyncTCP** by me-no-dev | ESP32 / ESP32-S3 / ESP32-CAM |
| **ESPAsyncTCP** by me-no-dev | ESP8266 |
| **ESPAsyncWebServer** by me-no-dev | All boards |
| **Adafruit NeoPixel** | ESP32-S3 (required — auto-enabled) and any board with `HAS_RGB_STATUS_LED` |

Everything else (`WiFi`, `esp_camera`, `ESPmDNS`) comes bundled with the board packages.

---

## ⚡ Quick Start

### Step 1 — Board selection (usually automatic)

The library auto-detects your board from the compiler toolchain — **you don't need to do anything** for standard setups:

| Detected when | Board used |
|---|---|
| `ARDUINO_ARCH_ESP8266` set | ESP8266 |
| `CONFIG_IDF_TARGET_ESP32S3` or `ARDUINO_ESP32S3_DEV` set | ESP32-S3 |
| `BOARD_HAS_PSRAM` set (AI-Thinker CAM modules) | ESP32-CAM |
| None of the above | ESP32 |

If auto-detection picks the wrong board, override it in your `.ino` **before** the `#include`:

```cpp
#define BOARD_ESP32CAM    // AI Thinker ESP32-CAM
// #define BOARD_ESP32    // Any ESP32 without camera
// #define BOARD_ESP32S3  // ESP32-S3 (e.g. ESP32-S3-DevKitC)
// #define BOARD_ESP8266  // NodeMCU, Wemos D1, etc.
#include <ESPController.h>
```

> Define exactly **one** board if overriding. Defining multiple is a compile-time error.

### Optional feature flags

Uncomment in your `.ino` **before** the `#include` only if needed. Many are auto-enabled (see board table above).

```cpp
// #define USE_PWM_MOTORS          // LEDC / analogWrite speed control (vs simple on/off)

// #define HAS_FLASH_LED           // board has a torch/flash LED
// #define FLASH_LED_PIN  4        // which pin

// #define HAS_STATUS_LED          // board has a status LED
// #define STATUS_LED_PIN 33       // which pin

// #define HAS_RGB_STATUS_LED      // addressable NeoPixel status LED
// #define RGB_STATUS_LED_PIN 48   // which pin (auto-set to 48 on ESP32-S3)
```

For ESP32-CAM you can also override the default AI-Thinker camera pin mapping:
```cpp
// #define CAM_PIN_PWDN  32
// #define CAM_PIN_XCLK   0
// #define CAM_PIN_SIOD  26
// ... (see ESPController.h for the full list)
```

> `BOARD_ESP32CAM` auto-enables `USE_PWM_MOTORS`, `HAS_FLASH_LED`, and `HAS_STATUS_LED`.  
> `BOARD_ESP32S3` auto-enables `HAS_RGB_STATUS_LED` on GPIO 48.

---

### Step 2 — Write your sketch

```cpp
#include <ESPController.h>

void setup() {
  Serial.begin(115200);

  // WiFi — AP mode creates its own hotspot
  Controller.setWiFi("MY_DEVICE", "12345678");

  // Optional: reach device at http://mydevice.local
  Controller.setMDNS("mydevice");

  // Motor pins — match your wiring
  Controller.setMotorAPins(12, 13);   // left  motor IN1, IN2
  Controller.setMotorBPins(14, 15);   // right motor IN3, IN4

  // Tuning
  Controller.setMaxSpeed(200);                   // 0-255
  Controller.setWatchdogTimeout(500);            // ms
  Controller.setAcceleration(20, 20);            // step, tickMs
  Controller.setMotorTrim(MotorSide::LEFT, 0);   // drift fix

  // D-Pad — state is HIGH when pressed, LOW auto-fired after 300ms
  Controller.onUp([](uint8_t state) {
    if (state == HIGH) { Controller.setMotorA( 255); Controller.setMotorB( 255); }
    else               { Controller.stopMotors(); }
  });
  Controller.onDown([](uint8_t state) {
    if (state == HIGH) { Controller.setMotorA(-255); Controller.setMotorB(-255); }
    else               { Controller.stopMotors(); }
  });
  Controller.onLeft([](uint8_t state) {
    if (state == HIGH) { Controller.setMotorA(-255); Controller.setMotorB( 255); }
    else               { Controller.stopMotors(); }
  });
  Controller.onRight([](uint8_t state) {
    if (state == HIGH) { Controller.setMotorA( 255); Controller.setMotorB(-255); }
    else               { Controller.stopMotors(); }
  });
  Controller.onStop([](uint8_t state) {
    Controller.emergencyStop();
  });

  // Joystick — arcade drive mixing
  // x: -255 = full left,  +255 = full right
  // y: -255 = full back,  +255 = full forward
  Controller.onJoystick([](int x, int y) {
    Controller.setMotorA(constrain(y + x, -255, 255));
    Controller.setMotorB(constrain(y - x, -255, 255));
  });

  Controller.begin();   // starts WiFi, camera, WebSocket server
}

void loop() {
  Controller.update();  // must be called every loop
}
```

---

### Step 3 — Connect the app

1. On your phone, connect to the `MY_DEVICE` WiFi hotspot (password: `12345678`)
2. Open the ESPController app
3. The app auto-connects to `192.168.4.1`

**Using STA mode** (ESP connects to your router or phone hotspot):
```cpp
Controller.setWiFi("YourRouterSSID", "RouterPassword", ControllerWiFiMode::STA);
```
Then check Serial Monitor for the assigned IP and enter it in the app settings.

> **STA fallback:** if connection is not established within 15 seconds, the library automatically falls back to AP mode.

---

## 🔌 Wiring

### Motor driver (L298N / L9110S / TB6612)

```
ESP32-CAM GPIO12  →  Left  motor IN1
ESP32-CAM GPIO13  →  Left  motor IN2
ESP32-CAM GPIO14  →  Right motor IN1
ESP32-CAM GPIO15  →  Right motor IN2

Motor driver power  →  separate battery (DO NOT use ESP 3.3V/5V)
Motor driver GND    →  ESP GND  (grounds must be connected)
```

### Safe GPIO pins on AI Thinker ESP32-CAM

These GPIOs are **free** for motors and other use:

| GPIO | Safe to use |
|---|---|
| 12, 13 | ✅ Motor A |
| 14, 15 | ✅ Motor B |
| 2 | ✅ General use |
| 16 | ✅ General use |

**Avoid:** 0, 4, 5, 18, 19, 21, 22, 23, 25, 26, 27, 33, 34, 35, 36, 39 — used by camera, flash, or status LED.

---

## 📖 Full API Reference

### Configuration

| Function | Description |
|---|---|
| `Controller.setWiFi(ssid, pass)` | AP mode — creates hotspot |
| `Controller.setWiFi(ssid, pass, ControllerWiFiMode::STA)` | STA mode — joins router (falls back to AP on failure) |
| `Controller.setMDNS("name")` | Reach device at `http://name.local` |
| `Controller.setMotorAPins(in1, in2)` | Motor A GPIO pins |
| `Controller.setMotorBPins(in1, in2)` | Motor B GPIO pins |
| `Controller.setMaxSpeed(0-255)` | Global speed cap (applied after trim) |
| `Controller.setMotorTrim(MotorSide::LEFT, -100 to +100)` | Fix drift |
| `Controller.setWatchdogTimeout(ms)` | Auto-stop if app goes silent. 0 = disabled |
| `Controller.setAcceleration(step, tickMs)` | Ramp speed. 255 step = instant |

### Callbacks

| Callback | When it fires |
|---|---|
| `Controller.onUp(cb)` | `state=HIGH` on press, `state=LOW` auto-fired after 300ms |
| `Controller.onDown(cb)` | same |
| `Controller.onLeft(cb)` | same |
| `Controller.onRight(cb)` | same |
| `Controller.onStop(cb)` | `state=HIGH` on press only |
| `Controller.onJoystick(cb)` | `cb(int x, int y)` — x,y each −255…+255 |
| `Controller.onJoystickRaw(cb)` | `cb(float x, float y)` — x,y each −1.0…+1.0 |
| `Controller.onControllerConnected(cb)` | App control socket connects |
| `Controller.onControllerDisconnected(cb)` | App disconnects (motors already stopped) |
| `Controller.onCameraConnected(cb)` | Camera viewer opens (ESP32-CAM only) |
| `Controller.onCameraDisconnected(cb)` | Camera viewer closes (ESP32-CAM only) |
| `Controller.onLowBattery(pin, mV, cb)` | Voltage on ADC pin drops below threshold |

### Motor helpers

| Function | Description |
|---|---|
| `Controller.setMotorA(speed)` | −255 to +255. Trim + maxSpeed + ramp applied |
| `Controller.setMotorB(speed)` | same for motor B |
| `Controller.stopMotors()` | Gradual stop — respects acceleration ramp |
| `Controller.emergencyStop()` | Instant cut — bypasses ramp, also kills flash / RGB LED |

### RGB LED helpers

Available on `BOARD_ESP32S3` (auto-enabled on GPIO 48) and any board with `HAS_RGB_STATUS_LED` defined.

| Function | Description |
|---|---|
| `Controller.setRGB(r, g, b)` | Set NeoPixel colour — r/g/b each 0–255 |
| `Controller.setRGBOff()` | Turn NeoPixel off |

> Safe to call before `Controller.begin()` — hardware is lazily initialised on first use.

### Connectivity

| Function | Returns |
|---|---|
| `Controller.getRSSI()` | Signal in dBm (e.g. −65) |
| `Controller.getRSSIQuality()` | 0–100 score |
| `Controller.getStatusJSON()` | Full status as JSON string |

`getStatusJSON()` always includes: `ip`, `rssi`, `rssi_quality`, `ctrl_clients`, `max_speed`.  
Additional fields by feature: `cam_clients` + `fps` (ESP32-CAM), `flash` (any board with `HAS_FLASH_LED`).

### Lifecycle

| Function | When to call |
|---|---|
| `Controller.begin()` | End of `setup()` |
| `Controller.update()` | Every `loop()` — drives everything |

---

## 🔄 What the library handles automatically

You never write code for these — the library and app handle them together:

| Feature | Detail |
|---|---|
| Flash LED | App sends `flash:180` → library sets brightness via LEDC / analogWrite |
| RGB NeoPixel | `flash:` command also drives NeoPixel as white on boards with `HAS_RGB_STATUS_LED` |
| Camera FPS | App sends `cam:fps=20` → library applies it (ESP32-CAM only) |
| Camera resolution | App sends `cam:res=VGA` → QVGA / VGA / SVGA (ESP32-CAM only) |
| JPEG quality | App sends `cam:quality=10` → library applies it (ESP32-CAM only) |
| RSSI broadcast | Sends `rssi:-65,q:70` to app every 3 seconds |
| Button auto-release | `LOW` fired automatically 300ms after last press |
| Emergency stop | Called automatically on controller disconnect; kills flash / RGB LED |
| Watchdog | Motors zeroed if no command received within `setWatchdogTimeout()` ms |
| WebSocket cleanup | Stale clients cleaned up every loop |
| mDNS update | `MDNS.update()` called automatically in `update()` on ESP8266 |
| HTTP fallback | `/up` `/down` `/left` `/right` `/stop` `/joystick` `/cam` `/status` |
| Serial logging | Connect/disconnect/commands printed automatically |
| STA → AP fallback | Falls back to AP mode if STA connection fails within 15 s |
| Status LED blink | 2 blinks on successful `begin()`; rapid blink on ESP32-CAM camera fault |

---

## 🧪 Examples

Upload these in order when building a new rover:

| Sketch | What it tests | Hardware needed |
|---|---|---|
| `example1_led_test` | WiFi connects, callbacks fire, app sends commands | Just ESP32-CAM |
| `example2_serial_test` | Joystick + D-Pad values in Serial Monitor | Just ESP32-CAM |
| `example3_single_motor` | One motor direction + speed | Motor driver + 1 motor |
| `example4_two_motors_drift` | Both motors + drift trim | Motor driver + 2 motors |
| `example5_joystick_rover` | Arcade drive + acceleration feel | Motor driver + 2 motors |
| `example6_full_rover` | Everything together — final sketch | Complete rover |
| `example_esp32cam_rover` | Library example for ESP32-CAM | Complete rover |
| `example_esp32_rover` | Library example for plain ESP32 | ESP32 + motors |
| `example_esp32s3_rover` | Library example for ESP32-S3 | ESP32-S3 + motors |
| `example_esp8266` | Library example for ESP8266 | ESP8266 + motors |

**Expected Serial Monitor output on boot:**
```
[CTRL] ESPController starting...
[MOTORS] Ready
[WIFI] AP IP: 192.168.4.1
[CAM] OK
[HTTP] Server ready on port 80
[CTRL] Ready
```
Then when app connects:
```
[CTRL] App connected
```

---

## ❓ Troubleshooting

**Camera init failed / rapid LED blink on boot**
- Wrong board selected — make sure `BOARD_ESP32CAM` is active (or let auto-detection run)
- Power issue — ESP32-CAM needs a stable 5V supply. USB from PC is often not enough. Use a phone charger or dedicated 5V supply
- Bad USB cable — try a different cable

**Motors spin wrong direction**
- Swap the two motor wire connections on the terminal block, OR
- Swap the IN1/IN2 pin numbers in `setMotorAPins()`

**Rover drifts left or right**
- One motor is physically faster than the other — normal for cheap motors
- Use `setMotorTrim()` to correct it — see `example4_two_motors_drift`

**App shows disconnected (red icon)**
- Make sure phone is connected to `MY_DEVICE` hotspot, not your home router
- Check IP in app settings matches your ESP IP
- Restart the ESP and reconnect

**Compilation error: "undefined reference" or "multiple definition"**
- Make sure you are not defining more than one board macro
- If you defined a board manually, verify it matches the board selected in Arduino IDE → Tools → Board

**NeoPixel / RGB LED not lighting up (ESP32-S3)**
- Adafruit NeoPixel library must be installed — install it via Arduino Library Manager
- GPIO 48 is the default; override with `#define RGB_STATUS_LED_PIN <pin>` before the `#include` if your board differs

**Short WiFi range**
- AI Thinker ESP32-CAM has a U.FL/IPEX connector for an external antenna
- Move resistor R21 from PCB antenna side to external antenna side
- Attach a 2.4GHz external antenna — range improves from ~20m to ~80m

---

## 👥 Authors

- **Shamil K**
- **Vismaya P**

## 📄 License

MIT License — see [LICENSE](LICENSE) file for details.
