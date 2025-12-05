#include "display.h"
#include "config.h"

#include <Arduino_GFX_Library.h>
#include <Wire.h>

// Touch controller state
static bool touchAvailable = false;
static int16_t touchX = 0;
static int16_t touchY = 0;
static bool touchPressed = false;

// Initialize CST816D touch controller
static bool initTouch() {
    Wire.begin(TOUCH_SDA, TOUCH_SCL);
    Wire.beginTransmission(TOUCH_I2C_ADDR);
    if (Wire.endTransmission() == 0) {
        Serial.println("CST816D touch controller found");
        return true;
    }
    Serial.println("CST816D touch controller not found");
    return false;
}

// Read touch data from CST816D
static void readTouch() {
    Wire.beginTransmission(TOUCH_I2C_ADDR);
    Wire.write(0x01);  // Start at register 0x01
    Wire.endTransmission();

    Wire.requestFrom(TOUCH_I2C_ADDR, 6);
    if (Wire.available() >= 6) {
        uint8_t gesture = Wire.read();      // 0x01: gesture
        uint8_t numPoints = Wire.read();    // 0x02: number of touch points
        uint8_t xh = Wire.read();           // 0x03: X high + event flag
        uint8_t xl = Wire.read();           // 0x04: X low
        uint8_t yh = Wire.read();           // 0x05: Y high + touch ID
        uint8_t yl = Wire.read();           // 0x06: Y low

        touchPressed = (numPoints > 0);
        if (touchPressed) {
            touchX = ((xh & 0x0F) << 8) | xl;
            touchY = ((yh & 0x0F) << 8) | yl;
        }
    }
}

// ============================================
// Arduino_GFX Configuration for ST7789
// ============================================

// Create SPI bus for LCD with explicit frequency (40MHz)
Arduino_DataBus *bus = new Arduino_ESP32SPI(
    LCD_DC,     // DC
    LCD_CS,     // CS
    LCD_SCLK,   // SCK
    LCD_MOSI,   // MOSI
    LCD_MISO,   // MISO
    FSPI,       // Use FSPI bus (SPI2) for ESP32-S3
    false       // not shared
);

// Create display driver for ST7789
Arduino_GFX *gfx = new Arduino_ST7789(
    bus,
    LCD_RST,      // RST (-1 means not used)
    LCD_ROTATION, // rotation (0-3)
    true,         // IPS
    LCD_WIDTH,    // width
    LCD_HEIGHT    // height
);

// LVGL buffer allocation
lv_disp_draw_buf_t Display::_drawBuf;
lv_color_t* Display::_buf1 = nullptr;
lv_color_t* Display::_buf2 = nullptr;
lv_disp_drv_t Display::_dispDrv;
lv_indev_drv_t Display::_indevDrv;

// Global display instance
Display display;

Display::Display()
    : _backlightLevel(255)
    , _lastActivityTime(0)
    , _isDimmed(false) {
}

bool Display::begin() {
    Serial.println("Initializing display...");

    // Initialize backlight pin first
    pinMode(LCD_BL, OUTPUT);
    digitalWrite(LCD_BL, HIGH);  // Turn on backlight
    delay(100);  // Allow backlight to stabilize

    // Initialize Arduino_GFX with explicit speed
    if (!gfx->begin(40000000)) {  // 40MHz SPI
        Serial.println("gfx->begin() failed!");
        return false;
    }

    // Fill with black (no inversion needed for this panel)
    gfx->fillScreen(0x0000);  // BLACK
    Serial.println("Display initialized");

    // Re-enable backlight after init
    digitalWrite(LCD_BL, HIGH);

    // Initialize LVGL
    lv_init();

    // Allocate display buffers (double buffering)
    size_t bufSize = LCD_WIDTH * 40;  // 40 lines at a time
    _buf1 = (lv_color_t*)heap_caps_malloc(bufSize * sizeof(lv_color_t), MALLOC_CAP_DMA | MALLOC_CAP_8BIT);
    _buf2 = (lv_color_t*)heap_caps_malloc(bufSize * sizeof(lv_color_t), MALLOC_CAP_DMA | MALLOC_CAP_8BIT);

    if (!_buf1 || !_buf2) {
        Serial.println("Failed to allocate LVGL buffers!");
        // Try single buffer
        if (_buf2) {
            free(_buf2);
            _buf2 = nullptr;
        }
        if (!_buf1) {
            _buf1 = (lv_color_t*)heap_caps_malloc(bufSize * sizeof(lv_color_t), MALLOC_CAP_8BIT);
            if (!_buf1) {
                Serial.println("Critical: Cannot allocate display buffer!");
                return false;
            }
        }
        lv_disp_draw_buf_init(&_drawBuf, _buf1, nullptr, bufSize);
    } else {
        lv_disp_draw_buf_init(&_drawBuf, _buf1, _buf2, bufSize);
    }

    // Initialize display driver
    lv_disp_drv_init(&_dispDrv);
    _dispDrv.hor_res = LCD_WIDTH;
    _dispDrv.ver_res = LCD_HEIGHT;
    _dispDrv.flush_cb = displayFlush;
    _dispDrv.draw_buf = &_drawBuf;
    lv_disp_drv_register(&_dispDrv);

    // Initialize touch controller
    touchAvailable = initTouch();
    if (touchAvailable) {
        lv_indev_drv_init(&_indevDrv);
        _indevDrv.type = LV_INDEV_TYPE_POINTER;
        _indevDrv.read_cb = touchpadRead;
        lv_indev_drv_register(&_indevDrv);
        Serial.println("Touch input registered with LVGL");
    }

    _lastActivityTime = millis();
    Serial.println("LVGL initialized");

    Serial.println("Display ready for LVGL");
    return true;
}

void Display::update() {
    lv_timer_handler();
    checkScreenTimeout();
}

void Display::setBacklight(uint8_t brightness) {
    _backlightLevel = brightness;
    // Use analogWrite for PWM brightness control
    analogWrite(LCD_BL, brightness);
    if (brightness > 0) {
        _isDimmed = false;
    }
}

void Display::resetActivityTimer() {
    _lastActivityTime = millis();
    if (_isDimmed) {
        setBacklight(_backlightLevel > 0 ? _backlightLevel : 255);
        _isDimmed = false;
    }
}

void Display::checkScreenTimeout() {
#if DISPLAY_TIMEOUT > 0
    if (!_isDimmed && (millis() - _lastActivityTime > DISPLAY_TIMEOUT)) {
        analogWrite(LCD_BL, 30);  // Dim to 30
        _isDimmed = true;
    }
#endif
}

uint16_t Display::getWidth() const {
    return LCD_WIDTH;
}

uint16_t Display::getHeight() const {
    return LCD_HEIGHT;
}

void Display::drawDebugMarker(int step) {
    // Draw a small square at position based on step number
    // Each step gets a different colored square along the top
    uint16_t colors[] = {0xF800, 0x07E0, 0x001F, 0xFFE0, 0xF81F, 0x07FF, 0xFFFF};
    int colorIndex = step % 7;
    gfx->fillRect(step * 30, 280, 25, 15, colors[colorIndex]);
}

// LVGL display flush callback
void Display::displayFlush(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t*)color_p, w, h);

    lv_disp_flush_ready(drv);
}

// LVGL touch read callback
void Display::touchpadRead(lv_indev_drv_t* drv, lv_indev_data_t* data) {
    if (touchAvailable) {
        readTouch();
        data->point.x = touchX;
        data->point.y = touchY;
        data->state = touchPressed ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;

        // Reset activity timer on touch
        if (touchPressed) {
            display.resetActivityTimer();
        }
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}
