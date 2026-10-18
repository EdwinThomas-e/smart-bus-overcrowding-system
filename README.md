# Smart Bus Overcrowding Detection & Alert System
### IoT-Powered Solution for Safer, Smarter Public Transit
**Muthoot Institute of Technology and Science (MITS Kochi)**  
**Department of Artificial Intelligence & Data Science**  

* **Project Team:**
  - Edwin Thomas (Roll 29)
  - Adwaith Vimal (Roll 12)
  - Bevan Bince (Roll 26)
  - Joswin Thomas (Roll 40)

---

## 🚀 Quick Launch Instructions

### Option 1: One-Click Windows Launcher (Recommended)
Double-click the file:
```
start_simulation.bat
```
This automatically starts the simulation server and opens your web browser to **`http://localhost:3000`**.

### Option 2: Run via Command Line
Run with the Antigravity runtime or Node.js:
```powershell
# Using Antigravity node runtime
& "$env:APPDATA\Antigravity\bin\agy-node.cmd" server.js

# Or standard Node.js
node server.js
```
Then visit: **`http://localhost:3000`** in your browser.

### Option 3: Direct Browser Launch (Zero-Server Standalone)
You can directly double-click or open:
```
public\index.html
```
in any web browser (Chrome, Edge, Firefox). All audio synthesis, bus animations, logic analyzer, and metric benchmarks work out-of-the-box.

---

## 🌟 Interactive Simulation Modules

1. **🚌 Bus & Sensor Simulation Lab**:
   - Interactive 2D cross-section bus graphic showing 30 seats, 15 standing aisle spots, front entry door, and rear exit door.
   - Dual-beam directional IR sensor simulation:
     - `Beam A -> Beam B`: Passenger Ingress (+1).
     - `Exit A -> Exit B`: Passenger Egress (-1).
   - "Autonomous Route Mode" simulating travel across 8 real Kochi stops (Aluva Metro, Kalamassery, CUSAT, Edappally Toll, Palarivattom, Kakkanad InfoPark).

2. **🎛️ Driver In-Bus Cockpit Terminal**:
   - Simulated 16x2 I2C Backlit LCD screen (`OCC: 38/45 [84%]`).
   - Tactile Triple Status LEDs: Green (`<75%`), Yellow (`75-99%`), Pulsing Red (`100%+ Overcrowded`).
   - Synthesized 2.4kHz Piezo Buzzer acoustic alarm via Web Audio API.
   - Cabin Public Address (PA) speech synthesizer for automated commuter warning announcements.

3. **🏢 Transit Authority Cloud Dashboard**:
   - Live telemetry feed, speed, and location tracking for Bus `KL-07-MITS-01`.
   - Real-time Circular Speedometer / Occupancy utilization gauge.
   - Live time-series occupancy curve canvas.
   - Overcrowding incident audit log with one-click CSV export.
   - Dispatch button to mobilize auxiliary relief transit.

4. **🔬 Virtual Breadboard & Logic Analyzer**:
   - Pin voltage state inspector for ATmega328P / ESP32.
   - Real-time logic analyzer plotting digital signal pulses for Pin D2, Pin D3, and HC-SR04 ultrasonic echo.

---

## 📁 Repository Structure

```
smart-bus-overcrowding-system/
├── start_simulation.bat          <- 1-click Windows execution launcher
├── server.js                     <- Node.js HTTP server & REST API
├── README.md                     <- Quickstart and system guide
├── public/
│   └── index.html                <- Master interactive simulation interface
├── arduino/
│   ├── smart_bus_firmware.ino    <- Complete Arduino C++ production firmware
│   └── circuit_diagram.md        <- Pinout guide, schematic & Bill of Materials
├── backend/
│   ├── app.py                    <- Python Flask / REST API backend
│   ├── schema.sql                <- PostgreSQL & SQLite database schema
│   └── requirements.txt          <- Python package dependencies
└── docs/
    ├── PROJECT_REPORT.md         <- Complete academic project report
    └── PRESENTATION_GUIDE.md     <- Slide-by-slide viva presentation defense guide
```
