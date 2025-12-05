# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ESP32-S3-Touch-LCD-2 based Christmas lights controller for WS2811 LED strips (50 individually addressable LEDs) with:
- 2-inch 240x320 capacitive touchscreen for local control
- WiFi connectivity with web-based remote control
- 3D position calibration for spatially-aware animations
- SD card storage for calibration data

## Hardware

**Board:** Waveshare ESP32-S3-Touch-LCD-2
- ESP32-S3R8 (8MB PSRAM, 16MB Flash)
- 2-inch 240x320 LCD (ST7789T3 driver)
- CST816D capacitive touch controller
- TF/SD card slot (FAT32)
- QMI8658 IMU (not used)

## Build Commands

```bash
# Build the project
pio run

# Upload to ESP32-S3
pio run --target upload

# Monitor serial output (USB CDC)
pio device monitor

# Build and upload with monitoring
pio run --target upload && pio device monitor

# Clean build files
pio run --target clean
```

## GPIO Pin Configuration

| Function | GPIO | Notes |
|----------|------|-------|
| WS2811 Data | 13 | LED strip data pin |
| LCD SCLK | 39 | SPI Clock |
| LCD MOSI | 38 | SPI Data Out |
| LCD MISO | 40 | SPI Data In (not used) |
| LCD DC | 42 | Data/Command |
| LCD CS | 45 | Chip Select |
| LCD RST | -1 | Not used (connected to EN) |
| LCD BL | 1 | Backlight PWM |
| Touch SDA | 48 | I2C Data |
| Touch SCL | 47 | I2C Clock |
| SD MOSI | 41 | SPI Data Out |
| SD MISO | 39 | SPI Data In |
| SD SCLK | 42 | SPI Clock |
| SD CS | 38 | Chip Select |

## Architecture

```
src/
├── main.cpp           # Entry point, initialization sequence
├── display.cpp        # Arduino_GFX display driver, LVGL integration
├── display_ui.cpp     # LVGL touch interface (controls, calibration, settings)
├── led_controller.cpp # FastLED wrapper, basic + spatial animations
├── calibration.cpp    # 3D position storage, SD card persistence
├── wifi_manager.cpp   # WiFi STA/AP mode handling
└── web_server.cpp     # AsyncWebServer, REST API, embedded HTML

include/
├── config.h           # All configuration constants and pin definitions
├── display.h          # Display driver interface
├── display_ui.h       # Touch UI interface
├── led_controller.h   # LED types, animation modes enum
├── calibration.h      # LEDPosition struct, sorted index helpers
├── wifi_manager.h     # WiFi state machine
├── web_server.h       # API handler declarations
└── lv_conf.h          # LVGL configuration
```

## Key Libraries

- **FastLED** - WS2811 LED control via RMT peripheral
- **GFX Library for Arduino** (Arduino_GFX) - Display driver for ST7789
- **LVGL** - Touch UI framework
- **ESPAsyncWebServer** - Non-blocking web server
- **ArduinoJson** - REST API JSON parsing
- **SD** - SD card access for calibration storage

## Touch Interface

The 2-inch touchscreen provides three tabs:

**Control Tab:**
- Power on/off switch
- Brightness slider (0-255)
- Color wheel for static color selection
- Animation mode buttons (9 basic animations)
- Speed slider (10-200ms)

**Calibrate Tab:**
- LED navigation (prev/next/start)
- X/Y/Z position sliders for 3D calibration
- Save/Exit buttons

**Settings Tab:**
- WiFi connection status
- IP address display
- Device information

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
| `/api/calibration/save` | POST | Saves to SD card |
| `/api/calibration/reset` | POST | Resets to linear layout |

## Animation Modes

**Basic (0-8):** Static, Rainbow, Chase, Twinkle, Fade, Sparkle, Candy Cane, Snow, Fire

**Spatial (9-13):** Wave, 3D Rainbow, Pulse, Rotate, Planes - these use calibrated 3D positions

## SD Card Storage

Calibration data is stored on the SD card at `/xmas/calibration.json`. The SD card must be formatted as FAT32. If no SD card is present, default linear calibration is used.

## WiFi Behavior

Attempts WiFi STA connection first. Falls back to AP mode ("XmasLights" / "christmas123") after 10 seconds if connection fails.

## Configuration

Edit `include/config.h` to set WiFi credentials before first upload:
```cpp
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
```

## Pin Adjustments

If using a different ESP32-S3 board variant, you may need to adjust pin definitions in `include/config.h`. The current configuration is optimized for the Waveshare ESP32-S3-Touch-LCD-2 board.
