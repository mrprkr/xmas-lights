#include "web_server.h"

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Christmas Lights Control</title>
    <style>
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%);
            min-height: 100vh;
            color: #fff;
            padding: 20px;
        }
        .container { max-width: 500px; margin: 0 auto; }
        h1 {
            text-align: center;
            margin-bottom: 20px;
            font-size: 1.8em;
            text-shadow: 0 0 20px rgba(255,100,100,0.5);
        }
        .tabs {
            display: flex;
            margin-bottom: 20px;
            background: rgba(255,255,255,0.1);
            border-radius: 12px;
            padding: 4px;
        }
        .tab {
            flex: 1;
            padding: 12px;
            border: none;
            background: transparent;
            color: #aaa;
            cursor: pointer;
            border-radius: 8px;
            font-size: 0.95em;
            transition: all 0.2s;
        }
        .tab.active {
            background: rgba(255,255,255,0.2);
            color: #fff;
        }
        .tab-content { display: none; }
        .tab-content.active { display: block; }
        .card {
            background: rgba(255,255,255,0.1);
            border-radius: 16px;
            padding: 20px;
            margin-bottom: 20px;
            backdrop-filter: blur(10px);
        }
        .card-title {
            font-size: 0.9em;
            text-transform: uppercase;
            letter-spacing: 1px;
            color: #aaa;
            margin-bottom: 15px;
        }
        .power-btn {
            width: 100%;
            padding: 20px;
            font-size: 1.2em;
            border: none;
            border-radius: 12px;
            cursor: pointer;
            transition: all 0.3s;
        }
        .power-btn.on {
            background: linear-gradient(135deg, #00b894, #00cec9);
            color: white;
        }
        .power-btn.off {
            background: linear-gradient(135deg, #636e72, #2d3436);
            color: #aaa;
        }
        .power-btn:active { transform: scale(0.98); }
        .slider-container { margin: 15px 0; }
        .slider-label {
            display: flex;
            justify-content: space-between;
            margin-bottom: 8px;
        }
        input[type="range"] {
            width: 100%;
            height: 8px;
            border-radius: 4px;
            background: #444;
            outline: none;
            -webkit-appearance: none;
        }
        input[type="range"]::-webkit-slider-thumb {
            -webkit-appearance: none;
            width: 24px;
            height: 24px;
            border-radius: 50%;
            background: #fff;
            cursor: pointer;
            box-shadow: 0 2px 10px rgba(0,0,0,0.3);
        }
        .color-picker {
            width: 100%;
            height: 50px;
            border: none;
            border-radius: 8px;
            cursor: pointer;
        }
        .animation-grid {
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            gap: 10px;
        }
        .anim-btn {
            padding: 12px 8px;
            border: 2px solid transparent;
            border-radius: 8px;
            background: rgba(255,255,255,0.1);
            color: #fff;
            cursor: pointer;
            font-size: 0.8em;
            transition: all 0.2s;
        }
        .anim-btn:hover { background: rgba(255,255,255,0.2); }
        .anim-btn.active {
            border-color: #00b894;
            background: rgba(0,184,148,0.2);
        }
        .anim-btn.spatial {
            border-color: #6c5ce7;
        }
        .anim-btn.spatial.active {
            border-color: #a29bfe;
            background: rgba(108,92,231,0.3);
        }
        .status {
            text-align: center;
            font-size: 0.8em;
            color: #888;
            margin-top: 20px;
        }
        .status.connected { color: #00b894; }
        .status.error { color: #ff7675; }

        /* Calibration styles */
        .calibration-nav {
            display: flex;
            gap: 10px;
            margin-bottom: 15px;
        }
        .calibration-nav button {
            flex: 1;
            padding: 12px;
            border: none;
            border-radius: 8px;
            background: rgba(255,255,255,0.15);
            color: #fff;
            cursor: pointer;
            font-size: 1em;
        }
        .calibration-nav button:disabled {
            opacity: 0.3;
            cursor: not-allowed;
        }
        .calibration-nav button:not(:disabled):hover {
            background: rgba(255,255,255,0.25);
        }
        .led-indicator {
            text-align: center;
            font-size: 3em;
            font-weight: bold;
            padding: 20px;
            background: rgba(255,255,255,0.1);
            border-radius: 12px;
            margin-bottom: 15px;
        }
        .led-indicator small {
            display: block;
            font-size: 0.3em;
            color: #888;
            margin-top: 5px;
        }
        .position-controls {
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            gap: 15px;
        }
        .axis-control {
            text-align: center;
        }
        .axis-control label {
            display: block;
            font-size: 1.2em;
            font-weight: bold;
            margin-bottom: 8px;
        }
        .axis-control label.x { color: #ff6b6b; }
        .axis-control label.y { color: #51cf66; }
        .axis-control label.z { color: #339af0; }
        .axis-control input[type="range"] {
            writing-mode: vertical-lr;
            direction: rtl;
            height: 120px;
            width: 24px;
        }
        .axis-control .value {
            margin-top: 8px;
            font-size: 0.9em;
            color: #aaa;
        }
        .calibration-actions {
            display: flex;
            gap: 10px;
            margin-top: 15px;
        }
        .calibration-actions button {
            flex: 1;
            padding: 12px;
            border: none;
            border-radius: 8px;
            cursor: pointer;
            font-size: 0.9em;
        }
        .btn-save {
            background: linear-gradient(135deg, #00b894, #00cec9);
            color: white;
        }
        .btn-reset {
            background: rgba(255,255,255,0.15);
            color: #fff;
        }
        .btn-exit {
            background: linear-gradient(135deg, #636e72, #2d3436);
            color: #fff;
        }
        .section-divider {
            border-top: 1px solid rgba(255,255,255,0.1);
            margin: 15px 0;
            padding-top: 15px;
        }
        .section-label {
            font-size: 0.75em;
            color: #666;
            margin-bottom: 10px;
            text-transform: uppercase;
            letter-spacing: 1px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Christmas Lights</h1>

        <div class="tabs">
            <button class="tab active" onclick="showTab('control')">Control</button>
            <button class="tab" onclick="showTab('calibrate')">Calibrate</button>
        </div>

        <!-- Control Tab -->
        <div id="control-tab" class="tab-content active">
            <div class="card">
                <button id="powerBtn" class="power-btn on" onclick="togglePower()">ON</button>
            </div>

            <div class="card">
                <div class="card-title">Brightness</div>
                <div class="slider-container">
                    <div class="slider-label">
                        <span>Dim</span>
                        <span id="brightnessVal">50%</span>
                    </div>
                    <input type="range" id="brightness" min="0" max="255" value="128" onchange="setBrightness(this.value)">
                </div>
            </div>

            <div class="card">
                <div class="card-title">Color</div>
                <input type="color" id="colorPicker" class="color-picker" value="#ff0000" onchange="setColor(this.value)">
            </div>

            <div class="card">
                <div class="card-title">Basic Animations</div>
                <div class="animation-grid">
                    <button class="anim-btn active" data-anim="0" onclick="setAnimation(0)">Static</button>
                    <button class="anim-btn" data-anim="1" onclick="setAnimation(1)">Rainbow</button>
                    <button class="anim-btn" data-anim="2" onclick="setAnimation(2)">Chase</button>
                    <button class="anim-btn" data-anim="3" onclick="setAnimation(3)">Twinkle</button>
                    <button class="anim-btn" data-anim="4" onclick="setAnimation(4)">Fade</button>
                    <button class="anim-btn" data-anim="5" onclick="setAnimation(5)">Sparkle</button>
                    <button class="anim-btn" data-anim="6" onclick="setAnimation(6)">Candy Cane</button>
                    <button class="anim-btn" data-anim="7" onclick="setAnimation(7)">Snow</button>
                    <button class="anim-btn" data-anim="8" onclick="setAnimation(8)">Fire</button>
                </div>

                <div class="section-divider">
                    <div class="section-label">3D Spatial Animations</div>
                </div>
                <div class="animation-grid">
                    <button class="anim-btn spatial" data-anim="9" onclick="setAnimation(9)">Wave</button>
                    <button class="anim-btn spatial" data-anim="10" onclick="setAnimation(10)">3D Rainbow</button>
                    <button class="anim-btn spatial" data-anim="11" onclick="setAnimation(11)">Pulse</button>
                    <button class="anim-btn spatial" data-anim="12" onclick="setAnimation(12)">Rotate</button>
                    <button class="anim-btn spatial" data-anim="13" onclick="setAnimation(13)">Planes</button>
                </div>
            </div>

            <div class="card">
                <div class="card-title">Animation Speed</div>
                <div class="slider-container">
                    <div class="slider-label">
                        <span>Fast</span>
                        <span id="speedVal">50ms</span>
                    </div>
                    <input type="range" id="speed" min="10" max="200" value="50" onchange="setSpeed(this.value)">
                </div>
            </div>
        </div>

        <!-- Calibration Tab -->
        <div id="calibrate-tab" class="tab-content">
            <div class="card">
                <div class="card-title">LED Position Calibration</div>
                <p style="color:#aaa;font-size:0.85em;margin-bottom:15px;">
                    Set the 3D position of each LED for spatially-aware animations.
                    The current LED will light up white, with red (previous) and green (next) neighbors.
                </p>

                <div class="led-indicator">
                    <span id="currentLed">-</span>
                    <small>of 50 LEDs</small>
                </div>

                <div class="calibration-nav">
                    <button onclick="calibratePrev()" id="prevBtn">← Prev</button>
                    <button onclick="calibrateStart()" id="startBtn">Start</button>
                    <button onclick="calibrateNext()" id="nextBtn">Next →</button>
                </div>

                <div id="positionControls" style="display:none;">
                    <div class="position-controls">
                        <div class="axis-control">
                            <label class="x">X</label>
                            <input type="range" id="posX" min="-100" max="100" value="0" oninput="updatePosition()">
                            <div class="value" id="posXVal">0.00</div>
                        </div>
                        <div class="axis-control">
                            <label class="y">Y</label>
                            <input type="range" id="posY" min="-100" max="100" value="0" oninput="updatePosition()">
                            <div class="value" id="posYVal">0.00</div>
                        </div>
                        <div class="axis-control">
                            <label class="z">Z</label>
                            <input type="range" id="posZ" min="-100" max="100" value="0" oninput="updatePosition()">
                            <div class="value" id="posZVal">0.00</div>
                        </div>
                    </div>

                    <div class="calibration-actions">
                        <button class="btn-save" onclick="saveCalibration()">Save</button>
                        <button class="btn-reset" onclick="resetCalibration()">Reset All</button>
                        <button class="btn-exit" onclick="exitCalibration()">Exit</button>
                    </div>
                </div>
            </div>
        </div>

        <div id="status" class="status connected">Connected</div>
    </div>

    <script>
        const API_BASE = '';
        let calibrationData = [];
        let currentCalibrationLed = -1;

        async function api(endpoint, data = null) {
            try {
                const opts = data ? {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify(data)
                } : { method: 'GET' };
                const res = await fetch(API_BASE + endpoint, opts);
                if (!res.ok) throw new Error('Request failed');
                document.getElementById('status').className = 'status connected';
                document.getElementById('status').textContent = 'Connected';
                return await res.json();
            } catch (e) {
                document.getElementById('status').className = 'status error';
                document.getElementById('status').textContent = 'Connection error';
                throw e;
            }
        }

        function showTab(tab) {
            document.querySelectorAll('.tab').forEach(t => t.classList.remove('active'));
            document.querySelectorAll('.tab-content').forEach(t => t.classList.remove('active'));
            document.querySelector(`.tab-content#${tab}-tab`).classList.add('active');
            document.querySelectorAll('.tab')[tab === 'control' ? 0 : 1].classList.add('active');

            if (tab === 'calibrate') {
                loadCalibration();
            } else {
                exitCalibration();
            }
        }

        async function loadState() {
            try {
                const state = await api('/api/state');
                updateUI(state);
            } catch (e) {
                console.error('Failed to load state:', e);
            }
        }

        function updateUI(state) {
            const powerBtn = document.getElementById('powerBtn');
            powerBtn.className = 'power-btn ' + (state.on ? 'on' : 'off');
            powerBtn.textContent = state.on ? 'ON' : 'OFF';

            document.getElementById('brightness').value = state.brightness;
            document.getElementById('brightnessVal').textContent = Math.round(state.brightness / 255 * 100) + '%';

            const hex = '#' + state.color.r.toString(16).padStart(2, '0') +
                              state.color.g.toString(16).padStart(2, '0') +
                              state.color.b.toString(16).padStart(2, '0');
            document.getElementById('colorPicker').value = hex;

            document.querySelectorAll('.anim-btn').forEach(btn => {
                btn.classList.toggle('active', parseInt(btn.dataset.anim) === state.animation);
            });

            document.getElementById('speed').value = state.speed;
            document.getElementById('speedVal').textContent = state.speed + 'ms';
        }

        async function togglePower() {
            const btn = document.getElementById('powerBtn');
            const isOn = btn.classList.contains('on');
            await api('/api/power', { on: !isOn });
            btn.className = 'power-btn ' + (!isOn ? 'on' : 'off');
            btn.textContent = !isOn ? 'ON' : 'OFF';
        }

        async function setBrightness(val) {
            document.getElementById('brightnessVal').textContent = Math.round(val / 255 * 100) + '%';
            await api('/api/brightness', { brightness: parseInt(val) });
        }

        async function setColor(hex) {
            const r = parseInt(hex.slice(1, 3), 16);
            const g = parseInt(hex.slice(3, 5), 16);
            const b = parseInt(hex.slice(5, 7), 16);
            await api('/api/color', { r, g, b });
        }

        async function setAnimation(mode) {
            document.querySelectorAll('.anim-btn').forEach(btn => {
                btn.classList.toggle('active', parseInt(btn.dataset.anim) === mode);
            });
            await api('/api/animation', { mode });
        }

        async function setSpeed(val) {
            document.getElementById('speedVal').textContent = val + 'ms';
            await api('/api/speed', { speed: parseInt(val) });
        }

        // Calibration functions
        async function loadCalibration() {
            try {
                const data = await api('/api/calibration');
                calibrationData = data.positions;
            } catch (e) {
                console.error('Failed to load calibration:', e);
            }
        }

        async function calibrateStart() {
            currentCalibrationLed = 0;
            await setCalibrationMode(0);
            updateCalibrationUI();
        }

        async function calibratePrev() {
            if (currentCalibrationLed > 0) {
                currentCalibrationLed--;
                await setCalibrationMode(currentCalibrationLed);
                updateCalibrationUI();
            }
        }

        async function calibrateNext() {
            if (currentCalibrationLed < 49) {
                currentCalibrationLed++;
                await setCalibrationMode(currentCalibrationLed);
                updateCalibrationUI();
            }
        }

        async function setCalibrationMode(led) {
            await api('/api/calibration/mode', { led });
        }

        function updateCalibrationUI() {
            document.getElementById('currentLed').textContent = currentCalibrationLed + 1;
            document.getElementById('prevBtn').disabled = currentCalibrationLed <= 0;
            document.getElementById('nextBtn').disabled = currentCalibrationLed >= 49;
            document.getElementById('startBtn').style.display = 'none';
            document.getElementById('positionControls').style.display = 'block';

            if (calibrationData[currentCalibrationLed]) {
                const pos = calibrationData[currentCalibrationLed];
                document.getElementById('posX').value = pos.x * 100;
                document.getElementById('posY').value = pos.y * 100;
                document.getElementById('posZ').value = pos.z * 100;
                document.getElementById('posXVal').textContent = pos.x.toFixed(2);
                document.getElementById('posYVal').textContent = pos.y.toFixed(2);
                document.getElementById('posZVal').textContent = pos.z.toFixed(2);
            }
        }

        async function updatePosition() {
            const x = parseInt(document.getElementById('posX').value) / 100;
            const y = parseInt(document.getElementById('posY').value) / 100;
            const z = parseInt(document.getElementById('posZ').value) / 100;

            document.getElementById('posXVal').textContent = x.toFixed(2);
            document.getElementById('posYVal').textContent = y.toFixed(2);
            document.getElementById('posZVal').textContent = z.toFixed(2);

            calibrationData[currentCalibrationLed] = { x, y, z };

            await api('/api/calibration/position', {
                led: currentCalibrationLed,
                x, y, z
            });
        }

        async function saveCalibration() {
            try {
                await api('/api/calibration/save', {});
                alert('Calibration saved!');
            } catch (e) {
                alert('Failed to save calibration');
            }
        }

        async function resetCalibration() {
            if (confirm('Reset all LED positions to default linear layout?')) {
                await api('/api/calibration/reset', {});
                await loadCalibration();
                if (currentCalibrationLed >= 0) {
                    updateCalibrationUI();
                }
            }
        }

        async function exitCalibration() {
            currentCalibrationLed = -1;
            await api('/api/calibration/mode', { led: -1 });
            document.getElementById('currentLed').textContent = '-';
            document.getElementById('startBtn').style.display = 'block';
            document.getElementById('positionControls').style.display = 'none';
            document.getElementById('prevBtn').disabled = true;
            document.getElementById('nextBtn').disabled = true;
        }

        // Load initial state
        loadState();
    </script>
</body>
</html>
)rawliteral";

WebServer::WebServer(LEDController& ledController, Calibration& calibration)
    : _server(WEB_SERVER_PORT)
    , _ledController(ledController)
    , _calibration(calibration) {
}

void WebServer::begin() {
    setupRoutes();
    _server.begin();
    Serial.println("Web server started on port 80");
}

void WebServer::setupRoutes() {
    // Serve main page
    _server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send_P(200, "text/html", INDEX_HTML);
    });

    // Get current state
    _server.on("/api/state", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleGetState(request);
    });

    // Set power
    AsyncCallbackJsonWebHandler* powerHandler = new AsyncCallbackJsonWebHandler("/api/power",
        [this](AsyncWebServerRequest* request, JsonVariant& json) {
            handleSetPower(request, json);
        });
    _server.addHandler(powerHandler);

    // Set brightness
    AsyncCallbackJsonWebHandler* brightnessHandler = new AsyncCallbackJsonWebHandler("/api/brightness",
        [this](AsyncWebServerRequest* request, JsonVariant& json) {
            handleSetBrightness(request, json);
        });
    _server.addHandler(brightnessHandler);

    // Set color
    AsyncCallbackJsonWebHandler* colorHandler = new AsyncCallbackJsonWebHandler("/api/color",
        [this](AsyncWebServerRequest* request, JsonVariant& json) {
            handleSetColor(request, json);
        });
    _server.addHandler(colorHandler);

    // Set animation
    AsyncCallbackJsonWebHandler* animationHandler = new AsyncCallbackJsonWebHandler("/api/animation",
        [this](AsyncWebServerRequest* request, JsonVariant& json) {
            handleSetAnimation(request, json);
        });
    _server.addHandler(animationHandler);

    // Set speed
    AsyncCallbackJsonWebHandler* speedHandler = new AsyncCallbackJsonWebHandler("/api/speed",
        [this](AsyncWebServerRequest* request, JsonVariant& json) {
            handleSetSpeed(request, json);
        });
    _server.addHandler(speedHandler);

    // Calibration endpoints
    _server.on("/api/calibration", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleGetCalibration(request);
    });

    AsyncCallbackJsonWebHandler* calibModeHandler = new AsyncCallbackJsonWebHandler("/api/calibration/mode",
        [this](AsyncWebServerRequest* request, JsonVariant& json) {
            handleSetCalibrationMode(request, json);
        });
    _server.addHandler(calibModeHandler);

    AsyncCallbackJsonWebHandler* calibPosHandler = new AsyncCallbackJsonWebHandler("/api/calibration/position",
        [this](AsyncWebServerRequest* request, JsonVariant& json) {
            handleSetLEDPosition(request, json);
        });
    _server.addHandler(calibPosHandler);

    AsyncCallbackJsonWebHandler* calibSaveHandler = new AsyncCallbackJsonWebHandler("/api/calibration/save",
        [this](AsyncWebServerRequest* request, JsonVariant& json) {
            handleSaveCalibration(request);
        });
    _server.addHandler(calibSaveHandler);

    AsyncCallbackJsonWebHandler* calibResetHandler = new AsyncCallbackJsonWebHandler("/api/calibration/reset",
        [this](AsyncWebServerRequest* request, JsonVariant& json) {
            handleResetCalibration(request);
        });
    _server.addHandler(calibResetHandler);
}

void WebServer::handleGetState(AsyncWebServerRequest* request) {
    request->send(200, "application/json", getStateJson());
}

void WebServer::handleSetPower(AsyncWebServerRequest* request, JsonVariant& json) {
    JsonObject obj = json.as<JsonObject>();
    if (obj.containsKey("on")) {
        _ledController.setOn(obj["on"].as<bool>());
    }
    request->send(200, "application/json", getStateJson());
}

void WebServer::handleSetBrightness(AsyncWebServerRequest* request, JsonVariant& json) {
    JsonObject obj = json.as<JsonObject>();
    if (obj.containsKey("brightness")) {
        _ledController.setBrightness(obj["brightness"].as<uint8_t>());
    }
    request->send(200, "application/json", getStateJson());
}

void WebServer::handleSetColor(AsyncWebServerRequest* request, JsonVariant& json) {
    JsonObject obj = json.as<JsonObject>();
    if (obj.containsKey("r") && obj.containsKey("g") && obj.containsKey("b")) {
        CRGB color(obj["r"].as<uint8_t>(), obj["g"].as<uint8_t>(), obj["b"].as<uint8_t>());
        _ledController.setSolidColor(color);
    }
    request->send(200, "application/json", getStateJson());
}

void WebServer::handleSetAnimation(AsyncWebServerRequest* request, JsonVariant& json) {
    JsonObject obj = json.as<JsonObject>();
    if (obj.containsKey("mode")) {
        _ledController.setAnimation(static_cast<AnimationMode>(obj["mode"].as<int>()));
    }
    request->send(200, "application/json", getStateJson());
}

void WebServer::handleSetSpeed(AsyncWebServerRequest* request, JsonVariant& json) {
    JsonObject obj = json.as<JsonObject>();
    if (obj.containsKey("speed")) {
        _ledController.setAnimationSpeed(obj["speed"].as<uint16_t>());
    }
    request->send(200, "application/json", getStateJson());
}

void WebServer::handleGetCalibration(AsyncWebServerRequest* request) {
    request->send(200, "application/json", getCalibrationJson());
}

void WebServer::handleSetCalibrationMode(AsyncWebServerRequest* request, JsonVariant& json) {
    JsonObject obj = json.as<JsonObject>();
    if (obj.containsKey("led")) {
        int16_t led = obj["led"].as<int16_t>();
        _calibration.setCalibrationLED(led);
    }
    request->send(200, "application/json", "{\"ok\":true}");
}

void WebServer::handleSetLEDPosition(AsyncWebServerRequest* request, JsonVariant& json) {
    JsonObject obj = json.as<JsonObject>();
    if (obj.containsKey("led") && obj.containsKey("x") && obj.containsKey("y") && obj.containsKey("z")) {
        uint16_t led = obj["led"].as<uint16_t>();
        float x = obj["x"].as<float>();
        float y = obj["y"].as<float>();
        float z = obj["z"].as<float>();
        _calibration.setPosition(led, x, y, z);
    }
    request->send(200, "application/json", "{\"ok\":true}");
}

void WebServer::handleSaveCalibration(AsyncWebServerRequest* request) {
    bool success = _calibration.save();
    if (success) {
        request->send(200, "application/json", "{\"ok\":true}");
    } else {
        request->send(500, "application/json", "{\"ok\":false,\"error\":\"Failed to save\"}");
    }
}

void WebServer::handleResetCalibration(AsyncWebServerRequest* request) {
    _calibration.resetToLinear();
    request->send(200, "application/json", getCalibrationJson());
}

String WebServer::getStateJson() {
    JsonDocument doc;
    doc["on"] = _ledController.isOn();
    doc["brightness"] = _ledController.getBrightness();

    CRGB color = _ledController.getSolidColor();
    doc["color"]["r"] = color.r;
    doc["color"]["g"] = color.g;
    doc["color"]["b"] = color.b;

    doc["animation"] = static_cast<int>(_ledController.getAnimation());
    doc["speed"] = _ledController.getAnimationSpeed();

    String output;
    serializeJson(doc, output);
    return output;
}

String WebServer::getCalibrationJson() {
    JsonDocument doc;
    JsonArray positions = doc["positions"].to<JsonArray>();

    for (uint16_t i = 0; i < NUM_LEDS; i++) {
        LEDPosition pos = _calibration.getPosition(i);
        JsonObject p = positions.add<JsonObject>();
        p["x"] = pos.x;
        p["y"] = pos.y;
        p["z"] = pos.z;
    }

    doc["calibrating"] = _calibration.isCalibrating();
    doc["currentLed"] = _calibration.getCalibrationLED();

    String output;
    serializeJson(doc, output);
    return output;
}
