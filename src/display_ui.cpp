#include "display_ui.h"
#include "config.h"

DisplayUI* displayUI = nullptr;

// Color presets - 9 colors for 3x3 grid
static const uint32_t COLOR_PRESETS[] = {
    0xFF0000,  // Red
    0xFF6600,  // Orange
    0xFFCC00,  // Warm Yellow
    0x00FF00,  // Green
    0x00FFAA,  // Cyan/Teal
    0x0066FF,  // Blue
    0x9900FF,  // Purple
    0xFF0099,  // Pink/Magenta
    0xFFFFFF,  // White
};
static const int NUM_COLOR_PRESETS = 9;

// Pattern names - short versions for tiles
static const char* PATTERN_NAMES[] = {
    "STATIC",
    "RAINBOW",
    "CHASE",
    "TWINKLE",
    "FADE",
    "SPARKLE",
    "CANDY",
    "SNOW",
    "FIRE"
};
static const int NUM_PATTERNS = 9;

// Layout constants
static const int TAB_BAR_HEIGHT = 50;
static const int CONTENT_HEIGHT = LCD_HEIGHT - TAB_BAR_HEIGHT;
static const int TILE_GAP = 3;
static const int TILE_BORDER = 2;

DisplayUI::DisplayUI(LEDController& ledController, Calibration& calibration, WiFiManager& wifiManager)
    : _ledController(ledController)
    , _calibration(calibration)
    , _wifiManager(wifiManager)
    , _activeTab(TAB_CONTROL)
    , _currentCalibLed(0) {
}

void DisplayUI::begin() {
    Serial.println("Creating bento UI...");

    _screen = lv_scr_act();
    lv_obj_set_style_bg_color(_screen, lv_color_black(), 0);

    // Content area (above tab bar)
    _contentArea = lv_obj_create(_screen);
    lv_obj_set_size(_contentArea, LCD_WIDTH, CONTENT_HEIGHT);
    lv_obj_set_pos(_contentArea, 0, 0);
    lv_obj_set_style_bg_color(_contentArea, lv_color_black(), 0);
    lv_obj_set_style_border_width(_contentArea, 0, 0);
    lv_obj_set_style_pad_all(_contentArea, 0, 0);
    lv_obj_set_style_radius(_contentArea, 0, 0);
    lv_obj_clear_flag(_contentArea, LV_OBJ_FLAG_SCROLLABLE);

    createTabBar();
    createControlTab();
    createCalibrateTab();
    createSettingsTab();
    createPatternOverlay();
    createBrightnessOverlay();
    createColorOverlay();

    switchTab(TAB_CONTROL);
    syncState();
    Serial.println("UI created");
}

