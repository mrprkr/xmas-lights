#ifndef DISPLAY_UI_H
#define DISPLAY_UI_H

#include <Arduino.h>
#include <lvgl.h>
#include "led_controller.h"
#include "calibration.h"
#include "wifi_manager.h"

// Tab indices
enum TabIndex {
    TAB_CONTROL = 0,
    TAB_CALIBRATE = 1,
    TAB_SETTINGS = 2
};

class DisplayUI {
public:
    DisplayUI(LEDController& ledController, Calibration& calibration, WiFiManager& wifiManager);
    void begin();
    void update();
    void syncState();

private:
    LEDController& _ledController;
    Calibration& _calibration;
    WiFiManager& _wifiManager;

    // Main containers
    lv_obj_t* _screen;
    lv_obj_t* _tabBar;
    lv_obj_t* _contentArea;
    lv_obj_t* _tabBtns[3];
    int _activeTab;

    // Control tab - bento grid
    lv_obj_t* _controlContainer;
    lv_obj_t* _powerTile;
    lv_obj_t* _powerIcon;
    lv_obj_t* _patternTile;
    lv_obj_t* _patternValue;
    lv_obj_t* _brightnessTile;
    lv_obj_t* _brightnessValue;
    lv_obj_t* _colorTile;
    lv_obj_t* _colorSwatch;

    // Pattern selector overlay
    lv_obj_t* _patternOverlay;
    lv_obj_t* _patternList;

    // Brightness overlay
    lv_obj_t* _brightnessOverlay;
    lv_obj_t* _brightnessSlider;
    lv_obj_t* _brightnessDisplay;

    // Color overlay
    lv_obj_t* _colorOverlay;
    lv_obj_t* _colorBtns[9];

    // Calibrate tab
    lv_obj_t* _calibContainer;
    lv_obj_t* _calibLedValue;
    lv_obj_t* _calibPrevBtn;
    lv_obj_t* _calibNextBtn;
    lv_obj_t* _calibXSlider;
    lv_obj_t* _calibYSlider;
    lv_obj_t* _calibZSlider;
    lv_obj_t* _calibSaveBtn;
    int16_t _currentCalibLed;

    // Settings tab
    lv_obj_t* _settingsContainer;
    lv_obj_t* _wifiIcon;
    lv_obj_t* _ipValue;

    // Build UI
    void createTabBar();
    void createControlTab();
    void createCalibrateTab();
    void createSettingsTab();
    void createPatternOverlay();
    void createBrightnessOverlay();
    void createColorOverlay();
    void switchTab(int tab);
    void showOverlay(lv_obj_t* overlay);
    void hideOverlay(lv_obj_t* overlay);

    // Event handlers
    static void onTabSelect(lv_event_t* e);
    static void onPowerTap(lv_event_t* e);
    static void onPatternTap(lv_event_t* e);
    static void onBrightnessTap(lv_event_t* e);
    static void onColorTap(lv_event_t* e);
    static void onOverlayClose(lv_event_t* e);
    static void onPatternSelect(lv_event_t* e);
    static void onBrightnessChange(lv_event_t* e);
    static void onColorSelect(lv_event_t* e);
    static void onCalibPrev(lv_event_t* e);
    static void onCalibNext(lv_event_t* e);
    static void onCalibPosChange(lv_event_t* e);
    static void onCalibSave(lv_event_t* e);

    // Helpers
    void updateControlTiles();
    void updateCalibrationUI();
    void updateWifiStatus();
};

extern DisplayUI* displayUI;

#endif
