# ESP32 Christmas Lights Controller

WiFi-enabled controller for WS2811 LED strips with a web interface and 3D position calibration for spatially-aware animations.

## Features

- Control 50 individually addressable WS2811 LEDs
- Mobile-friendly web interface
- 14 animation modes including 5 spatial animations that use 3D calibration
- 3D position calibration for optically-aligned effects on wrapped/3D installations
- Calibration data persists across reboots
- WiFi with automatic AP fallback

## Hardware Requirements

- ESP32 development board
- WS2811 LED strip (50 LEDs)
- 5V power supply (sized for your LED count, ~60mA per LED at full white)
- 330-470Ω resistor on data line (recommended)

## Wiring

| ESP32 | WS2811 Strip |
|-------|--------------|
| GPIO 13 | Data In |
| GND | GND |

> **Note:** Power the LED strip directly from your 5V supply, not through the ESP32. Connect grounds together.

## Software Setup

### Prerequisites

- [PlatformIO](https://platformio.org/) (VS Code extension or CLI)

### Configuration

1. Clone this repository:
   ```bash
   git clone https://github.com/mrprkr/xmas-lights.git
   cd xmas-lights
   ```

2. Edit `include/config.h` with your WiFi credentials:
   ```cpp
   #define WIFI_SSID "your-wifi-name"
   #define WIFI_PASSWORD "your-wifi-password"
   ```

3. Build and upload:
   ```bash
   pio run --target upload
   ```

4. Monitor serial output to get the IP address:
   ```bash
   pio device monitor
   ```

5. Open the IP address in your browser to access the control interface.

### AP Fallback Mode

If WiFi connection fails, the ESP32 creates an access point:
- SSID: `XmasLights`
- Password: `christmas123`
- Interface: `http://192.168.4.1`

## Web Interface

### Control Tab

- **Power** - On/off toggle
- **Brightness** - 0-100%
- **Color** - Color picker for static color and color-based animations
- **Animations** - 9 basic + 5 spatial animation modes
- **Speed** - Animation speed control

### Calibrate Tab

For installations where LEDs are wrapped around 3D objects (trees, sculptures, etc.), calibration maps each LED's physical position in 3D space. Spatial animations then appear optically correct rather than following the electrical sequence.

1. Click **Start** to begin calibration
2. The current LED lights white (previous=red, next=green for orientation)
3. Adjust **X**, **Y**, **Z** sliders to match the LED's physical position
4. Click **Next** to advance through all 50 LEDs
5. Click **Save** to persist calibration to flash storage

## Animation Modes

### Basic Animations
| Mode | Description |
|------|-------------|
| Static | Solid color |
| Rainbow | Sequential rainbow cycle |
| Chase | Single LED chases along strip |
| Twinkle | Random fade in/out |
| Fade | Whole strip breathes |
| Sparkle | Random white flashes |
| Candy Cane | Red/white stripes |
| Snow | Falling white flakes |
| Fire | Flame simulation |

### Spatial Animations (use 3D calibration)
| Mode | Description |
|------|-------------|
| Wave | Sine wave travels through 3D space |
| 3D Rainbow | Hue mapped to X+Y position |
| Pulse | Expands outward from center |
| Rotate | Beam sweeps around vertical axis |
| Planes | Horizontal color bands move up/down |

## REST API

| Endpoint | Method | Payload |
|----------|--------|---------|
| `/api/state` | GET | Current state |
| `/api/power` | POST | `{ "on": true }` |
| `/api/brightness` | POST | `{ "brightness": 128 }` |
| `/api/color` | POST | `{ "r": 255, "g": 0, "b": 0 }` |
| `/api/animation` | POST | `{ "mode": 1 }` |
| `/api/speed` | POST | `{ "speed": 50 }` |
| `/api/calibration` | GET | All LED positions |
| `/api/calibration/mode` | POST | `{ "led": 0 }` or `{ "led": -1 }` to exit |
| `/api/calibration/position` | POST | `{ "led": 0, "x": 0.5, "y": -0.3, "z": 0.1 }` |
| `/api/calibration/save` | POST | Save to flash |
| `/api/calibration/reset` | POST | Reset to linear |

## License

MIT
