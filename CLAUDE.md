# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ESP32-based Christmas lights controller for WS2811 LED strips (50 individually addressable LEDs) with WiFi connectivity, web-based remote control, and 3D position calibration for spatially-aware animations.

## Build Commands

```bash
# Build the project
pio run

# Upload to ESP32
pio run --target upload

# Monitor serial output
pio device monitor

# Build and upload with monitoring
pio run --target upload && pio device monitor

# Clean build files
pio run --target clean
```

## GPIO Pin Configuration

| Function | GPIO | Notes |
|----------|------|-------|
| WS2811 Data | 13 | Safe boot pin, RMT peripheral support |

**Safe alternative pins:** 12, 14, 27, 26, 25, 33, 32

**Pins to avoid:**
- GPIO 0, 2, 5, 15 - Boot strapping pins
- GPIO 6-11 - Connected to flash memory
- GPIO 34-39 - Input only (no output capability)

## Architecture

```
src/
├── main.cpp           # Entry point, initialization sequence
├── led_controller.cpp # FastLED wrapper, basic + spatial animations
├── calibration.cpp    # 3D position storage, sorting, SPIFFS persistence
├── wifi_manager.cpp   # WiFi STA/AP mode handling
└── web_server.cpp     # AsyncWebServer, REST API, embedded HTML

include/
├── config.h           # All configuration constants
├── led_controller.h   # LED types, animation modes enum
├── calibration.h      # LEDPosition struct, sorted index helpers
├── wifi_manager.h     # WiFi state machine
└── web_server.h       # API handler declarations
```

## Key Libraries

- **FastLED** - WS2811 LED control via RMT peripheral
- **ESPAsyncWebServer** - Non-blocking web server
- **ArduinoJson** - REST API JSON parsing
- **SPIFFS** - Calibration data persistence

## REST API Endpoints

| Endpoint | Method | Payload |
|----------|--------|---------|
| `/api/state` | GET | Returns full state |
| `/api/power` | POST | `{ "on": bool }` |
| `/api/brightness` | POST | `{ "brightness": 0-255 }` |
| `/api/color` | POST | `{ "r": 0-255, "g": 0-255, "b": 0-255 }` |
| `/api/animation` | POST | `{ "mode": 0-13 }` |
| `/api/speed` | POST | `{ "speed": ms }` |
| `/api/calibration` | GET | Returns all LED positions |
| `/api/calibration/mode` | POST | `{ "led": -1 to 49 }` (-1 exits calibration) |
| `/api/calibration/position` | POST | `{ "led": 0-49, "x": -1 to 1, "y": -1 to 1, "z": -1 to 1 }` |
| `/api/calibration/save` | POST | Saves to SPIFFS |
| `/api/calibration/reset` | POST | Resets to linear layout |

## Animation Modes

**Basic (0-8):** Static, Rainbow, Chase, Twinkle, Fade, Sparkle, Candy Cane, Snow, Fire

**Spatial (9-13):** Wave, 3D Rainbow, Pulse, Rotate, Planes - these use calibrated 3D positions

## 3D Calibration System

LEDs can be calibrated with X/Y/Z coordinates (-1.0 to 1.0) representing their physical position in 3D space. This enables optically-aligned animations when lights are wrapped around objects.

**Calibration workflow:**
1. Open web interface, go to "Calibrate" tab
2. Click "Start" - LED #1 lights white (neighbors show red/green)
3. Adjust X/Y/Z sliders to match physical position
4. Click "Next" to advance through all 50 LEDs
5. Click "Save" to persist to SPIFFS

Calibration data is stored in `/calibration.json` on SPIFFS and loaded on boot.

## WiFi Behavior

Attempts WiFi STA connection first. Falls back to AP mode ("XmasLights" / "christmas123") after 10 seconds if connection fails.

## Configuration

Edit `include/config.h` to set WiFi credentials before first upload:
```cpp
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
```
