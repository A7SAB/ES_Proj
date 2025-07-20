#include <WiFi.h>
#include <time.h>
#include <U8g2lib.h>
#include <Wire.h>
// #include <EEPROM.h>

// <<< WEB SERVER >>> Include necessary libraries
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "FS.h" // Optional: If storing HTML/CSS/JS in SPIFFS/LittleFS

#include <PZEM004Tv30.h>

#define BUTTON_PREV 5
#define BUTTON_SELECT 4
#define BUTTON_NEXT 15
#define SDA_PIN 21
#define SCL_PIN 22
#define RELAY_PIN 23
#define LED 19

// <<< PZEM-004T >>> Define Serial pins (Default for Serial2 on many ESP32 boards)
#define PZEM_RX_PIN 16 // Connect to TX pin of PZEM
#define PZEM_TX_PIN 17 // Connect to RX pin of PZEM

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Constructor for I2C 128x64 OLED
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

// <<< PZEM-004T >>> Initialize PZEM object using Serial2
PZEM004Tv30 pzem(&Serial2, PZEM_RX_PIN, PZEM_TX_PIN);

// --- Bitmaps ---
static const unsigned char image_calender_bits[] = {0x00,0x00,0x90,0x04,0xfe,0x3f,0x93,0x64,0x01,0x40,0x31,0x44,0x49,0x46,0x49,0x45,0x41,0x44,0x21,0x44,0x11,0x44,0x09,0x44,0x79,0x4f,0x03,0x60,0xfe,0x3f,0x00,0x00};
static const unsigned char image_menu_settings_gear_bits[] = {0xc0,0x03,0x48,0x12,0x34,0x2c,0x02,0x40,0xc4,0x23,0x24,0x24,0x13,0xc8,0x11,0x88,0x11,0x88,0x13,0xc8,0x24,0x24,0xc4,0x23,0x02,0x40,0x34,0x2c,0x48,0x12,0xc0,0x03};
static const unsigned char image_network_www_bits[] = {0xc0,0x03,0xb0,0x0d,0x4c,0x32,0x24,0x24,0x22,0x44,0xfe,0x7f,0x11,0x88,0x11,0x88,0x11,0x88,0x11,0x88,0xfe,0x7f,0x22,0x44,0x24,0x24,0x4c,0x32,0xb0,0x0d,0xc0,0x03};
static const unsigned char image_selection[] = {0x07,0x80,0x03,0x01,0x00,0x02,0x01,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x02,0x01,0x00,0x02,0x07,0x80,0x03};
static const unsigned char image_Lightining_logo_bits[] = {0x80,0x00,0x40,0x00,0x20,0x00,0x10,0x00,0x78,0x00,0x3c,0x00,0x10,0x00,0x08,0x00,0x04,0x00,0x02,0x00};
static const unsigned char image_On_logo_bits[] = {0xfc,0xff,0x01,0xfe,0xff,0x03,0xff,0xff,0x07,0xff,0xfd,0x07,0xff,0xfd,0x07,0xbf,0xed,0x07,0xdf,0xdd,0x07,0xef,0xbd,0x07,0xef,0xbd,0x07,0xef,0xbd,0x07,0xef,0xbf,0x07,0xef,0xbf,0x07,0xdf,0xdf,0x07,0xbf,0xef,0x07,0x7f,0xf0,0x07,0xff,0xff,0x07,0xff,0xff,0x07,0xff,0xff,0x07,0xfe,0xff,0x03,0xfc,0xff,0x01};
static const unsigned char image_off_logo_bits[] = {0xfc,0xff,0x01,0x02,0x00,0x02,0x01,0x00,0x04,0x01,0x02,0x04,0x01,0x02,0x04,0x41,0x12,0x04,0x21,0x22,0x04,0x11,0x42,0x04,0x11,0x42,0x04,0x11,0x42,0x04,0x11,0x40,0x04,0x11,0x40,0x04,0x21,0x20,0x04,0x41,0x10,0x04,0x81,0x0f,0x04,0x01,0x00,0x04,0x01,0x00,0x04,0x03,0x00,0x06,0xfe,0xff,0x03,0xfc,0xff,0x01};
static const unsigned char image_menu_settings_sliders_bits[] = {0x1c,0x00,0x22,0x00,0xe3,0x3f,0x22,0x00,0x1c,0x00,0x00,0x0e,0x00,0x11,0xff,0x31,0x00,0x11,0x00,0x0e,0x1c,0x00,0x22,0x00,0xe3,0x3f,0x22,0x00,0x1c,0x00,0x00,0x00}; // [NEW]


// --- Menu Setup ---
const unsigned char* icons[] = { image_menu_settings_gear_bits, image_network_www_bits, image_calender_bits, image_menu_settings_sliders_bits }; 
const char* menuLabels[] = { "SHOW READINGS", "WIFI INFO", "SCHEDULE", "RELAY CTRL" }; 
const int iconWidths[] = {16, 16, 15, 16}; //  icon for sliders is 16px wide
const int iconHeights[] = {16, 16, 16, 16}; // icon for sliders is 16px high
const int menuItemCount = sizeof(icons) / sizeof(icons[0]);
int currentIndex = 0;
// --- End Menu Setup ---


// --- State Management  ---
enum DisplayState { MENU, SHOW_READINGS, WIFI_INFO, SCHEDULE, SET_TIME, RELAY_CONTROL, EDIT_CURRENT_LIMIT }; 
DisplayState currentState = MENU;
enum ScheduleSubState { SELECT_ENABLED, SELECT_ON_TIME, SELECT_OFF_TIME };
ScheduleSubState currentScheduleSubState = SELECT_ENABLED;
enum SetTimeSubState { EDIT_HOUR, EDIT_MINUTE, EDIT_AMPM, SAVE_EXIT };
SetTimeSubState currentSetTimeSubState = EDIT_HOUR;

// Relay Control Page
enum RelayControlSubState { SELECT_MANUAL_STATE, SELECT_LIMIT_SETTING };
RelayControlSubState currentRelayControlSubState = SELECT_MANUAL_STATE;

// Current Limit Page
enum EditCurrentLimitSubState { ADJUST_LIMIT_VALUE, CONFIRM_LIMIT_VALUE };
EditCurrentLimitSubState currentEditLimitSubState = ADJUST_LIMIT_VALUE;
// --- End State Management ---

