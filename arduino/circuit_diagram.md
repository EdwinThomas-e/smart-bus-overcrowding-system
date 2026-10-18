# Hardware Circuit Diagram & Wiring Guide
**Project:** Smart Bus Overcrowding Detection & Alert System  
**Institution:** Muthoot Institute of Technology and Science (MITS Kochi)  
**Authors:** Edwin Thomas, Adwaith Vimal, Bevan Bince, Joswin Thomas  

---

## 1. System Pinout Mapping

| Component | Pin on Component | Microcontroller Pin (Arduino / ESP32) | Function / Description |
|---|---|---|---|
| **Entry IR Sensor A** | OUT | Digital Pin 2 (`INPUT_PULLUP`) | External beam break sensor (trips first on boarding) |
| | VCC / GND | 5V / GND | Power supply |
| **Entry IR Sensor B** | OUT | Digital Pin 3 (`INPUT_PULLUP`) | Internal beam break sensor (trips second on boarding) |
| | VCC / GND | 5V / GND | Power supply |
| **Exit IR Sensor A** | OUT | Digital Pin 4 (`INPUT_PULLUP`) | Internal exit beam break sensor |
| | VCC / GND | 5V / GND | Power supply |
| **Exit IR Sensor B** | OUT | Digital Pin 5 (`INPUT_PULLUP`) | External exit beam break sensor |
| | VCC / GND | 5V / GND | Power supply |
| **HC-SR04 Ultrasonic** | TRIG | Digital Pin 6 (`OUTPUT`) | 10µs trigger pulse to initiate echo |
| | ECHO | Digital Pin 7 (`INPUT`) | Echo pulse return for distance validation |
| | VCC / GND | 5V / GND | Power supply |
| **Green LED** | Anode (+) | Digital Pin 8 (via 220Ω resistor) | Safe Status indicator (< 75% capacity) |
| **Yellow LED** | Anode (+) | Digital Pin 9 (via 220Ω resistor) | Warning indicator (75% - 99% capacity) |
| **Red LED** | Anode (+) | Digital Pin 10 (via 220Ω resistor) | Overcrowded Violation Alert (>= 100%) |
| **Piezo Buzzer** | Positive (+) | Digital Pin 11 (`PWM`) | 2.4kHz acoustic alarm & audio feedback |
| | Negative (-) | GND | Ground return |
| **16x2 LCD Display** | SDA | Analog Pin A4 / ESP32 GPIO 21 | I2C Data line |
| | SCL | Analog Pin A5 / ESP32 GPIO 22 | I2C Clock line |
| | VCC / GND | 5V / GND | Backlight and logic power |

---

## 2. ASCII Circuit Schematic

```
               +-------------------------------------------+
               |            ARDUINO UNO / ESP32            |
               |                                           |
  +5V <--------+ 5V                     D2 (Ext IR Entry) <+--- OUT [IR Sensor Entry A]
  GND <--------+ GND                    D3 (Int IR Entry) <+--- OUT [IR Sensor Entry B]
               |                                           |
               |                        D4 (Int IR Exit)  <+--- OUT [IR Sensor Exit A]
               |                        D5 (Ext IR Exit)  <+--- OUT [IR Sensor Exit B]
               |                                           |
               |                        D6 (US Trig)       +---> TRIG [HC-SR04]
               |                        D7 (US Echo)      <+--- ECHO [HC-SR04]
               |                                           |
               |                        D8 (Green LED)     +---> [220Ω] ---> (|>| Green LED) ---> GND
               |                        D9 (Yellow LED)    +---> [220Ω] ---> (|>| Yellow LED) ---> GND
               |                        D10 (Red LED)      +---> [220Ω] ---> (|>| Red LED) ---> GND
               |                                           |
               |                        D11 (Buzzer PWM)   +---> (+) [Active Piezo Buzzer] (-) ---> GND
               |                                           |
               |                        A4 / SDA           <---> SDA [1602 LCD I2C Backpack]
               |                        A5 / SCL           <---> SCL [1602 LCD I2C Backpack]
               +-------------------------------------------+
```

---

## 3. Bill of Materials (BOM)

| Item No. | Component Name | Quantity | Specifications | Estimated Academic Cost (INR) |
|---|---|---|---|---|
| 1 | Arduino Uno R3 / ESP32 NodeMCU | 1 | ATmega328P / Dual-Core Xtensa 240MHz | ₹450 - ₹600 |
| 2 | IR Obstacle / Beam Break Modules | 4 | LM393 comparator, adjustable range (2-30cm) | ₹160 (₹40 each) |
| 3 | HC-SR04 Ultrasonic Distance Sensor | 1 | 2cm - 400cm range, 5V DC | ₹85 |
| 4 | 16x2 Character LCD with PCF8574 I2C | 1 | High contrast blue/yellow backlight | ₹180 |
| 5 | 5V Active Piezo Buzzer | 1 | 85dB at 10cm, 2.4kHz resonant frequency | ₹25 |
| 6 | Diffused 5mm LEDs (Green, Yellow, Red) | 3 | High brightness 20mA | ₹15 |
| 7 | Carbon Film Resistors (220Ω, 1/4W) | 3 | Current limiting for LEDs | ₹5 |
| 8 | Half-size Breadboard & Jumper Wires | 1 set | 400 tie-points + Male-Male / Male-Female wires | ₹120 |
| 9 | 9V DC Adapter / 5V USB Power Bank | 1 | Stable 1A-2A DC power supply | ₹150 |
| **Total** | | | | **~ ₹1,190 - ₹1,340** |
