#include "calibration.h"
#include <algorithm>

Calibration::Calibration()
    : _calibrationLED(-1)
    , _sdAvailable(false)
    , _sdSPI(nullptr) {
    resetToLinear();
}

bool Calibration::begin() {
    // Skip SD card for now - pin conflict with display
    // TODO: Find correct SD card pins for ESP32-S3-LCD-2
    Serial.println("SD card disabled (pin conflict), using defaults");
    _sdAvailable = false;

    Serial.println("Using default linear calibration");
    return true;
}

bool Calibration::initSD() {
    Serial.println("Initializing SD card...");

    // Create SPI instance for SD card
    _sdSPI = new SPIClass(HSPI);
    _sdSPI->begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);

    // Try to mount SD card
    if (!SD.begin(SD_CS, *_sdSPI)) {
        Serial.println("SD card mount failed");
        delete _sdSPI;
        _sdSPI = nullptr;
        return false;
    }

    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("No SD card attached");
        return false;
    }

    Serial.print("SD Card Type: ");
    switch (cardType) {
        case CARD_MMC:  Serial.println("MMC"); break;
        case CARD_SD:   Serial.println("SDSC"); break;
        case CARD_SDHC: Serial.println("SDHC"); break;
        default:        Serial.println("UNKNOWN"); break;
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);

    // Ensure directory exists
    ensureDirectory(CALIBRATION_FILE_PATH);

    return true;
}

bool Calibration::ensureDirectory(const char* path) {
    // Extract directory from path
    String dirPath = String(path);
    int lastSlash = dirPath.lastIndexOf('/');
    if (lastSlash > 0) {
        dirPath = dirPath.substring(0, lastSlash);

        if (!SD.exists(dirPath)) {
            Serial.printf("Creating directory: %s\n", dirPath.c_str());
            if (!SD.mkdir(dirPath)) {
                Serial.println("Failed to create directory");
                return false;
            }
        }
    }
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
    if (!_sdAvailable) {
        Serial.println("SD card not available for saving");
        return false;
    }

    // Ensure directory exists
    ensureDirectory(CALIBRATION_FILE_PATH);

    File file = SD.open(CALIBRATION_FILE_PATH, FILE_WRITE);
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
    Serial.println("Calibration saved to SD card");
    return true;
}

bool Calibration::load() {
    if (!_sdAvailable) {
        return false;
    }

    if (!SD.exists(CALIBRATION_FILE_PATH)) {
        Serial.println("Calibration file not found on SD card");
        return false;
    }

    File file = SD.open(CALIBRATION_FILE_PATH, FILE_READ);
    if (!file) {
        Serial.println("Failed to open calibration file for reading");
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