// --- Sensor Reading Variables ---
float sensorEnergy = 0 ;
float sensorVoltage = 0;
float sensorCurrent= 0;
float sensorPowerFactor= 0;
float sensorPower = 0;
unsigned long lastSensorReadTime = 0; // Timer for sensor reads

// --- Schedule Variables (shared between OLED and Web) ---
bool scheduleEnabled = false;
int scheduleOnHour = 10; int scheduleOnMinute = 0; bool scheduleOnIsPM = false;
int scheduleOffHour = 9; int scheduleOffMinute = 0; bool scheduleOffIsPM = true;

//-- Deafult control Variables ---
bool manualControlEngaged = false;      // True if manual relay control mode is active
bool manualRelayActualState = false;    // Desired relay state in manual mode (false=OFF/HIGH, true=ON/LOW)
float currentTripPoint = 5.0;          // Current limit in Amps (e.g., 5.0A)
float temporaryCurrentTripPoint;       // For editing the current limit
bool currentLimitFeatureEnabled = true; // Enable/disable the current limit check feature (default true)
unsigned long lastCurrentLimitCheckTime = 0; // Timer for current limit check message cooldown

// Temporary variables for OLED editing
int tempHour; int tempMinute; bool tempIsPM; bool isSettingOnTime;
// --- End Schedule Variables ---

unsigned long lastDebounceTime = 0;
unsigned long lastRelayCheckTime = 0;
unsigned long lastWebDataUpdateTime = 0;

// <<< WEB SERVER >>> Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// --- Function Prototypes ---
void drawRotatingMenu(int selected);
void drawShowReadingsPage();
void drawWifiInfoPage();
void drawSchedulePage();
void drawSetTimePage();
void handleButtons();
void drawPersistentElements();
void checkScheduleAndControlRelay();
void drawRelayControlPage();      
void drawEditCurrentLimitPage();  
void readSensors(); 

// Web Handlers
void handleRoot(AsyncWebServerRequest *request);
void handleGetData(AsyncWebServerRequest *request);
void handleRelayControl(AsyncWebServerRequest *request);
void handleScheduleUpdate(AsyncWebServerRequest *request);
void handleNotFound(AsyncWebServerRequest *request);
String processor(const String& var);

