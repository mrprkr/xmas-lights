#include "led_controller.h"

LEDController::LEDController(Calibration& calibration)
    : _calibration(calibration)
    , _isOn(true)
    , _brightness(DEFAULT_BRIGHTNESS)
    , _solidColor(CRGB::White)
    , _currentAnimation(ANIMATION_STATIC)
    , _animationSpeed(DEFAULT_ANIMATION_SPEED)
    , _lastUpdate(0)
    , _animationPhase(0) {
}

void LEDController::begin() {
    FastLED.addLeds<LED_TYPE, LED_DATA_PIN, COLOR_ORDER>(_leds, NUM_LEDS);
    FastLED.setBrightness(_brightness);
    FastLED.clear();
    FastLED.show();
}

void LEDController::update() {
    // Handle calibration mode
    if (_calibration.isCalibrating()) {
        showCalibrationLED(_calibration.getCalibrationLED());
        return;
    }

    if (!_isOn) {
        FastLED.clear();
        FastLED.show();
        return;
    }

    unsigned long now = millis();
    if (now - _lastUpdate < _animationSpeed) {
        return;
    }
    _lastUpdate = now;

    switch (_currentAnimation) {
        case ANIMATION_STATIC:
            // Static color - no animation needed
            break;
        case ANIMATION_RAINBOW:
            animateRainbow();
            break;
        case ANIMATION_CHASE:
            animateChase();
            break;
        case ANIMATION_TWINKLE:
            animateTwinkle();
            break;
        case ANIMATION_FADE:
            animateFade();
            break;
        case ANIMATION_SPARKLE:
            animateSparkle();
            break;
        case ANIMATION_CANDY_CANE:
            animateCandyCane();
            break;
        case ANIMATION_SNOW:
            animateSnow();
            break;
        case ANIMATION_FIRE:
            animateFire();
            break;
        case ANIMATION_SPATIAL_WAVE:
            animateSpatialWave();
            break;
        case ANIMATION_SPATIAL_RAINBOW:
            animateSpatialRainbow();
            break;
        case ANIMATION_SPATIAL_PULSE:
            animateSpatialPulse();
            break;
        case ANIMATION_SPATIAL_ROTATE:
            animateSpatialRotate();
            break;
        case ANIMATION_SPATIAL_PLANES:
            animateSpatialPlanes();
            break;
        default:
            break;
    }

    FastLED.show();
    _animationPhase += 0.05f;  // Increment phase for smooth animations
    if (_animationPhase > 2 * PI) {
        _animationPhase -= 2 * PI;
    }
}

void LEDController::setOn(bool on) {
    _isOn = on;
    if (!on) {
        FastLED.clear();
        FastLED.show();
    }
}

void LEDController::setBrightness(uint8_t brightness) {
    _brightness = brightness;
    FastLED.setBrightness(brightness);
    FastLED.show();
}

void LEDController::setSolidColor(CRGB color) {
    _solidColor = color;
    _currentAnimation = ANIMATION_STATIC;
    fill_solid(_leds, NUM_LEDS, color);
    FastLED.show();
}

void LEDController::setPixelColor(uint16_t index, CRGB color) {
    if (index < NUM_LEDS) {
        _leds[index] = color;
    }
}

void LEDController::setAnimation(AnimationMode mode) {
    _currentAnimation = mode;
    _animationPhase = 0;
    if (mode == ANIMATION_STATIC) {
        fill_solid(_leds, NUM_LEDS, _solidColor);
        FastLED.show();
    }
}

void LEDController::setAnimationSpeed(uint16_t speedMs) {
    _animationSpeed = speedMs;
}

void LEDController::showCalibrationLED(int16_t index) {
    FastLED.clear();
    if (index >= 0 && index < NUM_LEDS) {
        // Show the calibration LED in bright white
        _leds[index] = CRGB::White;
        // Show neighbors dimly to help with orientation
        if (index > 0) {
            _leds[index - 1] = CRGB(30, 0, 0);  // Previous: dim red
        }
        if (index < NUM_LEDS - 1) {
            _leds[index + 1] = CRGB(0, 30, 0);  // Next: dim green
        }
    }
    FastLED.show();
}

// ============================================
// Basic Animation Implementations
// ============================================

void LEDController::animateRainbow() {
    uint8_t hueStep = (uint8_t)(_animationPhase * 40.0f) % 256;
    fill_rainbow(_leds, NUM_LEDS, hueStep, 255 / NUM_LEDS);
}

void LEDController::animateChase() {
    fadeToBlackBy(_leds, NUM_LEDS, 50);
    int pos = (int)(_animationPhase * 3.0f) % NUM_LEDS;
    _leds[pos] = _solidColor;
}

