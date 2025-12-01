#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// GPIO Pin Configuration
// ============================================
// WS2811 LED Data Pin
// Using GPIO 13 - safe for boot, supports RMT peripheral
// Other suitable pins: GPIO 12, 14, 27, 26, 25, 33, 32
// Avoid: GPIO 0, 2, 5, 15 (boot pins), GPIO 6-11 (flash), GPIO 34-39 (input only)
#define LED_DATA_PIN 13

// ============================================
// LED Strip Configuration
// ============================================
#define NUM_LEDS 50
#define LED_TYPE WS2811
#define COLOR_ORDER RGB
#define DEFAULT_BRIGHTNESS 128  // 0-255

// ============================================
// WiFi Configuration
// ============================================
// Replace with your WiFi credentials
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// Access Point fallback (if WiFi connection fails)
#define AP_SSID "XmasLights"
#define AP_PASSWORD "christmas123"

// ============================================
// Web Server Configuration
// ============================================
#define WEB_SERVER_PORT 80

// ============================================
// Animation Settings
// ============================================
#define DEFAULT_ANIMATION_SPEED 50  // ms between frames
#define MAX_ANIMATIONS 10

#endif // CONFIG_H
