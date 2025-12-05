#include "display_ui.h"
#include "config.h"

// Global UI instance
DisplayUI* displayUI = nullptr;

// Animation button map labels
static const char* ANIM_BASIC_MAP[] = {
    "Static", "Rainbow", "Chase", "\n",
    "Twinkle", "Fade", "Sparkle", "\n",
    "Candy", "Snow", "Fire", ""
};

DisplayUI::DisplayUI(LEDController& ledController, Calibration& calibration, WiFiManager& wifiManager)
    : _ledController(ledController)
    , _calibration(calibration)
    , _wifiManager(wifiManager)
    , _currentCalibLed(-1) {
}

void DisplayUI::begin() {
    Serial.println("Creating UI...");

    createStyles();

    // Create tab view
    _tabview = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 35);
    lv_obj_set_style_bg_color(_tabview, lv_color_hex(0x1a1a2e), 0);

    // Add tabs
    _tabControl = lv_tabview_add_tab(_tabview, LV_SYMBOL_HOME);
    _tabCalibrate = lv_tabview_add_tab(_tabview, LV_SYMBOL_SETTINGS);
    _tabSettings = lv_tabview_add_tab(_tabview, LV_SYMBOL_WIFI);

    // Style tab buttons
    lv_obj_t* tabBtns = lv_tabview_get_tab_btns(_tabview);
    lv_obj_set_style_bg_color(tabBtns, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_text_color(tabBtns, lv_color_white(), 0);
    lv_obj_set_style_border_side(tabBtns, LV_BORDER_SIDE_BOTTOM, (lv_style_selector_t)(LV_PART_ITEMS | LV_STATE_CHECKED));
    lv_obj_set_style_border_color(tabBtns, lv_color_hex(0x00b894), (lv_style_selector_t)(LV_PART_ITEMS | LV_STATE_CHECKED));

    // Tab change event
    lv_obj_add_event_cb(_tabview, onTabChange, LV_EVENT_VALUE_CHANGED, this);

    createControlTab();
    createCalibrateTab();
    createSettingsTab();

    syncState();
    Serial.println("UI created");
}

void DisplayUI::createStyles() {
    // Styles are created inline for simplicity
}

