#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include "config.h"

enum WiFiState {
    WIFI_STATE_DISCONNECTED,
    WIFI_STATE_CONNECTING,
    WIFI_STATE_CONNECTED,
    WIFI_STATE_AP_MODE
};

class WiFiManager {
public:
    WiFiManager();
    void begin();
    void update();

    WiFiState getState() const { return _state; }
    String getIPAddress() const;
    String getSSID() const;
    bool isConnected() const { return _state == WIFI_STATE_CONNECTED || _state == WIFI_STATE_AP_MODE; }

private:
    WiFiState _state;
    unsigned long _lastAttempt;
    uint8_t _retryCount;
    static const uint8_t MAX_RETRIES = 20;

    void startAPMode();
};

#endif // WIFI_MANAGER_H
