#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// Board: ESP32-S3-Touch-LCD-2 (Waveshare)
// ============================================
// 2-inch 240x320 LCD with ST7789T3 driver
// CST816D capacitive touch controller
// ESP32-S3R8 with 8MB PSRAM, 16MB Flash
// ============================================

// ============================================
// GPIO Pin Configuration - WS2811 LEDs
// ============================================
// Using GPIO 13 - compatible with ESP32-S3
// Note: Camera Y3 uses GPIO 13, but we're not using camera
#define LED_DATA_PIN 13

// ============================================
// LED Strip Configuration
// ============================================
#define NUM_LEDS 50
#define LED_TYPE WS2811
#define COLOR_ORDER RGB
#define DEFAULT_BRIGHTNESS 128 // 0-255

// ============================================
// Display Configuration - ST7789T3
// ============================================
// SPI interface pins for LCD (from Waveshare ESP32-S3-LCD-2 demo)
#define LCD_SCLK    39   // SPI Clock
#define LCD_MOSI    38   // SPI Data Out (MOSI)
#define LCD_MISO    40   // SPI Data In (MISO) - not used but defined
#define LCD_DC      42   // Data/Command
#define LCD_CS      45   // Chip Select
#define LCD_RST     -1   // Reset (not used, directly connected to EN)
#define LCD_BL      1    // Backlight PWM

// Display parameters
#define LCD_WIDTH   240
#define LCD_HEIGHT  320
#define LCD_ROTATION 0   // 0, 1, 2, or 3 (90 degree increments)

// ============================================
// Touch Configuration - CST816D
// ============================================
// I2C interface pins for touch controller (from Waveshare ESP32-S3-LCD-2 demo)
#define TOUCH_SDA   48   // I2C Data
#define TOUCH_SCL   47   // I2C Clock

#define TOUCH_I2C_ADDR 0x15  // CST816D I2C address

// ============================================
// SD Card Configuration
// ============================================
// SPI interface pins for SD card
#define SD_MOSI     41
#define SD_MISO     39
#define SD_SCLK     42
#define SD_CS       38

// Calibration file path on SD card
#define CALIBRATION_FILE_PATH "/xmas/calibration.json"

// ============================================
// WiFi Configuration
// ============================================
// Replace with your WiFi credentials
#define WIFI_SSID "Par2"
#define WIFI_PASSWORD "Hongkong88"

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
#define DEFAULT_ANIMATION_SPEED 50 // ms between frames
#define MAX_ANIMATIONS 15

// ============================================
// UI Settings
// ============================================
#define UI_UPDATE_INTERVAL 16    // ~60fps for LVGL
#define DISPLAY_TIMEOUT 30000    // Screen dim after 30 seconds of inactivity (0 = disabled)

// ============================================
// Animation Names for UI
// ============================================
static const char* ANIMATION_NAMES[] = {
    "Static",
    "Rainbow",
    "Chase",
    "Twinkle",
    "Fade",
    "Sparkle",
    "Candy Cane",
    "Snow",
    "Fire",
    "Wave",
    "3D Rainbow",
    "Pulse",
    "Rotate",
    "Planes",
    "Custom"
};

#endif // CONFIG_H
