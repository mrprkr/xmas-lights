#include "wifi_manager.h"

WiFiManager::WiFiManager()
    : _state(WIFI_STATE_DISCONNECTED)
    , _lastAttempt(0)
    , _retryCount(0) {
}

void WiFiManager::begin() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    _state = WIFI_STATE_CONNECTING;
    _lastAttempt = millis();

    Serial.print("Connecting to WiFi");
}

void WiFiManager::update() {
    if (_state == WIFI_STATE_CONNECTING) {
        if (WiFi.status() == WL_CONNECTED) {
            _state = WIFI_STATE_CONNECTED;
            Serial.println();
            Serial.print("Connected! IP: ");
            Serial.println(WiFi.localIP());
            return;
        }

        if (millis() - _lastAttempt > 500) {
            _lastAttempt = millis();
            _retryCount++;
            Serial.print(".");

            if (_retryCount >= MAX_RETRIES) {
                Serial.println();
                Serial.println("WiFi connection failed, starting AP mode...");
                startAPMode();
            }
        }
    } else if (_state == WIFI_STATE_CONNECTED) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi disconnected, reconnecting...");
            _state = WIFI_STATE_CONNECTING;
            _retryCount = 0;
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        }
    }
}

void WiFiManager::startAPMode() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    _state = WIFI_STATE_AP_MODE;

    Serial.print("AP Mode started. Connect to: ");
    Serial.println(AP_SSID);
    Serial.print("IP: ");
    Serial.println(WiFi.softAPIP());
}

String WiFiManager::getIPAddress() const {
    if (_state == WIFI_STATE_AP_MODE) {
        return WiFi.softAPIP().toString();
    }
    return WiFi.localIP().toString();
}

String WiFiManager::getSSID() const {
    if (_state == WIFI_STATE_AP_MODE) {
        return String(AP_SSID);
    }
    return String(WIFI_SSID);
}
