#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include <FastLED.h>
#include "config.h"
#include "calibration.h"

enum AnimationMode {
    ANIMATION_STATIC = 0,
    ANIMATION_RAINBOW,
    ANIMATION_CHASE,
    ANIMATION_TWINKLE,
    ANIMATION_FADE,
    ANIMATION_SPARKLE,
    ANIMATION_CANDY_CANE,
    ANIMATION_SNOW,
    ANIMATION_FIRE,
    // Spatial animations (use calibration data)
    ANIMATION_SPATIAL_WAVE,
    ANIMATION_SPATIAL_RAINBOW,
    ANIMATION_SPATIAL_PULSE,
    ANIMATION_SPATIAL_ROTATE,
    ANIMATION_SPATIAL_PLANES,
    ANIMATION_CUSTOM
};

class LEDController {
public:
    LEDController(Calibration& calibration);
    void begin();
    void update();

    // Basic controls
    void setOn(bool on);
    bool isOn() const { return _isOn; }
    void setBrightness(uint8_t brightness);
    uint8_t getBrightness() const { return _brightness; }

    // Color controls
    void setSolidColor(CRGB color);
    void setPixelColor(uint16_t index, CRGB color);
    CRGB getSolidColor() const { return _solidColor; }

    // Animation controls
    void setAnimation(AnimationMode mode);
    AnimationMode getAnimation() const { return _currentAnimation; }
    void setAnimationSpeed(uint16_t speedMs);
    uint16_t getAnimationSpeed() const { return _animationSpeed; }

    // Get LED data for custom patterns
    CRGB* getLeds() { return _leds; }

    // Calibration mode
    void showCalibrationLED(int16_t index);

private:
    CRGB _leds[NUM_LEDS];
    Calibration& _calibration;
    bool _isOn;
    uint8_t _brightness;
    CRGB _solidColor;
    AnimationMode _currentAnimation;
    uint16_t _animationSpeed;
    unsigned long _lastUpdate;
    float _animationPhase;  // Changed to float for smoother spatial animations

    // Basic animation functions
    void animateRainbow();
    void animateChase();
    void animateTwinkle();
    void animateFade();
    void animateSparkle();
    void animateCandyCane();
    void animateSnow();
    void animateFire();

    // Spatial animation functions (use 3D positions)
    void animateSpatialWave();
    void animateSpatialRainbow();
    void animateSpatialPulse();
    void animateSpatialRotate();
    void animateSpatialPlanes();
};

#endif // LED_CONTROLLER_H
