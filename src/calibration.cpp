#include "calibration.h"
#include <algorithm>

const char* Calibration::CALIBRATION_FILE = "/calibration.json";

Calibration::Calibration() : _calibrationLED(-1) {
    resetToLinear();
}

bool Calibration::begin() {
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS mount failed");
        return false;
    }

    if (SPIFFS.exists(CALIBRATION_FILE)) {
        if (load()) {
            Serial.println("Calibration loaded from SPIFFS");
            return true;
        }
    }

    Serial.println("Using default linear calibration");
    return true;
}

void Calibration::setPosition(uint16_t index, float x, float y, float z) {
    if (index < NUM_LEDS) {
        _positions[index].x = constrain(x, -1.0f, 1.0f);
        _positions[index].y = constrain(y, -1.0f, 1.0f);
        _positions[index].z = constrain(z, -1.0f, 1.0f);
    }
}

LEDPosition Calibration::getPosition(uint16_t index) const {
    if (index < NUM_LEDS) {
        return _positions[index];
    }
    return {0, 0, 0};
}

void Calibration::getSortedByAxis(uint8_t axis, uint16_t* outIndices) const {
    // Initialize with sequential indices
    for (uint16_t i = 0; i < NUM_LEDS; i++) {
        outIndices[i] = i;
    }

    // Sort by the specified axis
    std::sort(outIndices, outIndices + NUM_LEDS, [this, axis](uint16_t a, uint16_t b) {
        float valA, valB;
        switch (axis) {
            case 0: valA = _positions[a].x; valB = _positions[b].x; break;
            case 1: valA = _positions[a].y; valB = _positions[b].y; break;
            case 2: valA = _positions[a].z; valB = _positions[b].z; break;
            default: valA = valB = 0; break;
        }
        return valA < valB;
    });
}

void Calibration::getSortedByAngle(uint16_t* outIndices) const {
    for (uint16_t i = 0; i < NUM_LEDS; i++) {
        outIndices[i] = i;
    }

    std::sort(outIndices, outIndices + NUM_LEDS, [this](uint16_t a, uint16_t b) {
        float angleA = atan2(_positions[a].z, _positions[a].x);
        float angleB = atan2(_positions[b].z, _positions[b].x);
        return angleA < angleB;
    });
}

void Calibration::getSortedByRadius(uint16_t* outIndices) const {
    for (uint16_t i = 0; i < NUM_LEDS; i++) {
        outIndices[i] = i;
    }

    std::sort(outIndices, outIndices + NUM_LEDS, [this](uint16_t a, uint16_t b) {
        float radA = _positions[a].x * _positions[a].x +
                     _positions[a].y * _positions[a].y +
                     _positions[a].z * _positions[a].z;
        float radB = _positions[b].x * _positions[b].x +
                     _positions[b].y * _positions[b].y +
                     _positions[b].z * _positions[b].z;
        return radA < radB;
    });
}

bool Calibration::save() {
    File file = SPIFFS.open(CALIBRATION_FILE, "w");
    if (!file) {
        Serial.println("Failed to open calibration file for writing");
        return false;
    }

    JsonDocument doc;
    JsonArray positions = doc["positions"].to<JsonArray>();

    for (uint16_t i = 0; i < NUM_LEDS; i++) {
        JsonObject pos = positions.add<JsonObject>();
        pos["x"] = _positions[i].x;
        pos["y"] = _positions[i].y;
        pos["z"] = _positions[i].z;
    }

    if (serializeJson(doc, file) == 0) {
        Serial.println("Failed to write calibration data");
        file.close();
        return false;
    }

    file.close();
    Serial.println("Calibration saved");
    return true;
}

bool Calibration::load() {
    File file = SPIFFS.open(CALIBRATION_FILE, "r");
    if (!file) {
        return false;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.print("Failed to parse calibration: ");
        Serial.println(error.c_str());
        return false;
    }

    JsonArray positions = doc["positions"].as<JsonArray>();
    uint16_t i = 0;
    for (JsonObject pos : positions) {
        if (i >= NUM_LEDS) break;
        _positions[i].x = pos["x"] | 0.0f;
        _positions[i].y = pos["y"] | 0.0f;
        _positions[i].z = pos["z"] | 0.0f;
        i++;
    }

    return true;
}

void Calibration::resetToLinear() {
    for (uint16_t i = 0; i < NUM_LEDS; i++) {
        // Default: spread evenly along X axis from -1 to 1
        _positions[i].x = (float)i / (NUM_LEDS - 1) * 2.0f - 1.0f;
        _positions[i].y = 0.0f;
        _positions[i].z = 0.0f;
    }
}