// --- HTML, CSS, JS content (as raw string literals - unchanged) ---
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Relay Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial, sans-serif; margin: 0; padding: 15px; background-color: #f4f4f4; color: #333; }
    .container { max-width: 600px; margin: auto; background: #fff; padding: 20px; border-radius: 8px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }
    h1 { color: #0056b3; text-align: center; }
    h2 { color: #0056b3; border-bottom: 1px solid #eee; padding-bottom: 5px; margin-top: 25px; }
    .section { margin-bottom: 20px; padding: 15px; background-color: #e9f5ff; border-radius: 5px; }
    .reading, .relay-status, .schedule-display { margin-bottom: 10px; }
    label { font-weight: bold; display: inline-block; min-width: 100px; }
    input[type=time], input[type=number], select { padding: 8px; margin-left: 10px; border: 1px solid #ccc; border-radius: 4px; }
    input[type=checkbox] { margin-left: 10px; transform: scale(1.2); }
    button, input[type=submit] { background-color: #007bff; color: white; padding: 10px 15px; border: none; border-radius: 4px; cursor: pointer; font-size: 1em; margin: 5px; }
    button:hover, input[type=submit]:hover { background-color: #0056b3; }
    .relay-buttons button { width: 80px; }
    .button-on { background-color: #28a745; }
    .button-on:hover { background-color: #218838; }
    .button-off { background-color: #dc3545; }
    .button-off:hover { background-color: #c82333; }
    .schedule-form div { margin-bottom: 10px; }
    #relayState { font-weight: bold; }
    .current-time { text-align: center; font-size: 0.9em; color: #555; margin-bottom: 15px; }
  </style>
</head>
<body>
  <div class="container">
    <h1>ESP32 Control Panel</h1>
    <div class="current-time">Current Time: <span id="currentTime">--:--:--</span></div>

    <div class="section">
      <h2>Sensor Readings</h2>
      <div class="reading"><span>Energy:</span> <span id="energy">--</span> W</div>
      <div class="reading"><span>Voltage:</span> <span id="voltage">--</span> V</div>
      <div class="reading"><span>Current:</span> <span id="current">--</span> A</div>
      <div class="reading"><span>Power Factor:</span> <span id="pf">--</span></div>
    </div>

    <div class="section">
      <h2>Relay Control</h2>
      <div class="relay-status">Current State: <span id="relayState">UNKNOWN</span></div>
      <div class="relay-buttons">
        <button class="button-on" onclick="controlRelay('on')">ON</button>
        <button class="button-off" onclick="controlRelay('off')">OFF</button>
      </div>
    </div>

    <div class="section">
      <h2>Schedule Settings</h2>
      <form id="scheduleForm" onsubmit="updateSchedule(event)">
          <div class="schedule-form">
            <label for="scheduleEnabled">Enable Schedule:</label>
            <input type="checkbox" id="scheduleEnabled" name="scheduleEnabled">
          </div>
          <div class="schedule-form">
            <label for="onTime">ON Time:</label>
            <input type="time" id="onTime" name="onTime" step="60"> </div>
          <div class="schedule-form">
            <label for="offTime">OFF Time:</label>
            <input type="time" id="offTime" name="offTime" step="60">
          </div>
          <div>
            <input type="submit" value="Update Schedule">
          </div>
      </form>
    </div>

  </div>

  <script>
    function controlRelay(state) {
      fetch('/relay?state=' + state)
        .then(response => {
          if (!response.ok) { console.error('Failed to control relay'); }
          // Wait for next data fetch to update status
        })
        .catch(error => console.error('Error:', error));
    }

    function updateSchedule(event) {
        event.preventDefault();
        const form = document.getElementById('scheduleForm');
        const formData = new FormData(form);
        const onTime24 = formData.get('onTime');
        const offTime24 = formData.get('offTime');
        const params = new URLSearchParams();
        params.append('enabled', formData.has('scheduleEnabled') ? 'true' : 'false');
        params.append('onTime', onTime24);
        params.append('offTime', offTime24);

        fetch('/schedule', {
            method: 'POST',
            headers: { 'Content-Type': 'application/x-www-form-urlencoded', },
            body: params.toString()
        })
        .then(response => {
          if (!response.ok) { alert('Failed to update schedule.'); console.error('Schedule update failed'); }
          else { alert('Schedule updated successfully!'); fetchData(); }
        })
        .catch(error => { alert('Error updating schedule.'); console.error('Error:', error); });
    }

    function formatTimeForInput(hour, minute, isPM) {
        let hour24 = hour;
        if (isPM && hour !== 12) hour24 += 12;
        if (!isPM && hour === 12) hour24 = 0;
        const h = String(hour24).padStart(2, '0');
        const m = String(minute).padStart(2, '0');
        return `${h}:${m}`;
    }

    function fetchData() {
      fetch('/data')
        .then(response => response.json())
        .then(data => {
          document.getElementById('energy').textContent = data.energy.toFixed(3); // Format number from JSON
          document.getElementById('voltage').textContent = data.voltage.toFixed(1);
          document.getElementById('current').textContent = data.current.toFixed(1);
          document.getElementById('pf').textContent = data.powerFactor.toFixed(2);

          const relayStateElement = document.getElementById('relayState');
          relayStateElement.textContent = data.relayState ? 'ON' : 'OFF';
          relayStateElement.style.color = data.relayState ? 'green' : 'red';

          // Update Schedule Form
          document.getElementById('scheduleEnabled').checked = data.scheduleEnabled;
          // Format times received from ESP (12-hour AM/PM) to 24-hour for input
          document.getElementById('onTime').value = formatTimeForInput(data.scheduleOnHour, data.scheduleOnMinute, data.scheduleOnIsPM);
          document.getElementById('offTime').value = formatTimeForInput(data.scheduleOffHour, data.scheduleOffMinute, data.scheduleOffIsPM);
          document.getElementById('currentTime').textContent = data.currentTime;
        })
        .catch(error => console.error('Error fetching data:', error));
    }

    fetchData();
    setInterval(fetchData, 5000); // Refresh every 5 seconds
  </script>
</body>
</html>
)rawliteral";


void setup() {
    Serial.begin(115200);
    u8g2.begin();
    Wire.begin(SDA_PIN, SCL_PIN);
    pinMode(BUTTON_PREV, INPUT_PULLUP);
    pinMode(BUTTON_SELECT, INPUT_PULLUP);
    pinMode(BUTTON_NEXT, INPUT_PULLUP);
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(LED, OUTPUT);
    digitalWrite(RELAY_PIN, HIGH);

    // <<< PZEM-004T >>> Initialize Serial2 for PZEM
    // Serial2.begin(9600, SERIAL_8N1, PZEM_RX_PIN, PZEM_TX_PIN); // Make sure baud rate matches sensor
    // Serial.println("PZEM Serial Initialized");

    // loadSchedule(); // Load saved settings if implementing persistence

    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, password); // <<< YOUR WIFI SSID and PASSWORD >>>
    while (WiFi.status() != WL_CONNECTED) {
        delay(500); Serial.print(".");
        u8g2.clearBuffer(); u8g2.setFont(u8g2_font_5x7_tr);
        u8g2.drawStr(0,10,"Connecting WiFi..."); u8g2.sendBuffer();
    }
    Serial.println("\nWiFi connected");
    Serial.print("IP address: "); Serial.println(WiFi.localIP());

    u8g2.clearBuffer(); u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(0, 10, "WiFi Connected!");
    u8g2.drawStr(0, 25, "IP Address:");
    u8g2.drawStr(0, 40, WiFi.localIP().toString().c_str());
    u8g2.sendBuffer(); delay(3000);

    configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov"); // GMT+8
    Serial.println("Waiting for time sync...");
    struct tm timeinfo;
    while (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time. Retrying..."); delay(1000);
        u8g2.clearBuffer(); u8g2.setFont(u8g2_font_5x7_tr);
        u8g2.drawStr(0,10,"Syncing Time..."); u8g2.sendBuffer();
    }
    Serial.println("Time synchronized");
    char timeStr[64]; strftime(timeStr, sizeof(timeStr), "%A, %B %d %Y %H:%M:%S", &timeinfo); Serial.println(timeStr);

    // <<< WEB SERVER >>> Define routes
    server.on("/", HTTP_GET, handleRoot);
    server.on("/data", HTTP_GET, handleGetData);
    server.on("/relay", HTTP_GET, handleRelayControl);
    server.on("/schedule", HTTP_POST, handleScheduleUpdate);
    server.onNotFound(handleNotFound);

    // <<< WEB SERVER >>> Start server
    server.begin();
    Serial.println("HTTP server started");

    drawRotatingMenu(currentIndex); // Initial draw
}

void loop() {
    handleButtons(); // Check physical buttons

    // Sensor reading and current limit check
    if (millis() - lastSensorReadTime > 2000) {
        readSensors(); // sensorCurrent is updated here

        // --- PRIORITY 1: Current Limit Override ---
        // If current limit is enabled, set (greater than 0), and exceeded, turn relay OFF.
        if (currentLimitFeatureEnabled && currentTripPoint > 0 && sensorCurrent > currentTripPoint) {
            if (digitalRead(RELAY_PIN) == LOW) { // If relay is currently ON
                digitalWrite(RELAY_PIN, HIGH); // Turn OFF
                digitalWrite(LED, LOW);
                if (millis() - lastCurrentLimitCheckTime > 5000) { // Avoid spamming serial
                    Serial.printf("OVERCURRENT (%.2fA > %.2fA limit)! Relay FORCED OFF.\n", sensorCurrent, currentTripPoint);
                    lastCurrentLimitCheckTime = millis();
                }
                // Note: This override does not change manualControlEngaged or scheduleEnabled flags.
                // The relay will remain off as long as current is high.
                // Once current drops, other logic (manual/schedule) might turn it back on if conditions allow.
            }
        } else {
            // --- Current is OK or limit not active ---
            // PRIORITY 2: Manual Control
            if (manualControlEngaged) {
                // If manual control is engaged, ensure relay state matches manualRelayActualState.
                // This is important if current was high and now it's normal again.
                bool currentRelayIsOn = (digitalRead(RELAY_PIN) == LOW);
                if (manualRelayActualState != currentRelayIsOn) {
                    Serial.printf("Manual control: Setting relay to %s (current OK)\n", manualRelayActualState ? "ON" : "OFF");
                    digitalWrite(RELAY_PIN, manualRelayActualState ? LOW : HIGH);
                    digitalWrite(LED, manualRelayActualState ? HIGH : LOW);
                }
            }
            // PRIORITY 3: Scheduled Control (only if not manually controlled)
            else if (scheduleEnabled) {
                if (millis() - lastRelayCheckTime > 1000) { // Check schedule more frequently if active
                    checkScheduleAndControlRelay();
                    lastRelayCheckTime = millis();
                }
            }
            // PRIORITY 4: Default OFF (if no active control system wants it ON)
            else {
                // Not manually controlled, not schedule enabled, and current is OK.
                // Ensure relay is OFF.
                if (digitalRead(RELAY_PIN) == LOW) { // If relay is ON
                    Serial.println("No active control (manual/schedule), current OK. Defaulting Relay OFF.");
                    digitalWrite(RELAY_PIN, HIGH); // Turn OFF
                    digitalWrite(LED, LOW);
                }
            }
        }
        lastSensorReadTime = millis();
    }

    // Minimal delay - removed the old lastRelayCheckTime block as it's integrated above
    delay(10);
}
// --- Sensor Reading Function ---
void readSensors() {
    // <<< PZEM-004T >>> Uncomment and adapt to read actual values
    Serial.println("Reading PZEM Sensor...");
    float voltage = pzem.voltage();
    float current = pzem.current();
    float power = pzem.power(); // Active Power (W)
    float energy = pzem.energy(); // Wh - convert to kWh if needed
    float frequency = pzem.frequency(); // Hz
    float pf = pzem.pf(); // Power Factor

    if (!isnan(voltage)) { sensorVoltage = voltage; } else { Serial.println("Error reading voltage"); }
    if (!isnan(current)) { sensorCurrent = current; } else { Serial.println("Error reading current"); }
    if (!isnan(energy)) { sensorEnergy = energy / 1000.0; } else { Serial.println("Error reading energy"); } // Convert Wh to kWh
    if (!isnan(power)) { sensorPower = power; } else { Serial.println("Error reading power"); }
    if (!isnan(pf)) { sensorPowerFactor = pf; } else { Serial.println("Error reading power factor"); }

    //Update the display IF the current state is SHOW_READINGS
    if (currentState == SHOW_READINGS) {
          drawShowReadingsPage();
    }
    Serial.printf(" V:%.1f V, I:%.3f A, P:%.2f W, E:%.3f kWh, PF:%.2f\n",
    sensorVoltage, sensorCurrent, sensorPower, sensorEnergy, sensorPowerFactor);
     if (currentState == SHOW_READINGS) {
          drawShowReadingsPage(); 
     }
     // --- End placeholder ---
}


// --- Button Handling (unchanged from previous version) ---
void handleButtons() {
    if (millis() - lastDebounceTime < 200) return; // Debounce

    bool prevPressed = (digitalRead(BUTTON_PREV) == LOW);
    bool nextPressed = (digitalRead(BUTTON_NEXT) == LOW);
    bool selectPressed = (digitalRead(BUTTON_SELECT) == LOW);

    if (!prevPressed && !nextPressed && !selectPressed) return;

    lastDebounceTime = millis();
    switch (currentState) {
        case MENU:
            if (prevPressed) { currentIndex = (currentIndex - 1 + menuItemCount) % menuItemCount; drawRotatingMenu(currentIndex); }
            else if (nextPressed) { currentIndex = (currentIndex + 1) % menuItemCount; drawRotatingMenu(currentIndex); }
            else if (selectPressed) {
                switch (currentIndex) {
                    case 0: currentState = SHOW_READINGS; drawShowReadingsPage(); break;
                    case 1: currentState = WIFI_INFO; drawWifiInfoPage(); break;
                    case 2: currentState = SCHEDULE; currentScheduleSubState = SELECT_ENABLED; drawSchedulePage(); break;
                    case 3: 
                        currentState = RELAY_CONTROL;
                        currentRelayControlSubState = SELECT_MANUAL_STATE;
                        drawRelayControlPage();
                        break;
                }
            }
            break;
        case SHOW_READINGS:
        case WIFI_INFO:
            if (selectPressed || prevPressed) { currentState = MENU; drawRotatingMenu(currentIndex); }
            break;
        case SCHEDULE:
            if (prevPressed) { currentState = MENU; drawRotatingMenu(currentIndex); }
            else if (nextPressed) { currentScheduleSubState = (ScheduleSubState)((currentScheduleSubState + 1) % 3); drawSchedulePage(); }
            else if (selectPressed) {
                switch (currentScheduleSubState) {
                    case SELECT_ENABLED:
                        scheduleEnabled = !scheduleEnabled;
                        if (scheduleEnabled) {
                            manualControlEngaged = false; // [MODIFIED] Schedule takes precedence if enabled
                        }
                        drawSchedulePage(); break;
                    case SELECT_ON_TIME:
                        currentState = SET_TIME; isSettingOnTime = true;
                        tempHour = scheduleOnHour; tempMinute = scheduleOnMinute; tempIsPM = scheduleOnIsPM;
                        currentSetTimeSubState = EDIT_HOUR; drawSetTimePage(); break;
                    case SELECT_OFF_TIME:
                        currentState = SET_TIME; isSettingOnTime = false;
                        tempHour = scheduleOffHour; tempMinute = scheduleOffMinute; tempIsPM = scheduleOffIsPM;
                        currentSetTimeSubState = EDIT_HOUR; drawSetTimePage(); break;
                }
            }
            break;
        case SET_TIME:
            // ... (existing SET_TIME logic remains the same)
            if (selectPressed) {
                if (currentSetTimeSubState == SAVE_EXIT) {
                    if (isSettingOnTime) { scheduleOnHour = tempHour; scheduleOnMinute = tempMinute; scheduleOnIsPM = tempIsPM; }
                    else { scheduleOffHour = tempHour; scheduleOffMinute = tempMinute; scheduleOffIsPM = tempIsPM; }
                    currentState = SCHEDULE; drawSchedulePage();
                } else { currentSetTimeSubState = (SetTimeSubState)((currentSetTimeSubState + 1) % 4); drawSetTimePage(); }
            } else if (prevPressed) {
                switch (currentSetTimeSubState) {
                    case EDIT_HOUR: tempHour--; if (tempHour < 1) tempHour = 12; break;
                    case EDIT_MINUTE: tempMinute = (tempMinute - 1 + 60) % 60; break;
                    case EDIT_AMPM: tempIsPM = !tempIsPM; break;
                    case SAVE_EXIT: currentSetTimeSubState = EDIT_AMPM; break; // Cycle back
                } drawSetTimePage();
            } else if (nextPressed) {
                switch (currentSetTimeSubState) {
                    case EDIT_HOUR: tempHour++; if (tempHour > 12) tempHour = 1; break;
                    case EDIT_MINUTE: tempMinute = (tempMinute + 1) % 60; break;
                    case EDIT_AMPM: tempIsPM = !tempIsPM; break;
                    case SAVE_EXIT: currentSetTimeSubState = EDIT_HOUR; break; // Cycle back
                } drawSetTimePage();
            }
            break;

        // [NEW] Handle buttons for Relay Control Page
        case RELAY_CONTROL:
            if (prevPressed) { // Go back to MENU
                currentState = MENU;
                drawRotatingMenu(currentIndex);
            } else if (nextPressed) { // Cycle selection on page
                currentRelayControlSubState = (currentRelayControlSubState == SELECT_MANUAL_STATE) ? SELECT_LIMIT_SETTING : SELECT_MANUAL_STATE;
                drawRelayControlPage();
            } else if (selectPressed) { // Activate selected item
                if (currentRelayControlSubState == SELECT_MANUAL_STATE) {
                    manualControlEngaged = true;    // Engage manual control
                    scheduleEnabled = false;        // Manual control disables schedule
                    manualRelayActualState = !manualRelayActualState; // Toggle manual state
                    digitalWrite(RELAY_PIN, manualRelayActualState ? LOW : HIGH); // Apply state
                    digitalWrite(LED, manualRelayActualState ? HIGH : LOW);
                    Serial.printf("Manual control: Relay %s\n", manualRelayActualState ? "ON" : "OFF");
                    drawRelayControlPage();
                } else if (currentRelayControlSubState == SELECT_LIMIT_SETTING) {
                    currentState = EDIT_CURRENT_LIMIT;
                    currentEditLimitSubState = ADJUST_LIMIT_VALUE;
                    temporaryCurrentTripPoint = currentTripPoint; // Load current limit into temp variable
                    drawEditCurrentLimitPage();
                }
            }
            break;

        case EDIT_CURRENT_LIMIT:
            if (selectPressed) { // "CONFIRM" or "SAVE"
                if (currentEditLimitSubState == ADJUST_LIMIT_VALUE) {
                    currentEditLimitSubState = CONFIRM_LIMIT_VALUE;
                    drawEditCurrentLimitPage();
                } else if (currentEditLimitSubState == CONFIRM_LIMIT_VALUE) {
                    currentTripPoint = temporaryCurrentTripPoint; // Save the new limit
                    // saveCurrentLimit(); // Optional: Implement EEPROM save
                    Serial.printf("New current limit set: %.2f A\n", currentTripPoint);
                    currentState = RELAY_CONTROL; // Go back to relay control page
                    currentRelayControlSubState = SELECT_LIMIT_SETTING; // Highlight the limit item
                    drawRelayControlPage();
                }
            } else if (prevPressed) { // "DECREASE" value or go back from "SAVE"
                if (currentEditLimitSubState == ADJUST_LIMIT_VALUE) {
                    temporaryCurrentTripPoint -= 0.1;
                    if (temporaryCurrentTripPoint < 0) temporaryCurrentTripPoint = 0; // Min limit 0
                    drawEditCurrentLimitPage();
                } else if (currentEditLimitSubState == CONFIRM_LIMIT_VALUE) {
                    currentEditLimitSubState = ADJUST_LIMIT_VALUE; // Go back to editing
                    drawEditCurrentLimitPage();
                }
            } else if (nextPressed) { // "INCREASE" value or go back from "SAVE"
                if (currentEditLimitSubState == ADJUST_LIMIT_VALUE) {
                    temporaryCurrentTripPoint += 0.1;
                    // Add a max cap if desired, e.g., if (temporaryCurrentTripPoint > 20) temporaryCurrentTripPoint = 20;
                    drawEditCurrentLimitPage();
                } else if (currentEditLimitSubState == CONFIRM_LIMIT_VALUE) {
                     currentEditLimitSubState = ADJUST_LIMIT_VALUE; // Go back to editing
                     drawEditCurrentLimitPage();
                }
            }
            break;
    }
}
// --- Drawing Functions ---

// drawHighlight (unchanged)
void drawHighlight(int x, int y, int w, int h) { u8g2.drawFrame(x-2, y-2, w+4, h+2); }
// drawPersistentElements (unchanged)
// drawPersistentElements (unchanged)
void drawPersistentElements() {
    u8g2.setFont(u8g2_font_4x6_tr); struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) { u8g2.drawStr(0, 5, "Time N/A"); }
    else {
        char timeStr[6]; strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);
        char dateStr[11]; strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", &timeinfo);
        u8g2.drawStr(0, 5, timeStr);
        u8g2.drawStr(u8g2.getDisplayWidth() - u8g2.getStrWidth(dateStr) - 1, 5, dateStr);
    }
    const char* L = "PREV"; const char* C = "SELECT"; const char* R = "NEXT";
    if (currentState == MENU) { C = "ENTER"; }
    else if (currentState == SET_TIME) { C = (currentSetTimeSubState == SAVE_EXIT) ? "SAVE" : "NEXT>"; }
    else if (currentState == SCHEDULE) { C = (currentScheduleSubState == SELECT_ENABLED) ? "TOGGLE" : "EDIT"; }
    else if (currentState == SHOW_READINGS || currentState == WIFI_INFO) { C = "BACK"; }
    // [MODIFIED] Add button labels for new states
    else if (currentState == RELAY_CONTROL) { L = "BACK"; C = "SELECT"; R = "CYCLE"; }
    else if (currentState == EDIT_CURRENT_LIMIT) {
        L = "DEC";
        C = (currentEditLimitSubState == ADJUST_LIMIT_VALUE) ? "CONFIRM" : "SAVE";
        R = "INC";
    }

    u8g2.drawStr(0, 64, L);
    u8g2.drawStr(u8g2.getDisplayWidth()/2 - u8g2.getStrWidth(C)/2 , 64, C);
    u8g2.drawStr(u8g2.getDisplayWidth() - u8g2.getStrWidth(R) -1 , 64, R);
    // The original code had the time/date inside an else, but button labels outside.
    // Moved button label drawing outside the timeinfo else for robustness,
    // though time should usually be available after setup.
}

// Updated drawShowReadingsPage
void drawShowReadingsPage() {
    u8g2.clearBuffer();
    u8g2.setFontMode(1); u8g2.setBitmapMode(1);

    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(38, 13, "Sensor Reading");
    u8g2.drawXBM(26, 6, 9, 10, image_Lightining_logo_bits);

    u8g2.setFont(u8g2_font_5x7_tr);
    char buffer[25]; // Buffer for formatting readings

    // Use snprintf to format float variables into the buffer
    snprintf(buffer, sizeof(buffer), "Power: %.2f W", sensorPower);
    u8g2.drawStr(6, 23, buffer);

    snprintf(buffer, sizeof(buffer), "VOLTAGE: %.1f V", sensorVoltage);
    u8g2.drawStr(6, 33, buffer);

    snprintf(buffer, sizeof(buffer), "CURRENT: %.2f A", sensorCurrent);
    u8g2.drawStr(6, 43, buffer);

    snprintf(buffer, sizeof(buffer), "PF: %.2f", sensorPowerFactor);
    u8g2.drawStr(6, 53, buffer);

    drawPersistentElements();
    u8g2.sendBuffer();
}

// drawWifiInfoPage (unchanged)
void drawWifiInfoPage() {
    u8g2.clearBuffer(); u8g2.setFontMode(1); u8g2.setBitmapMode(1);
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(u8g2.getDisplayWidth()/2 - u8g2.getStrWidth("WiFi Info")/2, 13, "WiFi Info");
    u8g2.setFont(u8g2_font_5x7_tr);
    if (WiFi.status() == WL_CONNECTED) {
        u8g2.drawStr(5, 25, "Status: Connected");
        String ipStr = "IP: " + WiFi.localIP().toString();
        u8g2.drawStr(5, 35, ipStr.c_str());
        String ssidStr = "SSID: " + WiFi.SSID(); 
        u8g2.drawStr(5, 45, ssidStr.c_str());
    } else { u8g2.drawStr(5, 35, "Status: Disconnected"); }
    drawPersistentElements(); u8g2.sendBuffer();
}

// drawSchedulePage 
void drawSchedulePage() {
    u8g2.clearBuffer(); u8g2.setFontMode(1); u8g2.setBitmapMode(1); char buffer[10];
    u8g2.setFont(u8g2_font_4x6_tr); u8g2.drawStr(7, 20, "USE SCHEDULING:");
    u8g2.setFont(u8g2_font_5x7_tr); int yesX = 81; int noX = 104;
    u8g2.drawStr(yesX, 21, "YES"); u8g2.drawStr(noX, 21, "NO");
    if (scheduleEnabled){ 
      u8g2.drawBox(yesX - 1, 14, u8g2.getStrWidth("YES") + 2, 9);
      u8g2.setDrawColor(0); u8g2.drawStr(yesX, 21, "YES"); 
      u8g2.setDrawColor(1);
    }
    else { u8g2.drawBox(noX - 1, 14, u8g2.getStrWidth("NO") + 2, 9); 
          u8g2.setDrawColor(0); 
          u8g2.drawStr(noX, 21, "NO"); 
          u8g2.setDrawColor(1); }
    if (currentScheduleSubState == SELECT_ENABLED){ 
      drawHighlight(4, 13, 120, 11); 
    }

    int iconY = 28; int timeTextY = 36; int timeValueY = 48;
    u8g2.drawXBM(5, iconY, 19, 20, image_On_logo_bits); u8g2.drawXBM(68, iconY, 19, 20, image_off_logo_bits);
    u8g2.setFont(u8g2_font_5x7_tr); u8g2.drawStr(28, timeTextY, "ON AT:"); u8g2.drawStr(91, timeTextY, "OFF AT:");
    u8g2.setFont(u8g2_font_4x6_tr);
    snprintf(buffer, sizeof(buffer), "%02d:%02d %s", scheduleOnHour, scheduleOnMinute, scheduleOnIsPM ? "PM" : "AM"); u8g2.drawStr(27, timeValueY, buffer);
    if (currentScheduleSubState == SELECT_ON_TIME) { 
      drawHighlight(4, iconY - 1, 60, 23);
    }
    snprintf(buffer, sizeof(buffer), "%02d:%02d %s", scheduleOffHour, scheduleOffMinute, scheduleOffIsPM ? "PM" : "AM"); u8g2.drawStr(89, timeValueY, buffer);
    if (currentScheduleSubState == SELECT_OFF_TIME) { 
      drawHighlight(67, iconY - 1, 58, 23);
    }
    drawPersistentElements(); u8g2.sendBuffer();
}

// drawSetTimePage (unchanged)
void drawSetTimePage() {
    u8g2.clearBuffer(); u8g2.setFontMode(1); char buffer[5];
    u8g2.setFont(u8g2_font_5x7_tr); const char* title = isSettingOnTime ? "Set ON Time" : "Set OFF Time";
    u8g2.drawStr(u8g2.getDisplayWidth()/2 - u8g2.getStrWidth(title)/2, 13, title);
    int timeY = 35; int hourX = 35; int colon1X = hourX + u8g2.getStrWidth("00") + 3;
    int minX = colon1X + u8g2.getStrWidth(":") + 3; int ampmX = minX + u8g2.getStrWidth("00") + 5;
    u8g2.setFont(u8g2_font_7x14B_tr);
    snprintf(buffer, sizeof(buffer), "%02d", tempHour); u8g2.drawStr(hourX, timeY, buffer);
    if (currentSetTimeSubState == EDIT_HOUR) { 
      drawHighlight(hourX - 1, timeY - 11, u8g2.getStrWidth(buffer) + 2, 14); 
    }
    u8g2.drawStr(colon1X, timeY, ":");
    snprintf(buffer, sizeof(buffer), "%02d", tempMinute); u8g2.drawStr(minX, timeY, buffer);
    if (currentSetTimeSubState == EDIT_MINUTE) {
      drawHighlight(minX - 1, timeY - 11, u8g2.getStrWidth(buffer) + 2, 14);
    }
    u8g2.drawStr(ampmX, timeY, tempIsPM ? "PM" : "AM");
    if (currentSetTimeSubState == EDIT_AMPM) { 
      drawHighlight(ampmX - 1, timeY - 11, u8g2.getStrWidth("MM") + 2, 14);
    }
    int saveY = 50; const char* saveText = "[SAVE & EXIT]"; int saveX = u8g2.getDisplayWidth()/2 - u8g2.getStrWidth(saveText)/2;
    u8g2.setFont(u8g2_font_5x7_tr); u8g2.drawStr(saveX, saveY, saveText);
    if (currentSetTimeSubState == SAVE_EXIT) { 
      drawHighlight(saveX - 2, saveY - 7, u8g2.getStrWidth(saveText) + 4, 10);
    }
    drawPersistentElements(); u8g2.sendBuffer();
}

void drawRelayControlPage() {
    u8g2.clearBuffer();
    u8g2.setFontMode(1);
    u8g2.setBitmapMode(1);
    char buffer[30];

    // Title
    u8g2.setFont(u8g2_font_5x7_tr); // A slightly larger font for title
    u8g2.drawStr(u8g2.getDisplayWidth()/2 - u8g2.getStrWidth("RELAY CONTROLS")/2, 13, "RELAY CONTROLS");

    // Manual Control Option
    u8g2.setFont(u8g2_font_4x6_tr);
    // Display based on manualRelayActualState, indicate if manualControlEngaged makes it active
    // If manualControlEngaged is true, this is the active state. Otherwise, it's the state it *would* be.
    sprintf(buffer, "Manual Control: %s %s",
            manualRelayActualState ? "ON" : "OFF",
            manualControlEngaged ? "*" : ""); // Asterisk indicates manual mode is active
    u8g2.drawStr(10, 30, buffer);
    if (currentRelayControlSubState == SELECT_MANUAL_STATE) {
        drawHighlight(7, 25, u8g2.getDisplayWidth() - 14, 10);
    }

    // Current Limit Option
    if (currentTripPoint > 0) {
        sprintf(buffer, "SET Current Limit: %.1f A", currentTripPoint);
    } else {
        sprintf(buffer, "SET Current Limit: OFF");
    }
    u8g2.drawStr(10, 45, buffer);
    if (currentRelayControlSubState == SELECT_LIMIT_SETTING) {
        drawHighlight(7, 40, u8g2.getDisplayWidth() - 14, 10);
    }

    drawPersistentElements();
    u8g2.sendBuffer();
}

// [NEW] Drawing function for Edit Current Limit Page
void drawEditCurrentLimitPage() {
    u8g2.clearBuffer();
    u8g2.setFontMode(1);
    u8g2.setBitmapMode(1);
    char buffer[20];

    // Title
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(u8g2.getDisplayWidth()/2 - u8g2.getStrWidth("Set Current Limit")/2, 13, "Set Current Limit");

    // Current Limit Value
    u8g2.setFont(u8g2_font_7x14B_tr); // Larger font for the value
    if (temporaryCurrentTripPoint > 0) {
        sprintf(buffer, "%.1f A", temporaryCurrentTripPoint);
    } else {
        sprintf(buffer, "OFF");
    }
    int valueWidth = u8g2.getStrWidth(buffer);
    u8g2.drawStr(u8g2.getDisplayWidth()/2 - valueWidth/2, 35, buffer);
    if (currentEditLimitSubState == ADJUST_LIMIT_VALUE) {
        // Highlight the numeric value being edited
        drawHighlight(u8g2.getDisplayWidth()/2 - valueWidth/2 - 2, 35 - 11, valueWidth + 4, 14);
    }

    // Save & Exit Text (as part of persistent elements C button or specific text)
    u8g2.setFont(u8g2_font_5x7_tr);
    if (currentEditLimitSubState == CONFIRM_LIMIT_VALUE) {
        const char* saveText = "[SAVE & EXIT]";
        int saveTextWidth = u8g2.getStrWidth(saveText);
        drawHighlight(u8g2.getDisplayWidth()/2 - saveTextWidth/2 - 2, 50 - 7, saveTextWidth + 4, 10);
        u8g2.drawStr(u8g2.getDisplayWidth()/2 - saveTextWidth/2, 50, saveText);
    }

    drawPersistentElements();
    u8g2.sendBuffer();
}


// drawRotatingMenu (unchanged)
void drawRotatingMenu(int selected) {
    u8g2.clearBuffer(); u8g2.setFontMode(1); u8g2.setBitmapMode(1); u8g2.setFont(u8g2_font_4x6_tr);
    int leftIndex = (selected - 1 + menuItemCount) % menuItemCount; 
    int centerIndex = selected; 
    int rightIndex = (selected + 1) % menuItemCount;
    int iconYOffset = 8; 
    int lw = iconWidths[leftIndex];
    int lh = iconHeights[leftIndex];
    u8g2.drawXBM(20, 23 + iconYOffset, lw, lh, icons[leftIndex]); 
  
    int rw = iconWidths[rightIndex]; 
    int rh = iconHeights[rightIndex];
    u8g2.drawXBM(90, 23 + iconYOffset, rw, rh, icons[rightIndex]);
  
    const char* currentLabel = menuLabels[centerIndex];
    int labelWidth = u8g2.getStrWidth(currentLabel); 
    int labelX = 64 - labelWidth / 2; 
    int labelY = 18;
  
    u8g2.drawStr(labelX, labelY, currentLabel); 
    u8g2.drawXBM(55, 22 + iconYOffset, 18, 18, image_selection);
    int cw = iconWidths[centerIndex]; 
    int ch = iconHeights[centerIndex]; 
    int cx = 64 - cw / 2; int cy = 23 + iconYOffset;
    u8g2.drawXBM(cx, cy, cw, ch, icons[centerIndex]); 
    drawPersistentElements(); u8g2.sendBuffer();
}

// --- Relay Control Logic (FIXED) ---
void checkScheduleAndControlRelay() {
    if (!scheduleEnabled) { return; }
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Relay Check: Cannot get time");
        return;
    }
    int onHour24 = scheduleOnHour;
    if (scheduleOnIsPM && onHour24 != 12) onHour24 += 12;
    if (!scheduleOnIsPM && onHour24 == 12) onHour24 = 0;
    int offHour24 = scheduleOffHour;
    if (scheduleOffIsPM && offHour24 != 12) offHour24 += 12;
    if (!scheduleOffIsPM && offHour24 == 12) offHour24 = 0;
    int scheduleStartMinutes = onHour24 * 60 + scheduleOnMinute;
    int scheduleEndMinutes = offHour24 * 60 + scheduleOffMinute;
    int currentMinutes = timeinfo.tm_hour * 60 + timeinfo.tm_min;
    bool shouldBeOn = false;

    if (scheduleEndMinutes < scheduleStartMinutes) {
        if (currentMinutes >= scheduleStartMinutes || currentMinutes < scheduleEndMinutes) {
            shouldBeOn = true;
        }
    } else {
        if (currentMinutes >= scheduleStartMinutes && currentMinutes < scheduleEndMinutes) {
            shouldBeOn = true;
        }
    }

    if (shouldBeOn) {
        if (digitalRead(RELAY_PIN) == HIGH) {
            Serial.printf("Schedule: Turning Relay ON - Current: %02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min);
            digitalWrite(RELAY_PIN, LOW);
            digitalWrite(LED, HIGH);
        }
    } else {
        if (digitalRead(RELAY_PIN) == LOW) {
            Serial.printf("Schedule: Turning Relay OFF - Current: %02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min);
            digitalWrite(RELAY_PIN, HIGH);
            digitalWrite(LED, LOW);
        }
    }
}

// --- Web Server Handlers ---

// handleRoot (unchanged)
void handleRoot(AsyncWebServerRequest *request) {
    Serial.println("Web request: /");
    request->send_P(200, "text/html", index_html);
}

// Updated handleGetData (FIXED)
void handleGetData(AsyncWebServerRequest *request) {
    Serial.println("Web request: /data");
    String json = "{";
    // Readings (Using global variables, sent as numbers)
    json += "\"energy\":" + String(sensorPower, 3) + ",";
    json += "\"voltage\":" + String(sensorVoltage, 1) + ",";
    json += "\"current\":" + String(sensorCurrent, 2) + ",";
    json += "\"powerFactor\":" + String(sensorPowerFactor, 2) + ",";
    // Relay State (ON is LOW)
    json += "\"relayState\":";
    json += (digitalRead(RELAY_PIN) == LOW) ? "true," : "false,";
    // Schedule Settings
    json += "\"scheduleEnabled\":";
    json += scheduleEnabled ? "true," : "false,";
    json += "\"scheduleOnHour\":";
    json += String(scheduleOnHour) + ",";
    json += "\"scheduleOnMinute\":";
    json += String(scheduleOnMinute) + ",";
    json += "\"scheduleOnIsPM\":";
    json += scheduleOnIsPM ? "true," : "false,";
    json += "\"scheduleOffHour\":";
    json += String(scheduleOffHour) + ",";
    json += "\"scheduleOffMinute\":";
    json += String(scheduleOffMinute) + ",";
    json += "\"scheduleOffIsPM\":";
    json += scheduleOffIsPM ? "true," : "false,";
    // Current Time
    struct tm timeinfo;
    char timeBuffer[9];
    if (getLocalTime(&timeinfo)) {
        strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", &timeinfo);
        json += "\"currentTime\":\"" + String(timeBuffer) + "\"";
    } else {
        json += "\"currentTime\":\"N/A\"";
    }
    json += "}";
    request->send(200, "application/json", json);
}

// handleRelayControl
void handleRelayControl(AsyncWebServerRequest *request) {
    if (request->hasParam("state")) {
        String state = request->getParam("state")->value();
        Serial.print("Web request: /relay?state=");
        Serial.println(state);
        if (state == "on") {
            digitalWrite(RELAY_PIN, LOW); // ON is LOW
            digitalWrite(LED, HIGH);
            request->send(200, "text/plain", "Relay turned ON");
        } else if (state == "off") {
            digitalWrite(RELAY_PIN, HIGH); // OFF is HIGH
            digitalWrite(LED, LOW); 
            request->send(200, "text/plain", "Relay turned OFF");
        } else {
            request->send(400, "text/plain", "Invalid state parameter");
        }
    } else {
        request->send(400, "text/plain", "Missing state parameter");
    }
}

// handleScheduleUpdate 
void handleScheduleUpdate(AsyncWebServerRequest *request) {
    Serial.println("Web request: /schedule (POST)");
    bool updated = false;
    if (request->hasParam("enabled", true)) {
        scheduleEnabled = (request->getParam("enabled", true)->value() == "true");
        Serial.printf("  Schedule Enabled: %s\n", scheduleEnabled ? "true" : "false");
        updated = true;
    }
    if (request->hasParam("onTime", true)) {
        String onTimeStr = request->getParam("onTime", true)->value();
        int hour = onTimeStr.substring(0, 2).toInt();
        int minute = onTimeStr.substring(3, 5).toInt();
        scheduleOnIsPM = (hour >= 12);
        scheduleOnHour = hour % 12;
        if (scheduleOnHour == 0) scheduleOnHour = 12;
        scheduleOnMinute = minute;
        Serial.printf("  Schedule ON Time: %s (%02d:%02d %s)\n", onTimeStr.c_str(), scheduleOnHour, scheduleOnMinute, scheduleOnIsPM ? "PM" : "AM");
        updated = true;
    }
    if (request->hasParam("offTime", true)) {
        String offTimeStr = request->getParam("offTime", true)->value();
        int hour = offTimeStr.substring(0, 2).toInt();
        int minute = offTimeStr.substring(3, 5).toInt();
        scheduleOffIsPM = (hour >= 12);
        scheduleOffHour = hour % 12;
        if (scheduleOffHour == 0) scheduleOffHour = 12;
        scheduleOffMinute = minute;
        Serial.printf("  Schedule OFF Time: %s (%02d:%02d %s)\n", offTimeStr.c_str(), scheduleOffHour, scheduleOffMinute, scheduleOffIsPM ? "PM" : "AM");
        updated = true;
    }
    if (updated) {
        request->send(200, "text/plain", "Schedule updated");
        // saveSchedule(); // Persist settings if implementing
    } else {
        request->send(400, "text/plain", "No valid schedule parameters received");
    }
}

void handleNotFound(AsyncWebServerRequest *request) {
    Serial.printf("Web Not Found: %s\n", request->url().c_str());
    request->send(404, "text/plain", "Not found");
}
