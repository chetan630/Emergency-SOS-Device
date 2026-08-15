# Emergency SOS Device

**LPC1768 | ARM Cortex-M3 | Embedded C | I²C | UART | GPIO Interrupt**

An embedded emergency alert prototype built using the **LPC1768 ARM Cortex-M3 microcontroller**. The system uses an **ADXL345 accelerometer** to detect a configured single-tap event and, when triggered, obtains the current GPS location and initiates an SMS alert through a GSM module.

The prototype was built using a general LPC1768 development board with external modules connected using jumper wires. **No custom PCB was designed for this project.**

---

## Features

- Accelerometer-based event detection using **ADXL345**
- Interrupt-driven event detection using the LPC1768 GPIO interrupt
- Real-time system status on a **16×2 LCD**
- GPS location acquisition using a **NEO-6M GPS module**
- GPS NMEA/GPRMC sentence processing
- SMS alert transmission using a **SIM908 GSM module**
- I²C communication for the ADXL345 and LCD
- UART communication for GPS and GSM
- Timer-based delays using LPC1768 Timer0
- Register-level peripheral programming

---

## System Architecture

```text
                         ┌─────────────────┐
                         │     ADXL345     │
                         │  Accelerometer  │
                         └────────┬────────┘
                                  │
                         I²C + Interrupt
                                  │
                                  ▼
                       ┌────────────────────┐
                       │      LPC1768       │
                       │    ARM Cortex-M3   │
                       │                    │
                       │   Embedded C       │
                       └──────┬─────┬───────┘
                              │     │
                       I²C    │     │ UART0
                              │     │
                    ┌─────────┘     └──────────┐
                    ▼                           ▼
             ┌─────────────┐              ┌─────────┐
             │   16×2 LCD  │              │   GPS   │
             │     I²C     │              │  NEO-6M │
             └─────────────┘              └─────────┘

                              UART3
                                │
                                ▼
                         ┌─────────────┐
                         │   SIM908    │
                         │     GSM     │
                         └─────────────┘
                                │
                                ▼
                           SMS Alert
```

---

## Working Principle

The system operates in the following sequence:

```text
Power On
   │
   ▼
Initialize Peripherals
   │
   ├── Timer0
   ├── UART0 → GPS
   ├── I²C0
   ├── ADXL345
   ├── LCD
   └── GPIO Interrupt
   │
   ▼
Normal Monitoring
   │
   ▼
ADXL345 Event
   │
   ▼
GPIO Interrupt
   │
   ▼
Event Flag Set
   │
   ▼
LED ON + LCD "Detected"
   │
   ▼
GPS Data Acquisition
   │
   ▼
Latitude / Longitude Extraction
   │
   ▼
GSM AT Commands
   │
   ▼
SMS Alert
   │
   ▼
Return to Monitoring
```

---

## Hardware

| Component | Purpose | Interface |
|---|---|---|
| LPC1768 | Main microcontroller | — |
| ADXL345 | Event / accelerometer detection | I²C |
| NEO-6M GPS | Location acquisition | UART |
| SIM908 GSM | SMS alert transmission | UART |
| 16×2 LCD | System status display | I²C |
| LED | Event indication | GPIO |

### Hardware Setup

The prototype uses a general **LPC1768 development board** with the external modules connected using jumper wires.

**No custom PCB was designed for this project.**

---

## Pin Configuration

The LPC1768 development board interfaces with external modules via I²C and UART connections. Key pins include:

- **I²C (P0.27, P0.28)** — ADXL345 & LCD
- **UART0 (P0.2, P0.3)** — GPS module
- **GPIO (P0.0, P1.29)** — ADXL345 interrupt & Status LED

📖 **[See detailed pin configuration →](docs/pin-configuration.md)**

---

## Communication Interfaces

The system uses three main communication protocols:

| Interface | Purpose | Details |
|---|---|---|
| **I²C0** | ADXL345 accelerometer & LCD | P0.27 (SDA), P0.28 (SCL) |
| **UART0** | GPS module (NEO-6M) | 9600 baud, NMEA/GPRMC processing |
| **UART3** | GSM module (SIM908) | 9600 baud, AT commands for SMS |

📖 **[See detailed interface specifications →](docs/pin-configuration.md#module-interfaces)**

---

## Event Detection & Processing

The ADXL345 accelerometer is configured to detect a **single-tap interrupt**, which triggers the alert sequence to acquire GPS location and send an SMS alert.

📖 **[See detailed system operation & testing →](docs/project-documentation.md)**

---

## GPS Processing

The GPS module (NEO-6M) provides NMEA sentence data through UART0. The firmware parses the GPRMC sentence to extract latitude and longitude coordinates for the alert message.

📖 **[See GPS interface details →](docs/pin-configuration.md#gps-module)**

---

## Testing & Validation

The system has been functionally tested including ADXL345 event detection, GPS data acquisition, and SMS alert transmission. All components have been verified for proper integration and operation.

📖 **[See detailed test results & procedures →](docs/test-results.md)**

---

## Firmware Implementation

**Single-file implementation:** `src/Emergency_SOS.c`

Organized functional sections:
- Timer, I²C, ADXL345, LCD, GPS, GSM functions
- GPIO interrupt handler
- Main application loop

**Development Environment:** Keil µVision 4 with LPC17xx device support

📖 **[See complete project documentation →](docs/project-documentation.md)**

---

## Repository Structure

```text
Emergency-SOS-Device/
├── README.md                          (project overview)
├── src/
│   └── Emergency_SOS.c               (firmware implementation)
├── keil/
│   └── Emergency_SOS.uvprojx         (Keil µVision project)
├── docs/
│   ├── pin-configuration.md          (hardware pin mapping)
│   ├── project-documentation.md      (system architecture & operation)
│   └── test-results.md               (test cases & validation)
├── datasheets/                       (component datasheets)
└── images/                           (project images)
```

---

## Quick Links

- 📋 [Pin Configuration Details](docs/pin-configuration.md) — Hardware pin mapping and module interfaces
- 📊 [System Documentation](docs/project-documentation.md) — Architecture, operation, and initialization
- ✅ [Test Results & Validation](docs/test-results.md) — Functional test cases and procedures

---

## Development Stack

- **Microcontroller:** LPC1768 (ARM Cortex-M3)
- **Language:** Embedded C
- **IDE:** Keil µVision 4
- **Peripherals:** I²C, UART, GPIO Interrupt, Timer0
- **Components:** ADXL345, NEO-6M GPS, SIM908 GSM, 16×2 LCD