void DisplayUI::createControlTab() {
    // Set tab to use flex column layout with gaps
    lv_obj_set_style_bg_color(_tabControl, lv_color_hex(0x1a1a2e), 0);
    lv_obj_set_style_pad_all(_tabControl, 6, 0);
    lv_obj_set_flex_flow(_tabControl, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(_tabControl, 6, 0);

    // Power switch row
    lv_obj_t* powerRow = lv_obj_create(_tabControl);
    lv_obj_set_size(powerRow, lv_pct(100), 40);
    lv_obj_set_style_bg_color(powerRow, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_border_width(powerRow, 0, 0);
    lv_obj_set_style_radius(powerRow, 8, 0);
    lv_obj_set_style_pad_all(powerRow, 8, 0);
    lv_obj_set_flex_flow(powerRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(powerRow, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* powerLabel = lv_label_create(powerRow);
    lv_label_set_text(powerLabel, "Power");
    lv_obj_set_style_text_color(powerLabel, lv_color_white(), 0);

    _powerSwitch = lv_switch_create(powerRow);
    lv_obj_set_style_bg_color(_powerSwitch, lv_color_hex(0x00b894), (lv_style_selector_t)(LV_PART_INDICATOR | LV_STATE_CHECKED));
    lv_obj_add_event_cb(_powerSwitch, onPowerSwitch, LV_EVENT_VALUE_CHANGED, this);

    // Brightness row
    lv_obj_t* brightRow = lv_obj_create(_tabControl);
    lv_obj_set_size(brightRow, lv_pct(100), 50);
    lv_obj_set_style_bg_color(brightRow, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_border_width(brightRow, 0, 0);
    lv_obj_set_style_radius(brightRow, 8, 0);
    lv_obj_set_style_pad_all(brightRow, 8, 0);

    lv_obj_t* brightLabel = lv_label_create(brightRow);
    lv_label_set_text(brightLabel, "Brightness");
    lv_obj_set_style_text_color(brightLabel, lv_color_hex(0xaaaaaa), 0);
    lv_obj_set_style_text_font(brightLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(brightLabel, LV_ALIGN_TOP_LEFT, 0, 0);

    _brightnessLabel = lv_label_create(brightRow);
    lv_label_set_text(_brightnessLabel, "50%");
    lv_obj_set_style_text_color(_brightnessLabel, lv_color_white(), 0);
    lv_obj_set_style_text_font(_brightnessLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(_brightnessLabel, LV_ALIGN_TOP_RIGHT, 0, 0);

    _brightnessSlider = lv_slider_create(brightRow);
    lv_obj_set_width(_brightnessSlider, lv_pct(100));
    lv_obj_align(_brightnessSlider, LV_ALIGN_BOTTOM_MID, 0, -2);
    lv_slider_set_range(_brightnessSlider, 0, 255);
    lv_obj_set_style_bg_color(_brightnessSlider, lv_color_hex(0x444444), LV_PART_MAIN);
    lv_obj_set_style_bg_color(_brightnessSlider, lv_color_hex(0x00b894), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(_brightnessSlider, lv_color_white(), LV_PART_KNOB);
    lv_obj_add_event_cb(_brightnessSlider, onBrightnessChange, LV_EVENT_VALUE_CHANGED, this);

    // Color picker row
    lv_obj_t* colorRow = lv_obj_create(_tabControl);
    lv_obj_set_size(colorRow, lv_pct(100), 80);
    lv_obj_set_style_bg_color(colorRow, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_border_width(colorRow, 0, 0);
    lv_obj_set_style_radius(colorRow, 8, 0);
    lv_obj_set_style_pad_all(colorRow, 8, 0);

    lv_obj_t* colorLabel = lv_label_create(colorRow);
    lv_label_set_text(colorLabel, "Color");
    lv_obj_set_style_text_color(colorLabel, lv_color_hex(0xaaaaaa), 0);
    lv_obj_set_style_text_font(colorLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(colorLabel, LV_ALIGN_TOP_LEFT, 0, 0);

    _colorWheel = lv_colorwheel_create(colorRow, true);
    lv_obj_set_size(_colorWheel, 60, 60);
    lv_obj_align(_colorWheel, LV_ALIGN_RIGHT_MID, -5, 0);
    lv_obj_add_event_cb(_colorWheel, onColorChange, LV_EVENT_VALUE_CHANGED, this);

    // Animation buttons
    lv_obj_t* animRow = lv_obj_create(_tabControl);
    lv_obj_set_size(animRow, lv_pct(100), 85);
    lv_obj_set_style_bg_color(animRow, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_border_width(animRow, 0, 0);
    lv_obj_set_style_radius(animRow, 8, 0);
    lv_obj_set_style_pad_all(animRow, 6, 0);

    lv_obj_t* animLabel = lv_label_create(animRow);
    lv_label_set_text(animLabel, "Animation");
    lv_obj_set_style_text_color(animLabel, lv_color_hex(0xaaaaaa), 0);
    lv_obj_set_style_text_font(animLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(animLabel, LV_ALIGN_TOP_LEFT, 0, 0);

    _animBtnMatrix = lv_btnmatrix_create(animRow);
    lv_btnmatrix_set_map(_animBtnMatrix, ANIM_BASIC_MAP);
    lv_obj_set_size(_animBtnMatrix, lv_pct(100), 60);
    lv_obj_align(_animBtnMatrix, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(_animBtnMatrix, lv_color_hex(0x16213e), LV_PART_MAIN);
    lv_obj_set_style_border_width(_animBtnMatrix, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(_animBtnMatrix, lv_color_hex(0x2d3436), LV_PART_ITEMS);
    lv_obj_set_style_text_color(_animBtnMatrix, lv_color_white(), LV_PART_ITEMS);
    lv_obj_set_style_text_font(_animBtnMatrix, &lv_font_montserrat_12, LV_PART_ITEMS);
    lv_obj_set_style_bg_color(_animBtnMatrix, lv_color_hex(0x00b894), (lv_style_selector_t)(LV_PART_ITEMS | LV_STATE_CHECKED));
    lv_btnmatrix_set_btn_ctrl_all(_animBtnMatrix, LV_BTNMATRIX_CTRL_CHECKABLE);
    lv_btnmatrix_set_one_checked(_animBtnMatrix, true);
    lv_obj_add_event_cb(_animBtnMatrix, onAnimationSelect, LV_EVENT_VALUE_CHANGED, this);

    // Speed slider row
    lv_obj_t* speedRow = lv_obj_create(_tabControl);
    lv_obj_set_size(speedRow, lv_pct(100), 50);
    lv_obj_set_style_bg_color(speedRow, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_border_width(speedRow, 0, 0);
    lv_obj_set_style_radius(speedRow, 8, 0);
    lv_obj_set_style_pad_all(speedRow, 8, 0);

    lv_obj_t* speedLabel = lv_label_create(speedRow);
    lv_label_set_text(speedLabel, "Speed");
    lv_obj_set_style_text_color(speedLabel, lv_color_hex(0xaaaaaa), 0);
    lv_obj_set_style_text_font(speedLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(speedLabel, LV_ALIGN_TOP_LEFT, 0, 0);

    _speedLabel = lv_label_create(speedRow);
    lv_label_set_text(_speedLabel, "50ms");
    lv_obj_set_style_text_color(_speedLabel, lv_color_white(), 0);
    lv_obj_set_style_text_font(_speedLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(_speedLabel, LV_ALIGN_TOP_RIGHT, 0, 0);

    _speedSlider = lv_slider_create(speedRow);
    lv_obj_set_width(_speedSlider, lv_pct(100));
    lv_obj_align(_speedSlider, LV_ALIGN_BOTTOM_MID, 0, -2);
    lv_slider_set_range(_speedSlider, 10, 200);
    lv_obj_set_style_bg_color(_speedSlider, lv_color_hex(0x444444), LV_PART_MAIN);
    lv_obj_set_style_bg_color(_speedSlider, lv_color_hex(0x6c5ce7), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(_speedSlider, lv_color_white(), LV_PART_KNOB);
    lv_obj_add_event_cb(_speedSlider, onSpeedChange, LV_EVENT_VALUE_CHANGED, this);
}

void DisplayUI::createCalibrateTab() {
    lv_obj_set_style_bg_color(_tabCalibrate, lv_color_hex(0x1a1a2e), 0);
    lv_obj_set_style_pad_all(_tabCalibrate, 6, 0);
    lv_obj_set_flex_flow(_tabCalibrate, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(_tabCalibrate, 6, 0);

    // LED indicator
    lv_obj_t* ledIndicator = lv_obj_create(_tabCalibrate);
    lv_obj_set_size(ledIndicator, lv_pct(100), 55);
    lv_obj_set_style_bg_color(ledIndicator, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_border_width(ledIndicator, 0, 0);
    lv_obj_set_style_radius(ledIndicator, 8, 0);

    _calibLedLabel = lv_label_create(ledIndicator);
    lv_label_set_text(_calibLedLabel, "LED: --");
    lv_obj_set_style_text_color(_calibLedLabel, lv_color_white(), 0);
    lv_obj_set_style_text_font(_calibLedLabel, &lv_font_montserrat_20, 0);
    lv_obj_center(_calibLedLabel);

    // Navigation buttons
    lv_obj_t* navRow = lv_obj_create(_tabCalibrate);
    lv_obj_set_size(navRow, lv_pct(100), 40);
    lv_obj_set_style_bg_opa(navRow, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(navRow, 0, 0);
    lv_obj_set_style_pad_all(navRow, 0, 0);
    lv_obj_set_flex_flow(navRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(navRow, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    _calibPrevBtn = lv_btn_create(navRow);
    lv_obj_set_size(_calibPrevBtn, 65, 35);
    lv_obj_set_style_bg_color(_calibPrevBtn, lv_color_hex(0x2d3436), 0);
    lv_obj_t* prevLabel = lv_label_create(_calibPrevBtn);
    lv_label_set_text(prevLabel, LV_SYMBOL_LEFT " Prev");
    lv_obj_center(prevLabel);
    lv_obj_add_event_cb(_calibPrevBtn, onCalibPrev, LV_EVENT_CLICKED, this);

    _calibStartBtn = lv_btn_create(navRow);
    lv_obj_set_size(_calibStartBtn, 65, 35);
    lv_obj_set_style_bg_color(_calibStartBtn, lv_color_hex(0x00b894), 0);
    lv_obj_t* startLabel = lv_label_create(_calibStartBtn);
    lv_label_set_text(startLabel, "Start");
    lv_obj_center(startLabel);
    lv_obj_add_event_cb(_calibStartBtn, onCalibStart, LV_EVENT_CLICKED, this);

    _calibNextBtn = lv_btn_create(navRow);
    lv_obj_set_size(_calibNextBtn, 65, 35);
    lv_obj_set_style_bg_color(_calibNextBtn, lv_color_hex(0x2d3436), 0);
    lv_obj_t* nextLabel = lv_label_create(_calibNextBtn);
    lv_label_set_text(nextLabel, "Next " LV_SYMBOL_RIGHT);
    lv_obj_center(nextLabel);
    lv_obj_add_event_cb(_calibNextBtn, onCalibNext, LV_EVENT_CLICKED, this);

    // Position sliders container
    lv_obj_t* posRow = lv_obj_create(_tabCalibrate);
    lv_obj_set_size(posRow, lv_pct(100), 120);
    lv_obj_set_style_bg_color(posRow, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_border_width(posRow, 0, 0);
    lv_obj_set_style_radius(posRow, 8, 0);
    lv_obj_set_style_pad_all(posRow, 8, 0);
    lv_obj_set_flex_flow(posRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(posRow, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(posRow, LV_OBJ_FLAG_HIDDEN);  // Hidden initially

    // X slider
    lv_obj_t* xCont = lv_obj_create(posRow);
    lv_obj_set_size(xCont, 55, 100);
    lv_obj_set_style_bg_opa(xCont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(xCont, 0, 0);
    lv_obj_set_style_pad_all(xCont, 0, 0);

    lv_obj_t* xLabel = lv_label_create(xCont);
    lv_label_set_text(xLabel, "X");
    lv_obj_set_style_text_color(xLabel, lv_color_hex(0xff6b6b), 0);
    lv_obj_set_style_text_font(xLabel, &lv_font_montserrat_14, 0);
    lv_obj_align(xLabel, LV_ALIGN_TOP_MID, 0, 0);

    _calibXSlider = lv_slider_create(xCont);
    lv_slider_set_range(_calibXSlider, -100, 100);
    lv_obj_set_size(_calibXSlider, 18, 70);
    lv_obj_align(_calibXSlider, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(_calibXSlider, lv_color_hex(0x444444), LV_PART_MAIN);
    lv_obj_set_style_bg_color(_calibXSlider, lv_color_hex(0xff6b6b), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(_calibXSlider, lv_color_hex(0xff6b6b), LV_PART_KNOB);
    lv_obj_add_event_cb(_calibXSlider, onCalibPosChange, LV_EVENT_VALUE_CHANGED, this);

    // Y slider
    lv_obj_t* yCont = lv_obj_create(posRow);
    lv_obj_set_size(yCont, 55, 100);
    lv_obj_set_style_bg_opa(yCont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(yCont, 0, 0);
    lv_obj_set_style_pad_all(yCont, 0, 0);

    lv_obj_t* yLabel = lv_label_create(yCont);
    lv_label_set_text(yLabel, "Y");
    lv_obj_set_style_text_color(yLabel, lv_color_hex(0x51cf66), 0);
    lv_obj_set_style_text_font(yLabel, &lv_font_montserrat_14, 0);
    lv_obj_align(yLabel, LV_ALIGN_TOP_MID, 0, 0);

    _calibYSlider = lv_slider_create(yCont);
    lv_slider_set_range(_calibYSlider, -100, 100);
    lv_obj_set_size(_calibYSlider, 18, 70);
    lv_obj_align(_calibYSlider, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(_calibYSlider, lv_color_hex(0x444444), LV_PART_MAIN);
    lv_obj_set_style_bg_color(_calibYSlider, lv_color_hex(0x51cf66), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(_calibYSlider, lv_color_hex(0x51cf66), LV_PART_KNOB);
    lv_obj_add_event_cb(_calibYSlider, onCalibPosChange, LV_EVENT_VALUE_CHANGED, this);

    // Z slider
    lv_obj_t* zCont = lv_obj_create(posRow);
    lv_obj_set_size(zCont, 55, 100);
    lv_obj_set_style_bg_opa(zCont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(zCont, 0, 0);
    lv_obj_set_style_pad_all(zCont, 0, 0);

    lv_obj_t* zLabel = lv_label_create(zCont);
    lv_label_set_text(zLabel, "Z");
    lv_obj_set_style_text_color(zLabel, lv_color_hex(0x339af0), 0);
    lv_obj_set_style_text_font(zLabel, &lv_font_montserrat_14, 0);
    lv_obj_align(zLabel, LV_ALIGN_TOP_MID, 0, 0);

    _calibZSlider = lv_slider_create(zCont);
    lv_slider_set_range(_calibZSlider, -100, 100);
    lv_obj_set_size(_calibZSlider, 18, 70);
    lv_obj_align(_calibZSlider, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(_calibZSlider, lv_color_hex(0x444444), LV_PART_MAIN);
    lv_obj_set_style_bg_color(_calibZSlider, lv_color_hex(0x339af0), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(_calibZSlider, lv_color_hex(0x339af0), LV_PART_KNOB);
    lv_obj_add_event_cb(_calibZSlider, onCalibPosChange, LV_EVENT_VALUE_CHANGED, this);

    // Position label
    _calibPosLabel = lv_label_create(_tabCalibrate);
    lv_label_set_text(_calibPosLabel, "X: 0.00  Y: 0.00  Z: 0.00");
    lv_obj_set_style_text_color(_calibPosLabel, lv_color_hex(0xaaaaaa), 0);
    lv_obj_set_style_text_font(_calibPosLabel, &lv_font_montserrat_12, 0);
    lv_obj_add_flag(_calibPosLabel, LV_OBJ_FLAG_HIDDEN);

    // Action buttons row
    lv_obj_t* actionRow = lv_obj_create(_tabCalibrate);
    lv_obj_set_size(actionRow, lv_pct(100), 40);
    lv_obj_set_style_bg_opa(actionRow, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(actionRow, 0, 0);
    lv_obj_set_style_pad_all(actionRow, 0, 0);
    lv_obj_set_flex_flow(actionRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(actionRow, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(actionRow, LV_OBJ_FLAG_HIDDEN);

    _calibSaveBtn = lv_btn_create(actionRow);
    lv_obj_set_size(_calibSaveBtn, 95, 35);
    lv_obj_set_style_bg_color(_calibSaveBtn, lv_color_hex(0x00b894), 0);
    lv_obj_t* saveLabel = lv_label_create(_calibSaveBtn);
    lv_label_set_text(saveLabel, LV_SYMBOL_SAVE " Save");
    lv_obj_center(saveLabel);
    lv_obj_add_event_cb(_calibSaveBtn, onCalibSave, LV_EVENT_CLICKED, this);

    _calibExitBtn = lv_btn_create(actionRow);
    lv_obj_set_size(_calibExitBtn, 95, 35);
    lv_obj_set_style_bg_color(_calibExitBtn, lv_color_hex(0x636e72), 0);
    lv_obj_t* exitLabel = lv_label_create(_calibExitBtn);
    lv_label_set_text(exitLabel, LV_SYMBOL_CLOSE " Exit");
    lv_obj_center(exitLabel);
    lv_obj_add_event_cb(_calibExitBtn, onCalibExit, LV_EVENT_CLICKED, this);

    // Initial state
    lv_obj_add_state(_calibPrevBtn, LV_STATE_DISABLED);
    lv_obj_add_state(_calibNextBtn, LV_STATE_DISABLED);
}

void DisplayUI::createSettingsTab() {
    lv_obj_set_style_bg_color(_tabSettings, lv_color_hex(0x1a1a2e), 0);
    lv_obj_set_style_pad_all(_tabSettings, 6, 0);
    lv_obj_set_flex_flow(_tabSettings, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(_tabSettings, 8, 0);

    // WiFi status card
    lv_obj_t* wifiCard = lv_obj_create(_tabSettings);
    lv_obj_set_size(wifiCard, lv_pct(100), 90);
    lv_obj_set_style_bg_color(wifiCard, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_border_width(wifiCard, 0, 0);
    lv_obj_set_style_radius(wifiCard, 8, 0);
    lv_obj_set_style_pad_all(wifiCard, 10, 0);

    lv_obj_t* wifiTitle = lv_label_create(wifiCard);
    lv_label_set_text(wifiTitle, LV_SYMBOL_WIFI " WiFi Status");
    lv_obj_set_style_text_color(wifiTitle, lv_color_hex(0xaaaaaa), 0);
    lv_obj_set_style_text_font(wifiTitle, &lv_font_montserrat_12, 0);
    lv_obj_align(wifiTitle, LV_ALIGN_TOP_LEFT, 0, 0);

    _wifiStatusLabel = lv_label_create(wifiCard);
    lv_label_set_text(_wifiStatusLabel, "Connecting...");
    lv_obj_set_style_text_color(_wifiStatusLabel, lv_color_white(), 0);
    lv_obj_set_style_text_font(_wifiStatusLabel, &lv_font_montserrat_14, 0);
    lv_obj_align(_wifiStatusLabel, LV_ALIGN_TOP_LEFT, 0, 20);

    _ipLabel = lv_label_create(wifiCard);
    lv_label_set_text(_ipLabel, "IP: --");
    lv_obj_set_style_text_color(_ipLabel, lv_color_hex(0x00b894), 0);
    lv_obj_set_style_text_font(_ipLabel, &lv_font_montserrat_16, 0);
    lv_obj_align(_ipLabel, LV_ALIGN_TOP_LEFT, 0, 42);

    // Info card
    lv_obj_t* infoCard = lv_obj_create(_tabSettings);
    lv_obj_set_size(infoCard, lv_pct(100), 70);
    lv_obj_set_style_bg_color(infoCard, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_border_width(infoCard, 0, 0);
    lv_obj_set_style_radius(infoCard, 8, 0);
    lv_obj_set_style_pad_all(infoCard, 10, 0);

    lv_obj_t* infoTitle = lv_label_create(infoCard);
    lv_label_set_text(infoTitle, LV_SYMBOL_HOME " Christmas Lights");
    lv_obj_set_style_text_color(infoTitle, lv_color_hex(0xaaaaaa), 0);
    lv_obj_set_style_text_font(infoTitle, &lv_font_montserrat_12, 0);
    lv_obj_align(infoTitle, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_t* infoText = lv_label_create(infoCard);
    lv_label_set_text(infoText, "WS2811 50 LEDs\nESP32-S3 Touch LCD");
    lv_obj_set_style_text_color(infoText, lv_color_white(), 0);
    lv_obj_set_style_text_font(infoText, &lv_font_montserrat_12, 0);
    lv_obj_align(infoText, LV_ALIGN_TOP_LEFT, 0, 18);

    updateWifiStatus();
}

void DisplayUI::update() {
    static unsigned long lastWifiUpdate = 0;
    if (millis() - lastWifiUpdate > 2000) {
        updateWifiStatus();
        lastWifiUpdate = millis();
    }
}

void DisplayUI::syncState() {
    // Power
    if (_ledController.isOn()) {
        lv_obj_add_state(_powerSwitch, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(_powerSwitch, LV_STATE_CHECKED);
    }

    // Brightness
    lv_slider_set_value(_brightnessSlider, _ledController.getBrightness(), LV_ANIM_OFF);
    char brightBuf[10];
    snprintf(brightBuf, sizeof(brightBuf), "%d%%", _ledController.getBrightness() * 100 / 255);
    lv_label_set_text(_brightnessLabel, brightBuf);

    // Color
    CRGB color = _ledController.getSolidColor();
    lv_colorwheel_set_rgb(_colorWheel, crgbToLvColor(color));

    // Animation
    int anim = (int)_ledController.getAnimation();
    if (anim >= 0 && anim < 9) {
        lv_btnmatrix_set_btn_ctrl(_animBtnMatrix, anim, LV_BTNMATRIX_CTRL_CHECKED);
    }

    // Speed
    lv_slider_set_value(_speedSlider, _ledController.getAnimationSpeed(), LV_ANIM_OFF);
    char speedBuf[10];
    snprintf(speedBuf, sizeof(speedBuf), "%dms", _ledController.getAnimationSpeed());
    lv_label_set_text(_speedLabel, speedBuf);
}

void DisplayUI::updateCalibrationUI() {
    // Get position row (child index 2 of calibrate tab)
    lv_obj_t* posRow = lv_obj_get_child(_tabCalibrate, 2);
    lv_obj_t* actionRow = lv_obj_get_child(_tabCalibrate, 4);

    if (_currentCalibLed < 0) {
        lv_label_set_text(_calibLedLabel, "LED: --");
        lv_obj_add_state(_calibPrevBtn, LV_STATE_DISABLED);
        lv_obj_add_state(_calibNextBtn, LV_STATE_DISABLED);
        lv_obj_clear_flag(_calibStartBtn, LV_OBJ_FLAG_HIDDEN);

        if (posRow) lv_obj_add_flag(posRow, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(_calibPosLabel, LV_OBJ_FLAG_HIDDEN);
        if (actionRow) lv_obj_add_flag(actionRow, LV_OBJ_FLAG_HIDDEN);
    } else {
        char buf[20];
        snprintf(buf, sizeof(buf), "LED: %d / 50", _currentCalibLed + 1);
        lv_label_set_text(_calibLedLabel, buf);

        // Enable/disable nav buttons
        if (_currentCalibLed <= 0) {
            lv_obj_add_state(_calibPrevBtn, LV_STATE_DISABLED);
        } else {
            lv_obj_clear_state(_calibPrevBtn, LV_STATE_DISABLED);
        }

        if (_currentCalibLed >= NUM_LEDS - 1) {
            lv_obj_add_state(_calibNextBtn, LV_STATE_DISABLED);
        } else {
            lv_obj_clear_state(_calibNextBtn, LV_STATE_DISABLED);
        }

        lv_obj_add_flag(_calibStartBtn, LV_OBJ_FLAG_HIDDEN);

        if (posRow) lv_obj_clear_flag(posRow, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(_calibPosLabel, LV_OBJ_FLAG_HIDDEN);
        if (actionRow) lv_obj_clear_flag(actionRow, LV_OBJ_FLAG_HIDDEN);

        // Update slider positions
        LEDPosition pos = _calibration.getPosition(_currentCalibLed);
        lv_slider_set_value(_calibXSlider, (int)(pos.x * 100), LV_ANIM_OFF);
        lv_slider_set_value(_calibYSlider, (int)(pos.y * 100), LV_ANIM_OFF);
        lv_slider_set_value(_calibZSlider, (int)(pos.z * 100), LV_ANIM_OFF);

        // Update position label
        char posBuf[40];
        snprintf(posBuf, sizeof(posBuf), "X: %.2f  Y: %.2f  Z: %.2f", pos.x, pos.y, pos.z);
        lv_label_set_text(_calibPosLabel, posBuf);
    }
}

void DisplayUI::updateWifiStatus() {
    WiFiState state = _wifiManager.getState();

    switch (state) {
        case WIFI_STATE_CONNECTED:
            lv_label_set_text(_wifiStatusLabel, "Connected");
            lv_obj_set_style_text_color(_wifiStatusLabel, lv_color_hex(0x00b894), 0);
            break;
        case WIFI_STATE_AP_MODE:
            lv_label_set_text(_wifiStatusLabel, "AP Mode");
            lv_obj_set_style_text_color(_wifiStatusLabel, lv_color_hex(0xfdcb6e), 0);
            break;
        case WIFI_STATE_CONNECTING:
            lv_label_set_text(_wifiStatusLabel, "Connecting...");
            lv_obj_set_style_text_color(_wifiStatusLabel, lv_color_hex(0xaaaaaa), 0);
            break;
        default:
            lv_label_set_text(_wifiStatusLabel, "Disconnected");
            lv_obj_set_style_text_color(_wifiStatusLabel, lv_color_hex(0xff7675), 0);
            break;
    }

    char ipBuf[30];
    snprintf(ipBuf, sizeof(ipBuf), "IP: %s", _wifiManager.getIPAddress().c_str());
    lv_label_set_text(_ipLabel, ipBuf);
}

lv_color_t DisplayUI::crgbToLvColor(CRGB color) {
    return lv_color_make(color.r, color.g, color.b);
}

CRGB DisplayUI::lvColorToCrgb(lv_color_t color) {
    return CRGB(
        (color.full >> 11) << 3,
        ((color.full >> 5) & 0x3F) << 2,
        (color.full & 0x1F) << 3
    );
}

// ============================================
// Event Handlers
// ============================================

void DisplayUI::onPowerSwitch(lv_event_t* e) {
    DisplayUI* ui = (DisplayUI*)lv_event_get_user_data(e);
    lv_obj_t* sw = lv_event_get_target(e);
    bool isOn = lv_obj_has_state(sw, LV_STATE_CHECKED);
    ui->_ledController.setOn(isOn);
}

void DisplayUI::onBrightnessChange(lv_event_t* e) {
    DisplayUI* ui = (DisplayUI*)lv_event_get_user_data(e);
    lv_obj_t* slider = lv_event_get_target(e);
    int val = lv_slider_get_value(slider);
    ui->_ledController.setBrightness(val);

    char buf[10];
    snprintf(buf, sizeof(buf), "%d%%", val * 100 / 255);
    lv_label_set_text(ui->_brightnessLabel, buf);
}

void DisplayUI::onColorChange(lv_event_t* e) {
    DisplayUI* ui = (DisplayUI*)lv_event_get_user_data(e);
    lv_obj_t* cw = lv_event_get_target(e);
    lv_color_t color = lv_colorwheel_get_rgb(cw);
    ui->_ledController.setSolidColor(ui->lvColorToCrgb(color));
}

void DisplayUI::onAnimationSelect(lv_event_t* e) {
    DisplayUI* ui = (DisplayUI*)lv_event_get_user_data(e);
    lv_obj_t* btnm = lv_event_get_target(e);
    uint16_t btn = lv_btnmatrix_get_selected_btn(btnm);
    if (btn != LV_BTNMATRIX_BTN_NONE) {
        ui->_ledController.setAnimation(static_cast<AnimationMode>(btn));
    }
}

void DisplayUI::onSpeedChange(lv_event_t* e) {
    DisplayUI* ui = (DisplayUI*)lv_event_get_user_data(e);
    lv_obj_t* slider = lv_event_get_target(e);
    int val = lv_slider_get_value(slider);
    ui->_ledController.setAnimationSpeed(val);

    char buf[10];
    snprintf(buf, sizeof(buf), "%dms", val);
    lv_label_set_text(ui->_speedLabel, buf);
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

void DisplayUI::onCalibStart(lv_event_t* e) {
    DisplayUI* ui = (DisplayUI*)lv_event_get_user_data(e);
    ui->_currentCalibLed = 0;
    ui->_calibration.setCalibrationLED(0);
    ui->updateCalibrationUI();
}

void DisplayUI::onCalibExit(lv_event_t* e) {
    DisplayUI* ui = (DisplayUI*)lv_event_get_user_data(e);
    ui->_currentCalibLed = -1;
    ui->_calibration.setCalibrationLED(-1);
    ui->updateCalibrationUI();
}

void DisplayUI::onCalibSave(lv_event_t* e) {
    DisplayUI* ui = (DisplayUI*)lv_event_get_user_data(e);
    if (ui->_calibration.save()) {
        lv_obj_t* msgbox = lv_msgbox_create(NULL, "Success", "Calibration saved!", NULL, true);
        lv_obj_center(msgbox);
    } else {
        lv_obj_t* msgbox = lv_msgbox_create(NULL, "Error", "Failed to save!", NULL, true);
        lv_obj_center(msgbox);
    }
}

void DisplayUI::onCalibReset(lv_event_t* e) {
    DisplayUI* ui = (DisplayUI*)lv_event_get_user_data(e);
    ui->_calibration.resetToLinear();
    ui->updateCalibrationUI();
}

void DisplayUI::onCalibPosChange(lv_event_t* e) {
    DisplayUI* ui = (DisplayUI*)lv_event_get_user_data(e);

    if (ui->_currentCalibLed < 0) return;

    float x = lv_slider_get_value(ui->_calibXSlider) / 100.0f;
    float y = lv_slider_get_value(ui->_calibYSlider) / 100.0f;
    float z = lv_slider_get_value(ui->_calibZSlider) / 100.0f;

    ui->_calibration.setPosition(ui->_currentCalibLed, x, y, z);

    char posBuf[40];
    snprintf(posBuf, sizeof(posBuf), "X: %.2f  Y: %.2f  Z: %.2f", x, y, z);
    lv_label_set_text(ui->_calibPosLabel, posBuf);
}

void DisplayUI::onTabChange(lv_event_t* e) {
    DisplayUI* ui = (DisplayUI*)lv_event_get_user_data(e);
    lv_obj_t* tabview = lv_event_get_target(e);
    uint16_t tab = lv_tabview_get_tab_act(tabview);

    // Exit calibration mode when leaving calibrate tab
    if (tab != 1 && ui->_currentCalibLed >= 0) {
        ui->_currentCalibLed = -1;
        ui->_calibration.setCalibrationLED(-1);
        ui->updateCalibrationUI();
    }
}
