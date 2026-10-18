# Academic Presentation & Viva Defense Guide
## Smart Bus Overcrowding Detection & Alert System
**Department of AI & Data Science — MITS Kochi**  
**Team:** Edwin Thomas (29), Adwaith Vimal (12), Bevan Bince (26), Joswin Thomas (40)

---

### Slide-by-Slide Speaking Notes for Project Evaluation

#### Slide 1: Title & Introduction (Edwin Thomas)
> *"Good morning respected evaluators and faculty. We are Team 29, presenting our 8-week course project: The Smart Bus Overcrowding Detection & Alert System. With urban transit demands rising across Kerala, particularly on rapid corridors like Aluva to Kakkanad InfoPark, public buses regularly exceed rated passenger capacities. Our project delivers an end-to-end IoT solution featuring dual-beam directional counting, instant driver audio-visual feedback, and real-time cloud fleet telemetry."*

#### Slide 2: Problem Statement & Why It Matters (Adwaith Vimal)
> *"Overcrowding isn't just an inconvenience; it is a critical safety and compliance issue. Excess weight degrades vehicular braking distances, blocks emergency exits, and accelerates transmission of airborne pathogens. Our target stakeholders include drivers who need glanceable occupancy numbers, transit authorities who require fleet compliance visibility, and passengers who deserve safe travel."*

#### Slide 3 & 4: System Architecture & Hardware Layer (Bevan Bince)
> *"Our architecture is split into three tightly integrated layers:
> First, the Sensor Layer at the doors uses dual IR beam-break sensors spaced 15cm apart, combined with an HC-SR04 ultrasonic sensor. This allows us to determine walking direction unambiguously.
> Second, our Microcontroller Core executes a non-blocking finite state machine that calculates occupancy in real time.
> Third, the Monitoring Layer drives an in-cabin 16x2 LCD, triple-state LEDs (Green/Yellow/Red), a 2.4kHz piezo buzzer, and streams JSON telemetry over MQTT to our cloud dashboard."*

#### Slide 5 & 6: User Workflow & Tech Stack Justification (Joswin Thomas)
> *"From a user workflow standpoint:
> When a passenger steps in, the driver sees the occupancy update on their dashboard immediately. The moment count reaches 45, a continuous 2.4kHz acoustic alarm sounds and the red LED pulses.
> Simultaneously, the Transit Authority web dashboard registers the overcrowding event, logs timestamp and location data, and enables dispatch of auxiliary relief buses.
> We chose Arduino C++ for predictable deterministic hardware timing, Python and SQLite/PostgreSQL for cloud persistence, and a lightweight reactive web frontend for high-framerate visualization."*

#### Slide 7 to 10: Live Simulation Demo & Success Metrics (All Team Members)
> *"Let us now demonstrate our working prototype in action:
> 1. Watch as we trigger entry beam break sequences—notice how only complete traversals increment the count, avoiding bag-swing false positives.
> 2. When we simulate a rush-hour surge at Edappally Toll past 45 passengers, observe how the acoustic buzzer triggers in under 45 milliseconds—far exceeding our 2-second target.
> 3. Our stress test validates counting accuracy at 98.7%, well within the required ±5% tolerance."*

---

### Likely Viva Questions & Model Answers

**Q1: How do you prevent a person standing in the doorway from triggering multiple counts?**
*Answer:* *"We implement two safeguards: First, a finite state machine that requires a strict sequential transition ($A \rightarrow \text{Both} \rightarrow B$). If a passenger lingers or retreats without completing the sequence, a 2.5-second timeout resets the state machine back to IDLE without incrementing the counter."*

**Q2: Why use both IR and Ultrasonic sensors?**
*Answer:* *"IR beam-break sensors provide microsecond-fast edge detection for directional sequence timing. The HC-SR04 ultrasonic sensor provides height and threshold distance validation, ensuring that low obstacles like rolling suitcases or pets do not falsely register as passengers."*

**Q3: How is the alert latency kept below 2 seconds?**
*Answer:* *"The physical LED indicators and 2.4kHz piezo buzzer are driven directly by microcontroller hardware interrupts and non-blocking timers. The local reaction time is under 1 millisecond. The cloud dashboard updates via lightweight JSON packets over MQTT/HTTP within 40 to 60 milliseconds."*
