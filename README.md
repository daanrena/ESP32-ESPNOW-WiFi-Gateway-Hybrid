ESP32 ESP-NOW to Wi-Fi Gateway
==================================================
Receive ESP-NOW sensor data and forward to Wi-Fi on a single ESP32 board.
[ 1. THE PROBLEM ]
--------------------------------------------------
When you try to run ESP-NOW and Wi-Fi together on one ESP32, you will usually face two issues:
* Problem A: Packets Never Arrive
  - ESP32 only has ONE 2.4 GHz radio.
  - When the Gateway connects to your router, the router locks the radio channel (e.g. Channel 6).
  - If your sensor node sends on default Channel 1, the gateway will NEVER hear it.
  - No error logs, just silent packet drops.
* Problem B: ESP32 Crashes & Reboots
  - Sending HTTP/MQTT requests directly inside `OnDataRecv()` callback crashes the Wi-Fi stack.
  - Callbacks run on interrupt level, so heavy tasks must be handled in `loop()`.
[ 2. HOW IT WORKS ]
--------------------------------------------------
* GATEWAY:
  1. Connects to the Wi-Fi router first.
  2. Reads the router channel with `WiFi.channel()`.
  3. Inits ESP-NOW on that exact channel.
  4. Stores incoming packets in a temporary buffer.
  5. Processes and sends data to serial/cloud safely inside `loop()`.
* SENSOR NODE:
  1. Does NOT need the Wi-Fi router password.
  2. Forces its radio channel to match the Gateway's router channel.
  3. Sends sensor data via ESP-NOW every 5 seconds.
[ 3. SHARED DATA STRUCTURE ]
--------------------------------------------------
Both `Gateway_ESP32.ino` and `Sensor_Node.ino` must use the exact same struct:
```cpp
typedef struct struct_message {
  int node_id;
  float temperature;
  float humidity;
  int battery_level;
} struct_message;
```
[ 4. STEP-BY-STEP SETUP ]
--------------------------------------------------
STEP 1: Upload the Gateway Code
- Open `Gateway_ESP32.ino`.
- Fill in your Wi-Fi credentials:
    ssid     = "YOUR_WIFI_NAME";
    password = "YOUR_WIFI_PASSWORD";
- Flash to your Gateway ESP32 and open Serial Monitor (115200 baud).
- Look for the printed channel (e.g. `Operating Radio Channel: 6`).
STEP 2: Upload the Sensor Code
- Open `Sensor_Node.ino`.
- Match the channel number with your Gateway:
    #define WIFI_CHANNEL 6
- Flash to your Sensor ESP32.
STEP 3: Verify Data
- Open Serial Monitor on Gateway ESP32.
- Data from the sensor will start flowing in every 5 seconds smoothly!
[ 5. IMPORTANT NOTE: ROUTER CHANNELS ]
--------------------------------------------------
If your home router is set to "Auto Channel", it might automatically hop channels (e.g. from 6 to 11). If that happens, the sensor will lose connection.
To fix this:
1. Open your router admin page (`192.168.1.1`).
2. Lock your 2.4 GHz Wi-Fi to a static channel (like Channel 6).
==================================================
