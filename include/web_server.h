#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include "led_controller.h"
#include "calibration.h"
#include "config.h"

class WebServer {
public:
    WebServer(LEDController& ledController, Calibration& calibration);
    void begin();

private:
    AsyncWebServer _server;
    LEDController& _ledController;
    Calibration& _calibration;

    void setupRoutes();
    void handleGetState(AsyncWebServerRequest* request);
    void handleSetPower(AsyncWebServerRequest* request, JsonVariant& json);
    void handleSetBrightness(AsyncWebServerRequest* request, JsonVariant& json);
    void handleSetColor(AsyncWebServerRequest* request, JsonVariant& json);
    void handleSetAnimation(AsyncWebServerRequest* request, JsonVariant& json);
    void handleSetSpeed(AsyncWebServerRequest* request, JsonVariant& json);

    // Calibration endpoints
    void handleGetCalibration(AsyncWebServerRequest* request);
    void handleSetCalibrationMode(AsyncWebServerRequest* request, JsonVariant& json);
    void handleSetLEDPosition(AsyncWebServerRequest* request, JsonVariant& json);
    void handleSaveCalibration(AsyncWebServerRequest* request);
    void handleResetCalibration(AsyncWebServerRequest* request);

    String getStateJson();
    String getCalibrationJson();
};

#endif // WEB_SERVER_H
