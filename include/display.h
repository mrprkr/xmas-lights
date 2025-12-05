#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <lvgl.h>

class Display {
public:
    Display();
    bool begin();
    void update();

    // Backlight control
    void setBacklight(uint8_t brightness);  // 0-255
    uint8_t getBacklight() const { return _backlightLevel; }

    // Activity tracking for screen timeout
    void resetActivityTimer();
    bool isDisplayDimmed() const { return _isDimmed; }

    // Get display dimensions
    uint16_t getWidth() const;
    uint16_t getHeight() const;

    // Debug: draw a colored marker at position
    void drawDebugMarker(int step);

private:
    uint8_t _backlightLevel;
    unsigned long _lastActivityTime;
    bool _isDimmed;

    // LVGL display buffer
    static lv_disp_draw_buf_t _drawBuf;
    static lv_color_t* _buf1;
    static lv_color_t* _buf2;
    static lv_disp_drv_t _dispDrv;
    static lv_indev_drv_t _indevDrv;

    // Callbacks for LVGL
    static void displayFlush(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* color_p);
    static void touchpadRead(lv_indev_drv_t* drv, lv_indev_data_t* data);

    void checkScreenTimeout();
};

// Global display instance
extern Display display;

#endif // DISPLAY_H
