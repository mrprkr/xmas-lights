#include <Arduino.h>
#include "config.h"
#include "display.h"
#include "display_ui.h"
#include "calibration.h"
#include "led_controller.h"
#include "wifi_manager.h"
#include "web_server.h"

// Global instances
Calibration calibration;
LEDController ledController(calibration);
WiFiManager wifiManager;
WebServer* webServer = nullptr;

// Timing
unsigned long lastUIUpdate = 0;
unsigned long lastLEDUpdate = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("================================");
    Serial.println("  ESP32-S3 Christmas Lights");
    Serial.println("  WS2811 - 50 LEDs");
    Serial.println("  Touch LCD Control");
    Serial.println("================================");
    Serial.println();

    // Initialize display first (for status feedback)
    Serial.println("Initializing display...");
    if (!display.begin()) {
        Serial.println("CRITICAL: Display initialization failed!");
        // Continue anyway, web control still works
    }
    Serial.println("Display init done");

    // Initialize calibration (loads from SD card if available)
    Serial.println("Initializing calibration...");
    calibration.begin();

    // Initialize LED controller
    Serial.println("Initializing LEDs...");
    ledController.begin();

    // Startup animation
    for (int i = 0; i < NUM_LEDS; i++) {
        ledController.setPixelColor(i, CRGB::Green);
        FastLED.show();
        delay(10);
    }
    FastLED.clear();
    FastLED.show();

    // Initialize WiFi (non-blocking)
    Serial.println("Initializing WiFi...");
    wifiManager.begin();

    // Create UI after display is ready
    Serial.println("Creating UI...");
    displayUI = new DisplayUI(ledController, calibration, wifiManager);
    displayUI->begin();
    Serial.println("UI created");

    // Wait for WiFi connection
    Serial.print("Connecting to WiFi");
    int wifiAttempts = 0;
    while (!wifiManager.isConnected() && wifiAttempts < 200) {  // 10 second timeout
        wifiManager.update();
        display.update();  // Keep UI responsive
        delay(50);
        wifiAttempts++;
        if (wifiAttempts % 20 == 0) {
            Serial.print(".");
        }
    }
    Serial.println();
    Serial.printf("WiFi loop done after %d attempts\n", wifiAttempts);

    // Initialize web server
    Serial.println("Initializing web server...");
    webServer = new WebServer(ledController, calibration);
    webServer->begin();

    // Display connection info
    Serial.println();
    Serial.println("================================");
    Serial.print("  WiFi: ");
    Serial.println(wifiManager.getSSID());
    Serial.print("  IP:   http://");
    Serial.println(wifiManager.getIPAddress());
    Serial.println("================================");
    Serial.println();

    // Set initial state - festive red
    ledController.setSolidColor(CRGB::Red);
    ledController.setOn(true);

    // Sync UI with current state
    displayUI->syncState();

    Serial.println("Setup complete!");
}

void loop() {
    unsigned long now = millis();

    // Update WiFi connection status
    wifiManager.update();

    // Update display and UI (~60fps target)
    if (now - lastUIUpdate >= UI_UPDATE_INTERVAL) {
        display.update();
        displayUI->update();
        lastUIUpdate = now;
    }

    // Update LED animations (independent timing)
    ledController.update();

    // Small delay to prevent watchdog issues
    delay(1);
}