void LEDController::animateTwinkle() {
    fadeToBlackBy(_leds, NUM_LEDS, 20);
    if (random8() < 80) {
        _leds[random16(NUM_LEDS)] = _solidColor;
    }
}

void LEDController::animateFade() {
    uint8_t brightness = beatsin8(30, 50, 255);
    fill_solid(_leds, NUM_LEDS, _solidColor);
    FastLED.setBrightness(scale8(brightness, _brightness));
}

void LEDController::animateSparkle() {
    fill_solid(_leds, NUM_LEDS, _solidColor);
    int pos = random16(NUM_LEDS);
    _leds[pos] = CRGB::White;
}

void LEDController::animateCandyCane() {
    int offset = (int)(_animationPhase * 2.0f);
    for (int i = 0; i < NUM_LEDS; i++) {
        if (((i + offset) % 6) < 3) {
            _leds[i] = CRGB::Red;
        } else {
            _leds[i] = CRGB::White;
        }
    }
}

void LEDController::animateSnow() {
    fadeToBlackBy(_leds, NUM_LEDS, 10);
    if (random8() < 30) {
        int pos = random16(NUM_LEDS);
        _leds[pos] = CRGB::White;
    }
}

void LEDController::animateFire() {
    static byte heat[NUM_LEDS];

    for (int i = 0; i < NUM_LEDS; i++) {
        heat[i] = qsub8(heat[i], random8(0, 35));
    }

    for (int k = NUM_LEDS - 1; k >= 2; k--) {
        heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
    }

    if (random8() < 120) {
        int y = random8(7);
        heat[y] = qadd8(heat[y], random8(160, 255));
    }

    for (int j = 0; j < NUM_LEDS; j++) {
        _leds[j] = HeatColor(heat[j]);
    }
}

// ============================================
// Spatial Animation Implementations
// These use the 3D calibration positions
// ============================================

void LEDController::animateSpatialWave() {
    // Wave traveling along X axis, using actual LED positions
    for (int i = 0; i < NUM_LEDS; i++) {
        LEDPosition pos = _calibration.getPosition(i);
        // Calculate wave based on X position
        float wave = sin(_animationPhase * 4.0f + pos.x * PI);
        uint8_t brightness = (uint8_t)((wave + 1.0f) * 127.5f);
        _leds[i] = _solidColor;
        _leds[i].nscale8(brightness);
    }
}

void LEDController::animateSpatialRainbow() {
    // Rainbow mapped to 3D position
    for (int i = 0; i < NUM_LEDS; i++) {
        LEDPosition pos = _calibration.getPosition(i);
        // Use combination of X and Y for hue
        float hueFloat = (pos.x + 1.0f) * 64.0f + (pos.y + 1.0f) * 64.0f + _animationPhase * 40.0f;
        uint8_t hue = (uint8_t)((int)hueFloat % 256);
        _leds[i] = CHSV(hue, 255, 255);
    }
}

void LEDController::animateSpatialPulse() {
    // Pulse emanating from center outward
    for (int i = 0; i < NUM_LEDS; i++) {
        LEDPosition pos = _calibration.getPosition(i);
        // Calculate distance from center
        float dist = sqrt(pos.x * pos.x + pos.y * pos.y + pos.z * pos.z);
        // Create expanding ring
        float ring = sin(_animationPhase * 3.0f - dist * PI * 2.0f);
        uint8_t brightness = (uint8_t)max(0.0f, ring * 255.0f);
        _leds[i] = _solidColor;
        _leds[i].nscale8(brightness);
    }
}

void LEDController::animateSpatialRotate() {
    // Rotating beam around Y axis
    for (int i = 0; i < NUM_LEDS; i++) {
        LEDPosition pos = _calibration.getPosition(i);
        // Calculate angle in XZ plane
        float angle = atan2(pos.z, pos.x);
        // Rotating beam
        float beam = cos(angle - _animationPhase * 2.0f);
        // Make beam narrower
        beam = pow(max(0.0f, beam), 4.0f);
        uint8_t brightness = (uint8_t)(beam * 255.0f);
        _leds[i] = _solidColor;
        _leds[i].nscale8(brightness);
    }
}

void LEDController::animateSpatialPlanes() {
    // Alternating horizontal planes (based on Y position)
    for (int i = 0; i < NUM_LEDS; i++) {
        LEDPosition pos = _calibration.getPosition(i);
        // Create horizontal bands that move up/down
        float band = sin(pos.y * PI * 3.0f + _animationPhase * 4.0f);

        // Determine color based on band position
        if (band > 0) {
            _leds[i] = CRGB::Red;
            _leds[i].nscale8((uint8_t)(band * 255.0f));
        } else {
            _leds[i] = CRGB::Green;
            _leds[i].nscale8((uint8_t)(-band * 255.0f));
        }
    }
}
