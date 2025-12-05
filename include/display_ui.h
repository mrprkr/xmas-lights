#ifndef DISPLAY_UI_H
#define DISPLAY_UI_H

#include <Arduino.h>
#include <lvgl.h>
#include "led_controller.h"
#include "calibration.h"
#include "wifi_manager.h"

class DisplayUI {
public:
    DisplayUI(LEDController& ledController, Calibration& calibration, WiFiManager& wifiManager);
    void begin();
    void update();

    // Sync UI with current state
    void syncState();

private:
    LEDController& _ledController;
    Calibration& _calibration;
    WiFiManager& _wifiManager;

    // Screen objects
    lv_obj_t* _tabview;
    lv_obj_t* _tabControl;
    lv_obj_t* _tabCalibrate;
    lv_obj_t* _tabSettings;

    // Control tab widgets
    lv_obj_t* _powerSwitch;
    lv_obj_t* _brightnessSlider;
    lv_obj_t* _brightnessLabel;
    lv_obj_t* _colorWheel;
    lv_obj_t* _animBtnMatrix;
    lv_obj_t* _speedSlider;
    lv_obj_t* _speedLabel;

    // Calibration tab widgets
    lv_obj_t* _calibLedLabel;
    lv_obj_t* _calibPrevBtn;
    lv_obj_t* _calibNextBtn;
    lv_obj_t* _calibStartBtn;
    lv_obj_t* _calibXSlider;
    lv_obj_t* _calibYSlider;
    lv_obj_t* _calibZSlider;
    lv_obj_t* _calibPosLabel;
    lv_obj_t* _calibSaveBtn;
    lv_obj_t* _calibExitBtn;

    // Settings tab widgets
    lv_obj_t* _wifiStatusLabel;
    lv_obj_t* _ipLabel;

    // Current calibration LED
    int16_t _currentCalibLed;

    // Build UI
    void createControlTab();
    void createCalibrateTab();
    void createSettingsTab();
    void createStyles();

    // Event handlers (static for LVGL callbacks)
    static void onPowerSwitch(lv_event_t* e);
    static void onBrightnessChange(lv_event_t* e);
    static void onColorChange(lv_event_t* e);
    static void onAnimationSelect(lv_event_t* e);
    static void onSpeedChange(lv_event_t* e);
    static void onCalibPrev(lv_event_t* e);
    static void onCalibNext(lv_event_t* e);
    static void onCalibStart(lv_event_t* e);
    static void onCalibExit(lv_event_t* e);
    static void onCalibSave(lv_event_t* e);
    static void onCalibReset(lv_event_t* e);
    static void onCalibPosChange(lv_event_t* e);
    static void onTabChange(lv_event_t* e);

    // Helper methods
    void updateCalibrationUI();
    void updateWifiStatus();
    lv_color_t crgbToLvColor(CRGB color);
    CRGB lvColorToCrgb(lv_color_t color);
};

// Global UI instance
extern DisplayUI* displayUI;

#endif // DISPLAY_UI_H
