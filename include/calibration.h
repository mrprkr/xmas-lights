#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include "config.h"

struct LEDPosition {
    float x;  // -1.0 to 1.0 normalized coordinates
    float y;  // -1.0 to 1.0
    float z;  // -1.0 to 1.0
};

class Calibration {
public:
    Calibration();
    bool begin();

    // SD card status
    bool isSDAvailable() const { return _sdAvailable; }

    // Position management
    void setPosition(uint16_t index, float x, float y, float z);
    LEDPosition getPosition(uint16_t index) const;
    LEDPosition* getAllPositions() { return _positions; }

    // Get sorted indices for directional animations
    // Returns indices sorted by position along an axis
    void getSortedByAxis(uint8_t axis, uint16_t* outIndices) const;  // 0=X, 1=Y, 2=Z

    // Get indices sorted by angle around Y axis (for rotational effects)
    void getSortedByAngle(uint16_t* outIndices) const;

    // Get indices sorted by distance from center
    void getSortedByRadius(uint16_t* outIndices) const;

    // Persistence
    bool save();
    bool load();
    void resetToLinear();  // Default: evenly spaced along X axis

    // Calibration mode helpers
    void setCalibrationLED(int16_t index) { _calibrationLED = index; }
    int16_t getCalibrationLED() const { return _calibrationLED; }
    bool isCalibrating() const { return _calibrationLED >= 0; }

private:
    LEDPosition _positions[NUM_LEDS];
    int16_t _calibrationLED;  // -1 = not calibrating
    bool _sdAvailable;
    SPIClass* _sdSPI;

    bool initSD();
    bool ensureDirectory(const char* path);
};

#endif // CALIBRATION_H
