# Room Environment Monitor

A full-stack IoT project based on the ESP32 microcontroller, basic express server, and a web interface. The system monitors room temperature and humidity using a DHT11 sensor and allows remote control of a LED indicator(imaginary fan) via a web application.

## Architecture Of Project

- **Firmware:** Written in C using ESP-IDF and FreeRTOS. Reads data from the DHT11 sensor and sends it to the backend, also receive control commands for the fan.
- **Backend:** Acts as a bridge between the hardware and the frontend. It logs incoming sensor data and forwards fan control commands from the web client to the ESP32.
- **Frontend:** Basic web interface with "Turn On" and "Turn Off" buttons to control the room fan.

## Prerequisites

### Hardware
- **ESP32 Development Board:** ESP32-WROOM-32
- **DHT11 Sensor**
- **LED & Resistor** (e.g., 220Ω - 330Ω) as a placeholder for the fan
- **Breadboard and jumper wires**

### Software
- **ESP-IDF**
- **Node.js and npm**

## Tech Stack
* **Firmware:** ESP32 (ESP-IDF, C, FreeRTOS)
* **Backend:** Node.js, Express.js, Axios
* **Frontend:** HTML, CSS, JavaScript, Axios
---

# Installation and Setup

Change directory to desired folder and clone the repository

```bash
git clone https://github.com/Tr1ggerbtw/room-environment-monitor.git 
```

## Firmware
Open the firmware folder in your ESP-IDF environment.<br>
Access the project configuration menu:

 ```bash
 idf.py menuconfig
 ```

Navigate to **Wi-Fi configuration** and enter your local Wi-Fi `SSID` and `Password`. Save and exit.<br>
Open `main.c` file and update the `POST_URL` macro to match your backend IP address.<br>
   * *Example:* `#define POST_URL "http://192.168.0.102:3000/ping"` <br>

## Backend
Open a new terminal and navigate to your backend directory. <br>
Install the required Node.js dependencies:

```bash
npm install express axios cors
```

Open `server.js` and update the `FIRMWARE_URL` to match the IP address assigned to your ESP32 by your router. <br>
- *Example:* `const FIRMWARE_URL = "http://192.168.0.100:80/fan"`

## Frontend

Navigate to your frontend directory. <br>
Open the `index.html` file in your code editor. <br>
In the `<script>` section at the bottom, update the `BACKEND_URL` to point to your server's IP address. <br>
   * *Example:* `const BACKEND_URL = "http://192.168.0.102:3000/backend";` <br>
---
# Start the project
After everything is setuped, just start the project in relevant directories

Build the project, flash it to your ESP32, and open the serial monitor:

```bash
idf.py build flash monitor
```

Start the backend server:
```bash
node server.js
```

Open `index.html` in any web browser to view the control panel.