void DisplayUI::createTabBar() {
    _tabBar = lv_obj_create(_screen);
    lv_obj_set_size(_tabBar, LCD_WIDTH, TAB_BAR_HEIGHT);
    lv_obj_set_pos(_tabBar, 0, CONTENT_HEIGHT);
    lv_obj_set_style_bg_color(_tabBar, lv_color_black(), 0);
    lv_obj_set_style_border_color(_tabBar, lv_color_white(), 0);
    lv_obj_set_style_border_width(_tabBar, TILE_BORDER, 0);
    lv_obj_set_style_border_side(_tabBar, LV_BORDER_SIDE_TOP, 0);
    lv_obj_set_style_pad_all(_tabBar, 0, 0);
    lv_obj_set_style_radius(_tabBar, 0, 0);
    lv_obj_clear_flag(_tabBar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(_tabBar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(_tabBar, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Tab icons: Power/Bulb, Crosshair, Gear
    const char* icons[] = { LV_SYMBOL_HOME, LV_SYMBOL_GPS, LV_SYMBOL_SETTINGS };

    for (int i = 0; i < 3; i++) {
        lv_obj_t* btn = lv_obj_create(_tabBar);
        lv_obj_set_size(btn, LCD_WIDTH / 3 - 4, TAB_BAR_HEIGHT - 8);
        lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(btn, 0, 0);
        lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_user_data(btn, (void*)(intptr_t)i);
        lv_obj_add_event_cb(btn, onTabSelect, LV_EVENT_CLICKED, this);
        lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t* icon = lv_label_create(btn);
        lv_label_set_text(icon, icons[i]);
        lv_obj_set_style_text_color(icon, lv_color_hex(0x666666), 0);
        lv_obj_set_style_text_font(icon, &lv_font_montserrat_20, 0);
        lv_obj_center(icon);

        _tabBtns[i] = btn;
    }
}

void DisplayUI::createControlTab() {
    _controlContainer = lv_obj_create(_contentArea);
    lv_obj_set_size(_controlContainer, LCD_WIDTH, CONTENT_HEIGHT);
    lv_obj_set_pos(_controlContainer, 0, 0);
    lv_obj_set_style_bg_color(_controlContainer, lv_color_black(), 0);
    lv_obj_set_style_border_width(_controlContainer, 0, 0);
    lv_obj_set_style_pad_all(_controlContainer, TILE_GAP, 0);
    lv_obj_set_style_radius(_controlContainer, 0, 0);
    lv_obj_clear_flag(_controlContainer, LV_OBJ_FLAG_SCROLLABLE);

    int tileW = (LCD_WIDTH - TILE_GAP * 3) / 2;
    int tileH = (CONTENT_HEIGHT - TILE_GAP * 3) / 2;

    // Power tile - top left
    _powerTile = lv_obj_create(_controlContainer);
    lv_obj_set_size(_powerTile, tileW, tileH);
    lv_obj_set_pos(_powerTile, 0, 0);
    lv_obj_set_style_bg_color(_powerTile, lv_color_black(), 0);
    lv_obj_set_style_border_color(_powerTile, lv_color_white(), 0);
    lv_obj_set_style_border_width(_powerTile, TILE_BORDER, 0);
    lv_obj_set_style_radius(_powerTile, 0, 0);
    lv_obj_add_flag(_powerTile, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(_powerTile, onPowerTap, LV_EVENT_CLICKED, this);
    lv_obj_clear_flag(_powerTile, LV_OBJ_FLAG_SCROLLABLE);

    _powerIcon = lv_label_create(_powerTile);
    lv_label_set_text(_powerIcon, LV_SYMBOL_POWER);
    lv_obj_set_style_text_color(_powerIcon, lv_color_hex(0x444444), 0);
    lv_obj_set_style_text_font(_powerIcon, &lv_font_montserrat_24, 0);
    lv_obj_center(_powerIcon);

    // Pattern tile - top right
    _patternTile = lv_obj_create(_controlContainer);
    lv_obj_set_size(_patternTile, tileW, tileH);
    lv_obj_set_pos(_patternTile, tileW + TILE_GAP, 0);
    lv_obj_set_style_bg_color(_patternTile, lv_color_black(), 0);
    lv_obj_set_style_border_color(_patternTile, lv_color_white(), 0);
    lv_obj_set_style_border_width(_patternTile, TILE_BORDER, 0);
    lv_obj_set_style_radius(_patternTile, 0, 0);
    lv_obj_add_flag(_patternTile, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(_patternTile, onPatternTap, LV_EVENT_CLICKED, this);
    lv_obj_clear_flag(_patternTile, LV_OBJ_FLAG_SCROLLABLE);

    _patternValue = lv_label_create(_patternTile);
    lv_label_set_text(_patternValue, "STATIC");
    lv_obj_set_style_text_color(_patternValue, lv_color_white(), 0);
    lv_obj_set_style_text_font(_patternValue, &lv_font_montserrat_18, 0);
    lv_obj_center(_patternValue);

    // Brightness tile - bottom left
    _brightnessTile = lv_obj_create(_controlContainer);
    lv_obj_set_size(_brightnessTile, tileW, tileH);
    lv_obj_set_pos(_brightnessTile, 0, tileH + TILE_GAP);
    lv_obj_set_style_bg_color(_brightnessTile, lv_color_black(), 0);
    lv_obj_set_style_border_color(_brightnessTile, lv_color_white(), 0);
    lv_obj_set_style_border_width(_brightnessTile, TILE_BORDER, 0);
    lv_obj_set_style_radius(_brightnessTile, 0, 0);
    lv_obj_add_flag(_brightnessTile, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(_brightnessTile, onBrightnessTap, LV_EVENT_CLICKED, this);
    lv_obj_clear_flag(_brightnessTile, LV_OBJ_FLAG_SCROLLABLE);

    _brightnessValue = lv_label_create(_brightnessTile);
    lv_label_set_text(_brightnessValue, "50%");
    lv_obj_set_style_text_color(_brightnessValue, lv_color_white(), 0);
    lv_obj_set_style_text_font(_brightnessValue, &lv_font_montserrat_24, 0);
    lv_obj_center(_brightnessValue);

    // Color tile - bottom right
    _colorTile = lv_obj_create(_controlContainer);
    lv_obj_set_size(_colorTile, tileW, tileH);
    lv_obj_set_pos(_colorTile, tileW + TILE_GAP, tileH + TILE_GAP);
    lv_obj_set_style_bg_color(_colorTile, lv_color_black(), 0);
    lv_obj_set_style_border_color(_colorTile, lv_color_white(), 0);
    lv_obj_set_style_border_width(_colorTile, TILE_BORDER, 0);
    lv_obj_set_style_radius(_colorTile, 0, 0);
    lv_obj_add_flag(_colorTile, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(_colorTile, onColorTap, LV_EVENT_CLICKED, this);
    lv_obj_clear_flag(_colorTile, LV_OBJ_FLAG_SCROLLABLE);

    _colorSwatch = lv_obj_create(_colorTile);
    lv_obj_set_size(_colorSwatch, 50, 50);
    lv_obj_set_style_bg_color(_colorSwatch, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_border_color(_colorSwatch, lv_color_white(), 0);
    lv_obj_set_style_border_width(_colorSwatch, 1, 0);
    lv_obj_set_style_radius(_colorSwatch, 0, 0);
    lv_obj_center(_colorSwatch);
    lv_obj_clear_flag(_colorSwatch, LV_OBJ_FLAG_SCROLLABLE);
}

void DisplayUI::createCalibrateTab() {
    _calibContainer = lv_obj_create(_contentArea);
    lv_obj_set_size(_calibContainer, LCD_WIDTH, CONTENT_HEIGHT);
    lv_obj_set_pos(_calibContainer, 0, 0);
    lv_obj_set_style_bg_color(_calibContainer, lv_color_black(), 0);
    lv_obj_set_style_border_width(_calibContainer, 0, 0);
    lv_obj_set_style_pad_all(_calibContainer, TILE_GAP, 0);
    lv_obj_set_style_radius(_calibContainer, 0, 0);
    lv_obj_add_flag(_calibContainer, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(_calibContainer, LV_OBJ_FLAG_SCROLLABLE);

    // LED number display - large centered
    lv_obj_t* ledTile = lv_obj_create(_calibContainer);
    lv_obj_set_size(ledTile, LCD_WIDTH - TILE_GAP * 2, 70);
    lv_obj_set_pos(ledTile, 0, 0);
    lv_obj_set_style_bg_color(ledTile, lv_color_black(), 0);
    lv_obj_set_style_border_color(ledTile, lv_color_white(), 0);
    lv_obj_set_style_border_width(ledTile, TILE_BORDER, 0);
    lv_obj_set_style_radius(ledTile, 0, 0);
    lv_obj_clear_flag(ledTile, LV_OBJ_FLAG_SCROLLABLE);

    _calibLedValue = lv_label_create(ledTile);
    lv_label_set_text(_calibLedValue, "1/50");
    lv_obj_set_style_text_color(_calibLedValue, lv_color_white(), 0);
    lv_obj_set_style_text_font(_calibLedValue, &lv_font_montserrat_24, 0);
    lv_obj_center(_calibLedValue);

    // Nav buttons row
    int navY = 70 + TILE_GAP;
    int btnW = (LCD_WIDTH - TILE_GAP * 3) / 2;

    _calibPrevBtn = lv_obj_create(_calibContainer);
    lv_obj_set_size(_calibPrevBtn, btnW, 50);
    lv_obj_set_pos(_calibPrevBtn, 0, navY);
    lv_obj_set_style_bg_color(_calibPrevBtn, lv_color_black(), 0);
    lv_obj_set_style_border_color(_calibPrevBtn, lv_color_white(), 0);
    lv_obj_set_style_border_width(_calibPrevBtn, TILE_BORDER, 0);
    lv_obj_set_style_radius(_calibPrevBtn, 0, 0);
    lv_obj_add_flag(_calibPrevBtn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(_calibPrevBtn, onCalibPrev, LV_EVENT_CLICKED, this);
    lv_obj_clear_flag(_calibPrevBtn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t* prevLbl = lv_label_create(_calibPrevBtn);
    lv_label_set_text(prevLbl, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(prevLbl, lv_color_white(), 0);
    lv_obj_set_style_text_font(prevLbl, &lv_font_montserrat_20, 0);
    lv_obj_center(prevLbl);

    _calibNextBtn = lv_obj_create(_calibContainer);
    lv_obj_set_size(_calibNextBtn, btnW, 50);
    lv_obj_set_pos(_calibNextBtn, btnW + TILE_GAP, navY);
    lv_obj_set_style_bg_color(_calibNextBtn, lv_color_black(), 0);
    lv_obj_set_style_border_color(_calibNextBtn, lv_color_white(), 0);
    lv_obj_set_style_border_width(_calibNextBtn, TILE_BORDER, 0);
    lv_obj_set_style_radius(_calibNextBtn, 0, 0);
    lv_obj_add_flag(_calibNextBtn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(_calibNextBtn, onCalibNext, LV_EVENT_CLICKED, this);
    lv_obj_clear_flag(_calibNextBtn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t* nextLbl = lv_label_create(_calibNextBtn);
    lv_label_set_text(nextLbl, LV_SYMBOL_RIGHT);
    lv_obj_set_style_text_color(nextLbl, lv_color_white(), 0);
    lv_obj_set_style_text_font(nextLbl, &lv_font_montserrat_20, 0);
    lv_obj_center(nextLbl);

    // XYZ sliders
    int sliderY = navY + 50 + TILE_GAP;
    int sliderH = 35;

    auto createSlider = [this, sliderH](lv_obj_t* parent, int y, const char* label) -> lv_obj_t* {
        lv_obj_t* row = lv_obj_create(parent);
        lv_obj_set_size(row, LCD_WIDTH - TILE_GAP * 2, sliderH);
        lv_obj_set_pos(row, 0, y);
        lv_obj_set_style_bg_color(row, lv_color_black(), 0);
        lv_obj_set_style_border_color(row, lv_color_white(), 0);
        lv_obj_set_style_border_width(row, TILE_BORDER, 0);
        lv_obj_set_style_radius(row, 0, 0);
        lv_obj_set_style_pad_left(row, 8, 0);
        lv_obj_set_style_pad_right(row, 8, 0);
        lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t* lbl = lv_label_create(row);
        lv_label_set_text(lbl, label);
        lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_16, 0);
        lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 0, 0);

        lv_obj_t* slider = lv_slider_create(row);
        lv_slider_set_range(slider, -100, 100);
        lv_slider_set_value(slider, 0, LV_ANIM_OFF);
        lv_obj_set_size(slider, 160, 6);
        lv_obj_align(slider, LV_ALIGN_RIGHT_MID, 0, 0);
        lv_obj_set_style_bg_color(slider, lv_color_hex(0x333333), LV_PART_MAIN);
        lv_obj_set_style_bg_color(slider, lv_color_white(), LV_PART_INDICATOR);
        lv_obj_set_style_bg_color(slider, lv_color_white(), LV_PART_KNOB);
        lv_obj_set_style_pad_all(slider, 8, LV_PART_KNOB);
        lv_obj_add_event_cb(slider, onCalibPosChange, LV_EVENT_VALUE_CHANGED, this);

        return slider;
    };

    _calibXSlider = createSlider(_calibContainer, sliderY, "X");
    _calibYSlider = createSlider(_calibContainer, sliderY + sliderH + TILE_GAP, "Y");
    _calibZSlider = createSlider(_calibContainer, sliderY + (sliderH + TILE_GAP) * 2, "Z");

    // Save button
    int saveY = sliderY + (sliderH + TILE_GAP) * 3;
    _calibSaveBtn = lv_obj_create(_calibContainer);
    lv_obj_set_size(_calibSaveBtn, LCD_WIDTH - TILE_GAP * 2, 45);
    lv_obj_set_pos(_calibSaveBtn, 0, saveY);
    lv_obj_set_style_bg_color(_calibSaveBtn, lv_color_white(), 0);
    lv_obj_set_style_border_width(_calibSaveBtn, 0, 0);
    lv_obj_set_style_radius(_calibSaveBtn, 0, 0);
    lv_obj_add_flag(_calibSaveBtn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(_calibSaveBtn, onCalibSave, LV_EVENT_CLICKED, this);
    lv_obj_clear_flag(_calibSaveBtn, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* saveLbl = lv_label_create(_calibSaveBtn);
    lv_label_set_text(saveLbl, "SAVE");
    lv_obj_set_style_text_color(saveLbl, lv_color_black(), 0);
    lv_obj_set_style_text_font(saveLbl, &lv_font_montserrat_16, 0);
    lv_obj_center(saveLbl);
}

void DisplayUI::createSettingsTab() {
    _settingsContainer = lv_obj_create(_contentArea);
    lv_obj_set_size(_settingsContainer, LCD_WIDTH, CONTENT_HEIGHT);
    lv_obj_set_pos(_settingsContainer, 0, 0);
    lv_obj_set_style_bg_color(_settingsContainer, lv_color_black(), 0);
    lv_obj_set_style_border_width(_settingsContainer, 0, 0);
    lv_obj_set_style_pad_all(_settingsContainer, TILE_GAP, 0);
    lv_obj_set_style_radius(_settingsContainer, 0, 0);
    lv_obj_add_flag(_settingsContainer, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(_settingsContainer, LV_OBJ_FLAG_SCROLLABLE);

    // WiFi status tile
    lv_obj_t* wifiTile = lv_obj_create(_settingsContainer);
    lv_obj_set_size(wifiTile, LCD_WIDTH - TILE_GAP * 2, 100);
    lv_obj_set_pos(wifiTile, 0, 0);
    lv_obj_set_style_bg_color(wifiTile, lv_color_black(), 0);
    lv_obj_set_style_border_color(wifiTile, lv_color_white(), 0);
    lv_obj_set_style_border_width(wifiTile, TILE_BORDER, 0);
    lv_obj_set_style_radius(wifiTile, 0, 0);
    lv_obj_clear_flag(wifiTile, LV_OBJ_FLAG_SCROLLABLE);

    _wifiIcon = lv_label_create(wifiTile);
    lv_label_set_text(_wifiIcon, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_color(_wifiIcon, lv_color_white(), 0);
    lv_obj_set_style_text_font(_wifiIcon, &lv_font_montserrat_24, 0);
    lv_obj_align(_wifiIcon, LV_ALIGN_TOP_MID, 0, 15);

    _ipValue = lv_label_create(wifiTile);
    lv_label_set_text(_ipValue, "---");
    lv_obj_set_style_text_color(_ipValue, lv_color_white(), 0);
    lv_obj_set_style_text_font(_ipValue, &lv_font_montserrat_18, 0);
    lv_obj_align(_ipValue, LV_ALIGN_BOTTOM_MID, 0, -15);

    // Device info tile
    lv_obj_t* deviceTile = lv_obj_create(_settingsContainer);
    lv_obj_set_size(deviceTile, LCD_WIDTH - TILE_GAP * 2, 80);
    lv_obj_set_pos(deviceTile, 0, 100 + TILE_GAP);
    lv_obj_set_style_bg_color(deviceTile, lv_color_black(), 0);
    lv_obj_set_style_border_color(deviceTile, lv_color_white(), 0);
    lv_obj_set_style_border_width(deviceTile, TILE_BORDER, 0);
    lv_obj_set_style_radius(deviceTile, 0, 0);
    lv_obj_clear_flag(deviceTile, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* deviceInfo = lv_label_create(deviceTile);
    lv_label_set_text(deviceInfo, "50 LEDs\nESP32-S3");
    lv_obj_set_style_text_color(deviceInfo, lv_color_white(), 0);
    lv_obj_set_style_text_font(deviceInfo, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_align(deviceInfo, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(deviceInfo);

    updateWifiStatus();
}

void DisplayUI::createPatternOverlay() {
    _patternOverlay = lv_obj_create(_screen);
    lv_obj_set_size(_patternOverlay, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_set_pos(_patternOverlay, 0, 0);
    lv_obj_set_style_bg_color(_patternOverlay, lv_color_black(), 0);
    lv_obj_set_style_border_width(_patternOverlay, 0, 0);
    lv_obj_set_style_pad_all(_patternOverlay, TILE_GAP, 0);
    lv_obj_set_style_radius(_patternOverlay, 0, 0);
    lv_obj_add_flag(_patternOverlay, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(_patternOverlay, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(_patternOverlay, onOverlayClose, LV_EVENT_CLICKED, this);

    // Pattern list
    _patternList = lv_obj_create(_patternOverlay);
    lv_obj_set_size(_patternList, LCD_WIDTH - TILE_GAP * 2, LCD_HEIGHT - TILE_GAP * 2);
    lv_obj_set_pos(_patternList, 0, 0);
    lv_obj_set_style_bg_color(_patternList, lv_color_black(), 0);
    lv_obj_set_style_border_color(_patternList, lv_color_white(), 0);
    lv_obj_set_style_border_width(_patternList, TILE_BORDER, 0);
    lv_obj_set_style_radius(_patternList, 0, 0);
    lv_obj_set_style_pad_all(_patternList, 0, 0);
    lv_obj_set_flex_flow(_patternList, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scroll_dir(_patternList, LV_DIR_VER);

    int itemH = (LCD_HEIGHT - TILE_GAP * 2) / NUM_PATTERNS;

    for (int i = 0; i < NUM_PATTERNS; i++) {
        lv_obj_t* item = lv_obj_create(_patternList);
        lv_obj_set_size(item, LCD_WIDTH - TILE_GAP * 2 - TILE_BORDER * 2, itemH);
        lv_obj_set_style_bg_color(item, lv_color_black(), 0);
        lv_obj_set_style_border_color(item, lv_color_hex(0x333333), 0);
        lv_obj_set_style_border_width(item, 1, 0);
        lv_obj_set_style_border_side(item, LV_BORDER_SIDE_BOTTOM, 0);
        lv_obj_set_style_radius(item, 0, 0);
        lv_obj_add_flag(item, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_user_data(item, (void*)(intptr_t)i);
        lv_obj_add_event_cb(item, onPatternSelect, LV_EVENT_CLICKED, this);
        lv_obj_clear_flag(item, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t* label = lv_label_create(item);
        lv_label_set_text(label, PATTERN_NAMES[i]);
        lv_obj_set_style_text_color(label, lv_color_white(), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
        lv_obj_center(label);
    }
}

void DisplayUI::createBrightnessOverlay() {
    _brightnessOverlay = lv_obj_create(_screen);
    lv_obj_set_size(_brightnessOverlay, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_set_pos(_brightnessOverlay, 0, 0);
    lv_obj_set_style_bg_color(_brightnessOverlay, lv_color_black(), 0);
    lv_obj_set_style_border_width(_brightnessOverlay, 0, 0);
    lv_obj_set_style_radius(_brightnessOverlay, 0, 0);
    lv_obj_add_flag(_brightnessOverlay, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(_brightnessOverlay, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(_brightnessOverlay, onOverlayClose, LV_EVENT_CLICKED, this);
    lv_obj_clear_flag(_brightnessOverlay, LV_OBJ_FLAG_SCROLLABLE);

    // Center tile
    lv_obj_t* tile = lv_obj_create(_brightnessOverlay);
    lv_obj_set_size(tile, LCD_WIDTH - TILE_GAP * 2, 180);
    lv_obj_align(tile, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(tile, lv_color_black(), 0);
    lv_obj_set_style_border_color(tile, lv_color_white(), 0);
    lv_obj_set_style_border_width(tile, TILE_BORDER, 0);
    lv_obj_set_style_radius(tile, 0, 0);
    lv_obj_clear_flag(tile, LV_OBJ_FLAG_SCROLLABLE);

    _brightnessDisplay = lv_label_create(tile);
    lv_label_set_text(_brightnessDisplay, "50%");
    lv_obj_set_style_text_color(_brightnessDisplay, lv_color_white(), 0);
    lv_obj_set_style_text_font(_brightnessDisplay, &lv_font_montserrat_24, 0);
    lv_obj_align(_brightnessDisplay, LV_ALIGN_TOP_MID, 0, 30);

    _brightnessSlider = lv_slider_create(tile);
    lv_obj_set_size(_brightnessSlider, LCD_WIDTH - 80, 8);
    lv_slider_set_range(_brightnessSlider, 0, 255);
    lv_obj_align(_brightnessSlider, LV_ALIGN_BOTTOM_MID, 0, -40);
    lv_obj_set_style_bg_color(_brightnessSlider, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_set_style_bg_color(_brightnessSlider, lv_color_white(), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(_brightnessSlider, lv_color_white(), LV_PART_KNOB);
    lv_obj_set_style_pad_all(_brightnessSlider, 12, LV_PART_KNOB);
    lv_obj_add_event_cb(_brightnessSlider, onBrightnessChange, LV_EVENT_VALUE_CHANGED, this);
}

void DisplayUI::createColorOverlay() {
    _colorOverlay = lv_obj_create(_screen);
    lv_obj_set_size(_colorOverlay, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_set_pos(_colorOverlay, 0, 0);
    lv_obj_set_style_bg_color(_colorOverlay, lv_color_black(), 0);
    lv_obj_set_style_border_width(_colorOverlay, 0, 0);
    lv_obj_set_style_radius(_colorOverlay, 0, 0);
    lv_obj_add_flag(_colorOverlay, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(_colorOverlay, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(_colorOverlay, onOverlayClose, LV_EVENT_CLICKED, this);
    lv_obj_clear_flag(_colorOverlay, LV_OBJ_FLAG_SCROLLABLE);

    // Grid of colors - 3x3
    int swatchW = (LCD_WIDTH - TILE_GAP * 4) / 3;
    int swatchH = (LCD_HEIGHT - TILE_GAP * 4) / 3;

    for (int i = 0; i < NUM_COLOR_PRESETS; i++) {
        int col = i % 3;
        int row = i / 3;

        lv_obj_t* swatch = lv_obj_create(_colorOverlay);
        lv_obj_set_size(swatch, swatchW, swatchH);
        lv_obj_set_pos(swatch, TILE_GAP + col * (swatchW + TILE_GAP), TILE_GAP + row * (swatchH + TILE_GAP));
        lv_obj_set_style_bg_color(swatch, lv_color_hex(COLOR_PRESETS[i]), 0);
        lv_obj_set_style_border_color(swatch, lv_color_white(), 0);
        lv_obj_set_style_border_width(swatch, TILE_BORDER, 0);
        lv_obj_set_style_radius(swatch, 0, 0);
        lv_obj_add_flag(swatch, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_user_data(swatch, (void*)(intptr_t)i);
        lv_obj_add_event_cb(swatch, onColorSelect, LV_EVENT_CLICKED, this);
        lv_obj_clear_flag(swatch, LV_OBJ_FLAG_SCROLLABLE);

        _colorBtns[i] = swatch;
    }
}

void DisplayUI::switchTab(int tab) {
    _activeTab = tab;

    // Hide all containers
    lv_obj_add_flag(_controlContainer, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(_calibContainer, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(_settingsContainer, LV_OBJ_FLAG_HIDDEN);

    // Show selected
    switch (tab) {
        case TAB_CONTROL:
            lv_obj_clear_flag(_controlContainer, LV_OBJ_FLAG_HIDDEN);
            break;
        case TAB_CALIBRATE:
            lv_obj_clear_flag(_calibContainer, LV_OBJ_FLAG_HIDDEN);
            _calibration.setCalibrationLED(_currentCalibLed);
            updateCalibrationUI();
            break;
        case TAB_SETTINGS:
            lv_obj_clear_flag(_settingsContainer, LV_OBJ_FLAG_HIDDEN);
            updateWifiStatus();
            break;
    }

    // Update tab button colors
    for (int i = 0; i < 3; i++) {
        lv_obj_t* icon = lv_obj_get_child(_tabBtns[i], 0);
        if (i == tab) {
            lv_obj_set_style_text_color(icon, lv_color_white(), 0);
        } else {
            lv_obj_set_style_text_color(icon, lv_color_hex(0x666666), 0);
        }
    }
}

void DisplayUI::showOverlay(lv_obj_t* overlay) {
    lv_obj_clear_flag(overlay, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(overlay);
}

void DisplayUI::hideOverlay(lv_obj_t* overlay) {
    lv_obj_add_flag(overlay, LV_OBJ_FLAG_HIDDEN);
}

void DisplayUI::update() {
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 2000) {
        if (_activeTab == TAB_SETTINGS) {
            updateWifiStatus();
        }
        lastUpdate = millis();
    }
}

void DisplayUI::syncState() {
    updateControlTiles();
}

void DisplayUI::updateControlTiles() {
    // Power
    if (_ledController.isOn()) {
        lv_obj_set_style_text_color(_powerIcon, lv_color_white(), 0);
        lv_obj_set_style_bg_color(_powerTile, lv_color_black(), 0);
    } else {
        lv_obj_set_style_text_color(_powerIcon, lv_color_hex(0x444444), 0);
        lv_obj_set_style_bg_color(_powerTile, lv_color_black(), 0);
    }

    // Pattern
    int anim = (int)_ledController.getAnimation();
    if (anim >= 0 && anim < NUM_PATTERNS) {
        lv_label_set_text(_patternValue, PATTERN_NAMES[anim]);
    }

    // Brightness
    char buf[8];
    snprintf(buf, sizeof(buf), "%d%%", _ledController.getBrightness() * 100 / 255);
    lv_label_set_text(_brightnessValue, buf);

    // Color
    CRGB color = _ledController.getSolidColor();
    lv_obj_set_style_bg_color(_colorSwatch, lv_color_make(color.r, color.g, color.b), 0);
}

void DisplayUI::updateCalibrationUI() {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d/50", _currentCalibLed + 1);
    lv_label_set_text(_calibLedValue, buf);

    LEDPosition pos = _calibration.getPosition(_currentCalibLed);
    lv_slider_set_value(_calibXSlider, (int)(pos.x * 100), LV_ANIM_OFF);
    lv_slider_set_value(_calibYSlider, (int)(pos.y * 100), LV_ANIM_OFF);
    lv_slider_set_value(_calibZSlider, (int)(pos.z * 100), LV_ANIM_OFF);
}

void DisplayUI::updateWifiStatus() {
    WiFiState state = _wifiManager.getState();

    if (state == WIFI_STATE_CONNECTED || state == WIFI_STATE_AP_MODE) {
        lv_obj_set_style_text_color(_wifiIcon, lv_color_white(), 0);
    } else {
        lv_obj_set_style_text_color(_wifiIcon, lv_color_hex(0x444444), 0);
    }

    lv_label_set_text(_ipValue, _wifiManager.getIPAddress().c_str());
}

// ============================================
// Event Handlers
// ============================================

void DisplayUI::onTabSelect(lv_event_t* e) {
    DisplayUI* ui = (DisplayUI*)lv_event_get_user_data(e);
    lv_obj_t* btn = lv_event_get_target(e);
    int tab = (int)(intptr_t)lv_obj_get_user_data(btn);
    ui->switchTab(tab);
}

void DisplayUI::onPowerTap(lv_event_t* e) {
    DisplayUI* ui = (DisplayUI*)lv_event_get_user_data(e);
    ui->_ledController.setOn(!ui->_ledController.isOn());
    ui->updateControlTiles();
}

void DisplayUI::onPatternTap(lv_event_t* e) {
    DisplayUI* ui = (DisplayUI*)lv_event_get_user_data(e);
    ui->showOverlay(ui->_patternOverlay);
}

void DisplayUI::onBrightnessTap(lv_event_t* e) {
    DisplayUI* ui = (DisplayUI*)lv_event_get_user_data(e);
    lv_slider_set_value(ui->_brightnessSlider, ui->_ledController.getBrightness(), LV_ANIM_OFF);
    char buf[8];
    snprintf(buf, sizeof(buf), "%d%%", ui->_ledController.getBrightness() * 100 / 255);
    lv_label_set_text(ui->_brightnessDisplay, buf);
    ui->showOverlay(ui->_brightnessOverlay);
}

void DisplayUI::onColorTap(lv_event_t* e) {
    DisplayUI* ui = (DisplayUI*)lv_event_get_user_data(e);
    ui->showOverlay(ui->_colorOverlay);
}

void DisplayUI::onOverlayClose(lv_event_t* e) {
    DisplayUI* ui = (DisplayUI*)lv_event_get_user_data(e);
    lv_obj_t* overlay = lv_event_get_target(e);

    // Only close if clicking the overlay background, not children
    if (overlay == ui->_patternOverlay || overlay == ui->_brightnessOverlay || overlay == ui->_colorOverlay) {
        ui->hideOverlay(overlay);
        ui->updateControlTiles();
    }
}

void DisplayUI::onPatternSelect(lv_event_t* e) {
    DisplayUI* ui = (DisplayUI*)lv_event_get_user_data(e);
    lv_obj_t* item = lv_event_get_target(e);
    int index = (int)(intptr_t)lv_obj_get_user_data(item);

    ui->_ledController.setAnimation(static_cast<AnimationMode>(index));
    ui->hideOverlay(ui->_patternOverlay);
    ui->updateControlTiles();
}

void DisplayUI::onBrightnessChange(lv_event_t* e) {
    DisplayUI* ui = (DisplayUI*)lv_event_get_user_data(e);
    int val = lv_slider_get_value(ui->_brightnessSlider);

    ui->_ledController.setBrightness(val);

    char buf[8];
    snprintf(buf, sizeof(buf), "%d%%", val * 100 / 255);
    lv_label_set_text(ui->_brightnessDisplay, buf);
}

void DisplayUI::onColorSelect(lv_event_t* e) {
    DisplayUI* ui = (DisplayUI*)lv_event_get_user_data(e);
    lv_obj_t* swatch = lv_event_get_target(e);
    int index = (int)(intptr_t)lv_obj_get_user_data(swatch);

    if (index >= 0 && index < NUM_COLOR_PRESETS) {
        uint32_t hex = COLOR_PRESETS[index];
        CRGB color((hex >> 16) & 0xFF, (hex >> 8) & 0xFF, hex & 0xFF);
        ui->_ledController.setSolidColor(color);
        ui->_ledController.setAnimation(ANIMATION_STATIC);
    }

    ui->hideOverlay(ui->_colorOverlay);
    ui->updateControlTiles();
}

void DisplayUI::onCalibPrev(lv_event_t* e) {
    DisplayUI* ui = (DisplayUI*)lv_event_get_user_data(e);
    if (ui->_currentCalibLed > 0) {
        ui->_currentCalibLed--;
        ui->_calibration.setCalibrationLED(ui->_currentCalibLed);
        ui->updateCalibrationUI();
    }
}

void DisplayUI::onCalibNext(lv_event_t* e) {
    DisplayUI* ui = (DisplayUI*)lv_event_get_user_data(e);
    if (ui->_currentCalibLed < NUM_LEDS - 1) {
        ui->_currentCalibLed++;
        ui->_calibration.setCalibrationLED(ui->_currentCalibLed);
        ui->updateCalibrationUI();
    }
}

void DisplayUI::onCalibPosChange(lv_event_t* e) {
    DisplayUI* ui = (DisplayUI*)lv_event_get_user_data(e);

    float x = lv_slider_get_value(ui->_calibXSlider) / 100.0f;
    float y = lv_slider_get_value(ui->_calibYSlider) / 100.0f;
    float z = lv_slider_get_value(ui->_calibZSlider) / 100.0f;

    ui->_calibration.setPosition(ui->_currentCalibLed, x, y, z);
}

void DisplayUI::onCalibSave(lv_event_t* e) {
    DisplayUI* ui = (DisplayUI*)lv_event_get_user_data(e);

    if (ui->_calibration.save()) {
        lv_obj_t* msgbox = lv_msgbox_create(NULL, NULL, "SAVED", NULL, true);
        lv_obj_set_style_bg_color(msgbox, lv_color_black(), 0);
        lv_obj_set_style_text_color(msgbox, lv_color_white(), 0);
        lv_obj_set_style_border_color(msgbox, lv_color_white(), 0);
        lv_obj_set_style_border_width(msgbox, TILE_BORDER, 0);
        lv_obj_center(msgbox);
    }
}
