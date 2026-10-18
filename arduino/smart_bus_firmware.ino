/*
 * =========================================================================================
 * Project: Smart Bus Overcrowding Detection & Alert System
 * Institution: Muthoot Institute of Technology and Science (MITS Kochi)
 * Department: Department of Artificial Intelligence and Data Science
 * Authors: Edwin Thomas (Roll 29), Adwaith Vimal (Roll 12), 
 *          Bevan Bince (Roll 26), Joswin Thomas (Roll 40)
 * 
 * Hardware Target: Arduino Uno / Nano / ESP32
 * Sensors: 
 *   - Dual IR Beam-Break Sensors (Door Entry/Exit directional detection)
 *   - HC-SR04 Ultrasonic Sensor (Height / Door Threshold validation)
 * Output Actuators:
 *   - 16x2 LCD Display with I2C (Address: 0x27)
 *   - Triple Status LEDs (Green: Safe, Yellow: Warning, Red: Overcrowded)
 *   - 5V Piezo Buzzer (Audio alert on threshold breach)
 * Communication:
 *   - Serial Telemetry (115200 baud) / MQTT JSON packet to Cloud Backend
 * =========================================================================================
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ---------------------- PIN DEFINITIONS ----------------------
// Entry Door Dual IR Sensors (for directional entry detection)
const int PIN_IR_ENTRY_A   = 2;  // External IR Beam (broken first on entry)
const int PIN_IR_ENTRY_B   = 3;  // Internal IR Beam (broken second on entry)

// Exit Door Dual IR Sensors (for directional exit detection)
const int PIN_IR_EXIT_A    = 4;  // Internal IR Beam (broken first on exit)
const int PIN_IR_EXIT_B    = 5;  // External IR Beam (broken second on exit)

// Ultrasonic Sensor (Passenger verification at doorway)
const int PIN_US_TRIG      = 6;  
const int PIN_US_ECHO      = 7;

// Alert Actuators
const int PIN_LED_GREEN    = 8;  // Safe capacity (< 75%)
const int PIN_LED_YELLOW   = 9;  // Approaching capacity (75% - 99%)
const int PIN_LED_RED      = 10; // Overcrowded alert (>= 100%)
const int PIN_BUZZER       = 11; // Active/PWM Piezo Buzzer

// ------------------ CAPACITY CONFIGURATION -------------------
const int SEATING_CAPACITY = 30; // Max seated passengers
const int STANDING_LIMIT   = 15; // Max allowed standees
const int MAX_SAFE_CAPACITY = SEATING_CAPACITY + STANDING_LIMIT; // 45 passengers
const int WARN_THRESHOLD   = 36; // 80% threshold for warning alert

// --------------------- SYSTEM VARIABLES ----------------------
int currentPassengerCount  = 0;
int totalBoardedToday      = 0;
int totalAlightedToday     = 0;
bool isOvercrowded         = false;
bool isWarningZone         = false;
unsigned long lastBuzzerToggle = 0;
bool buzzerState           = false;
unsigned long lastTelemetrySent = 0;
const unsigned long TELEMETRY_INTERVAL_MS = 1000; // 1Hz telemetry

// Entry state machine tracking
enum SensorState {
  STATE_IDLE,
  STATE_BEAM_A_TRIGGERED,
  STATE_BOTH_TRIGGERED,
  STATE_BEAM_B_TRIGGERED
};

SensorState entryState = STATE_IDLE;
SensorState exitState  = STATE_IDLE;
unsigned long entryStateTimestamp = 0;
unsigned long exitStateTimestamp  = 0;
const unsigned long SENSOR_TIMEOUT_MS = 2500; // Reset if passenger stops in doorway

// Initialize LCD (16 columns, 2 rows at address 0x27)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Function Declarations
void processEntryDoor();
void processExitDoor();
long readUltrasonicDistanceCm();
void updateActuators();
void updateLcdDisplay();
void sendTelemetryJson();
void triggerBeep(int frequencyHz, int durationMs);

// --------------------------- SETUP ---------------------------
void setup() {
  Serial.begin(115200);
  while (!Serial) { ; } // Wait for serial monitor if USB connected

  // Configure input pins with internal pull-up (most IR modules output active LOW)
  pinMode(PIN_IR_ENTRY_A, INPUT_PULLUP);
  pinMode(PIN_IR_ENTRY_B, INPUT_PULLUP);
  pinMode(PIN_IR_EXIT_A,  INPUT_PULLUP);
  pinMode(PIN_IR_EXIT_B,  INPUT_PULLUP);

  pinMode(PIN_US_TRIG, OUTPUT);
  pinMode(PIN_US_ECHO, INPUT);

  pinMode(PIN_LED_GREEN,  OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_RED,    OUTPUT);
  pinMode(PIN_BUZZER,     OUTPUT);

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SMART BUS SYSTEM");
  lcd.setCursor(0, 1);
  lcd.print("MITS AI&DS 2026");
  
  // Self-test LED & Buzzer startup sequence
  digitalWrite(PIN_LED_GREEN, HIGH);
  delay(150);
  digitalWrite(PIN_LED_YELLOW, HIGH);
  delay(150);
  digitalWrite(PIN_LED_RED, HIGH);
  triggerBeep(2000, 100);
  delay(300);
  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_LED_YELLOW, LOW);
  digitalWrite(PIN_LED_RED, LOW);

  lcd.clear();
  Serial.println(F("[SYSTEM READY] Smart Bus Overcrowding System Initialized."));
}

// ------------------------- MAIN LOOP -------------------------
void loop() {
  // 1. Process door sensors with state machines
  processEntryDoor();
  processExitDoor();

  // 2. Evaluate capacity thresholds
  isWarningZone = (currentPassengerCount >= WARN_THRESHOLD && currentPassengerCount < MAX_SAFE_CAPACITY);
  isOvercrowded = (currentPassengerCount >= MAX_SAFE_CAPACITY);

  // 3. Update physical indicators (LEDs + Audio Alerts)
  updateActuators();

  // 4. Update In-bus Driver LCD Display
  static unsigned long lastLcdUpdate = 0;
  if (millis() - lastLcdUpdate > 250) {
    updateLcdDisplay();
    lastLcdUpdate = millis();
  }

  // 5. Send periodic JSON telemetry over Serial / MQTT bridge
  if (millis() - lastTelemetrySent > TELEMETRY_INTERVAL_MS) {
    sendTelemetryJson();
    lastTelemetrySent = millis();
  }
}

// ----------------- ENTRY SENSOR STATE MACHINE ----------------
// Directional sequence for Entry: A triggers -> Both trigger -> B triggers -> IDLE
void processEntryDoor() {
  int a = digitalRead(PIN_IR_ENTRY_A); // LOW when beam broken
  int b = digitalRead(PIN_IR_ENTRY_B);

  // Reset state if timeout occurs (stuck beam or retreat)
  if (entryState != STATE_IDLE && (millis() - entryStateTimestamp > SENSOR_TIMEOUT_MS)) {
    entryState = STATE_IDLE;
  }

  switch (entryState) {
    case STATE_IDLE:
      if (a == LOW && b == HIGH) {
        entryState = STATE_BEAM_A_TRIGGERED;
        entryStateTimestamp = millis();
      }
      break;

    case STATE_BEAM_A_TRIGGERED:
      if (a == LOW && b == LOW) {
        entryState = STATE_BOTH_TRIGGERED;
      } else if (a == HIGH && b == HIGH) {
        // Person retreated
        entryState = STATE_IDLE;
      }
      break;

    case STATE_BOTH_TRIGGERED:
      if (a == HIGH && b == LOW) {
        entryState = STATE_BEAM_B_TRIGGERED;
      } else if (a == HIGH && b == HIGH) {
        // Quick pass completed
        currentPassengerCount++;
        totalBoardedToday++;
        triggerBeep(1800, 50);
        entryState = STATE_IDLE;
      }
      break;

    case STATE_BEAM_B_TRIGGERED:
      if (a == HIGH && b == HIGH) {
        // Successfully crossed inside!
        currentPassengerCount++;
        totalBoardedToday++;
        triggerBeep(1800, 50);
        entryState = STATE_IDLE;
      }
      break;
  }
}

// ------------------ EXIT SENSOR STATE MACHINE -----------------
// Directional sequence for Exit: Exit_A (inside) -> Both -> Exit_B (outside) -> IDLE
void processExitDoor() {
  int a = digitalRead(PIN_IR_EXIT_A);
  int b = digitalRead(PIN_IR_EXIT_B);

  if (exitState != STATE_IDLE && (millis() - exitStateTimestamp > SENSOR_TIMEOUT_MS)) {
    exitState = STATE_IDLE;
  }

  switch (exitState) {
    case STATE_IDLE:
      if (a == LOW && b == HIGH) {
        exitState = STATE_BEAM_A_TRIGGERED;
        exitStateTimestamp = millis();
      }
      break;

    case STATE_BEAM_A_TRIGGERED:
      if (a == LOW && b == LOW) {
        exitState = STATE_BOTH_TRIGGERED;
      } else if (a == HIGH && b == HIGH) {
        exitState = STATE_IDLE;
      }
      break;

    case STATE_BOTH_TRIGGERED:
      if (a == HIGH && b == LOW) {
        exitState = STATE_BEAM_B_TRIGGERED;
      } else if (a == HIGH && b == HIGH) {
        if (currentPassengerCount > 0) currentPassengerCount--;
        totalAlightedToday++;
        triggerBeep(1200, 50);
        exitState = STATE_IDLE;
      }
      break;

    case STATE_BEAM_B_TRIGGERED:
      if (a == HIGH && b == HIGH) {
        if (currentPassengerCount > 0) currentPassengerCount--;
        totalAlightedToday++;
        triggerBeep(1200, 50);
        exitState = STATE_IDLE;
      }
      break;
  }
}

// ---------------- ULTRASONIC SENSOR READING ------------------
long readUltrasonicDistanceCm() {
  digitalWrite(PIN_US_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_US_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_US_TRIG, LOW);

  long duration = pulseIn(PIN_US_ECHO, HIGH, 30000); // 30ms timeout (~5m max)
  if (duration == 0) return 999; // Out of range
  return duration * 0.034 / 2;
}

// --------------------- ACTUATOR CONTROL ----------------------
void updateActuators() {
  if (isOvercrowded) {
    // Red LED Solid, Green & Yellow OFF
    digitalWrite(PIN_LED_GREEN,  LOW);
    digitalWrite(PIN_LED_YELLOW, LOW);
    digitalWrite(PIN_LED_RED,    HIGH);

    // Urgent pulsing audio alarm (e.g. 2400Hz alternating every 200ms)
    if (millis() - lastBuzzerToggle >= 200) {
      buzzerState = !buzzerState;
      if (buzzerState) {
        tone(PIN_BUZZER, 2400);
      } else {
        noTone(PIN_BUZZER);
      }
      lastBuzzerToggle = millis();
    }
  } else if (isWarningZone) {
    // Yellow LED Solid, Green & Red OFF
    digitalWrite(PIN_LED_GREEN,  LOW);
    digitalWrite(PIN_LED_YELLOW, HIGH);
    digitalWrite(PIN_LED_RED,    LOW);

    // Occasional warning chirp every 1.5s
    if (millis() - lastBuzzerToggle >= 1500) {
      tone(PIN_BUZZER, 1600, 80);
      lastBuzzerToggle = millis();
    }
  } else {
    // Normal Safe Status: Green LED Solid
    digitalWrite(PIN_LED_GREEN,  HIGH);
    digitalWrite(PIN_LED_YELLOW, LOW);
    digitalWrite(PIN_LED_RED,    LOW);
    noTone(PIN_BUZZER);
  }
}

// -------------------- LCD 1602 IN-BUS DISPLAY ----------------
void updateLcdDisplay() {
  lcd.setCursor(0, 0);
  lcd.print("OCC:");
  if (currentPassengerCount < 10) lcd.print(" ");
  lcd.print(currentPassengerCount);
  lcd.print("/");
  lcd.print(MAX_SAFE_CAPACITY);
  lcd.print(" [");
  int pct = (currentPassengerCount * 100) / MAX_SAFE_CAPACITY;
  if (pct < 100) lcd.print(" ");
  lcd.print(pct);
  lcd.print("%]");

  lcd.setCursor(0, 1);
  if (isOvercrowded) {
    lcd.print("! OVERCROWDED ! ");
  } else if (isWarningZone) {
    lcd.print("STATUS: WARNING ");
  } else {
    lcd.print("STATUS: NORMAL  ");
  }
}

// ----------------- JSON TELEMETRY EMISSION -------------------
void sendTelemetryJson() {
  int pct = (currentPassengerCount * 100) / MAX_SAFE_CAPACITY;
  Serial.print(F("{\"bus_id\":\"KL-07-MITS-01\",\"occupancy\":"));
  Serial.print(currentPassengerCount);
  Serial.print(F(",\"max_capacity\":"));
  Serial.print(MAX_SAFE_CAPACITY);
  Serial.print(F(",\"occupancy_pct\":"));
  Serial.print(pct);
  Serial.print(F(",\"boarded\":"));
  Serial.print(totalBoardedToday);
  Serial.print(F(",\"alighted\":"));
  Serial.print(totalAlightedToday);
  Serial.print(F(",\"status\":\""));
  if (isOvercrowded) {
    Serial.print(F("OVERCROWDED"));
  } else if (isWarningZone) {
    Serial.print(F("WARNING"));
  } else {
    Serial.print(F("NORMAL"));
  }
  Serial.print(F("\",\"uptime_sec\":"));
  Serial.print(millis() / 1000);
  Serial.println(F("}"));
}

// ----------------- HELPER: TRIGGER TONE ----------------------
void triggerBeep(int frequencyHz, int durationMs) {
  tone(PIN_BUZZER, frequencyHz, durationMs);
}
