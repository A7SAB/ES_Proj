# ESP32 Power Monitoring & Control ⚡

A multifunctional ESP32-based controlling system featuring:

- Real-time power monitoring via PZEM-004T sensor
- 128×64 OLED interface with hardware button navigation
- Wi‑Fi web server for remote control and scheduling
- Relay control with manual, scheduled, and current-limit protections

---

## 📌 Features

1. **Sensor Readings**
   - Voltage, current, power, energy, and power factor from PZEM‑004T.
   - Displayed on OLED and accessible via web interface.

2. **Manual Relay Control**
   - Toggle relay ON/OFF using buttons or web UI.
   - Activates `manualControlEngaged`, disabling scheduling.

3. **Scheduled Control**
   - Set ON/OFF times via OLED or web.
   - Executes automatically when schedule is enabled.

4. **Current-Limit Safety**
   - Define a current threshold (e.g. 5 A).
   - Relay auto-trips OFF if sensor current exceeds limit.

5. **OLED Display & Navigation**
   - Menu using buttons (Prev/Select/Next) for:
     - Show readings
     - Wi‑Fi info
     - Schedule settings
     - Relay controls / limit settings

6. **Web Dashboard**
   - Live sensor data and current time
   - Manual relay toggles
   - Schedule management

---

## 🔌 Hardware Requirements

- ESP32 Dev Module
- PZEM‑004T V3 sensor 
- 128×64 OLED display 
- Relay module 
- RGB LED 
- 3 buttons (Prev, Select, Next) on pins 5, 4, 15

---

## 🧩 Wiring Summary

| ESP32 Pin | Connected Component                |
|-----------|-------------------------------------|
| 21, 22    | OLED display SDA / SCL              |
| 16, 17    | PZEM‑004T TX/RX via Serial2         |
| 5, 4, 15  | Buttons Prev / Select / Next        |
| 23        | Relay input (LOW = ON) & Status LED (GREEN)              |
| 19        | Status LED (RED)                          |

---

## 🛠️ Setup & Build


### 1. Install Arduino IDE

Download the latest version of the Arduino IDE from:  
🔗 https://www.arduino.cc/en/software

---

### 2. Install ESP32 Board Support

1. Open Arduino IDE
2. Go to **File > Preferences**
3. In *"Additional Board Manager URLs"*, paste:
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json

4. Go to **Tools > Board > Boards Manager**
5. Search for `esp32` and install the package by **Espressif Systems**

---

### 3. Select Your Board

- Go to **Tools > Board**
- Select: `ESP32 Dev Module` or the board you are using

---


### 4. Install Required Libraries

Use the **Library Manager** (**Tools > Manage Libraries**) to install:

- `PZEM004Tv30`
- `U8g2`

Some libraries must be installed manually:

#### 📦 Manual Library Installation

Download and install the following libraries as ZIP files:

- 🔗 [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
- 🔗 [AsyncTCP](https://github.com/me-no-dev/AsyncTCP)

To install:
1. Go to **Sketch > Include Library > Add .ZIP Library...**
2. Select each downloaded ZIP file

---

### 5. Configure Wi-Fi

Open the main code file and update these lines:

```cpp
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
```

## ⚠️ Known Issues
- Web page issues with manual control, currenlty its advisable to use the OLED Menu only. 


