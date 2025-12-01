#include <Arduino.h>
#include "config.h"
#include "calibration.h"
#include "led_controller.h"
#include "wifi_manager.h"
#include "web_server.h"

Calibration calibration;
LEDController ledController(calibration);
WiFiManager wifiManager;
WebServer* webServer;

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("================================");
    Serial.println("  ESP32 Christmas Lights");
    Serial.println("  WS2811 - 50 LEDs");
    Serial.println("================================");
    Serial.println();

    // Initialize calibration (loads from SPIFFS if available)
    Serial.println("Initializing calibration...");
    calibration.begin();

    // Initialize LED controller
    Serial.println("Initializing LEDs...");
    ledController.begin();

    // Show startup animation
    for (int i = 0; i < NUM_LEDS; i++) {
        ledController.setPixelColor(i, CRGB::Red);
        FastLED.show();
        delay(20);
    }
    delay(200);
    FastLED.clear();
    FastLED.show();

    // Initialize WiFi
    Serial.println("Initializing WiFi...");
    wifiManager.begin();

    // Wait for WiFi connection
    while (!wifiManager.isConnected()) {
        wifiManager.update();
        delay(100);
    }

    // Initialize web server
    Serial.println("Initializing web server...");
    webServer = new WebServer(ledController, calibration);
    webServer->begin();

    Serial.println();
    Serial.println("================================");
    Serial.print("  Access at: http://");
    Serial.println(wifiManager.getIPAddress());
    Serial.println("================================");
    Serial.println();

    // Set initial state - festive red
    ledController.setSolidColor(CRGB::Red);
}

void loop() {
    // Update WiFi connection status
    wifiManager.update();

    // Update LED animations
    ledController.update();

    // Small delay to prevent watchdog issues
    delay(1);
}
