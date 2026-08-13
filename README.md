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

| LPC1768 Pin | Function | Module |
|---|---|---|
| `P0.27` | SDA0 | ADXL345 / LCD |
| `P0.28` | SCL0 | ADXL345 / LCD |
| `P0.2` | TXD0 | GPS |
| `P0.3` | RXD0 | GPS |
| `P0.0` | GPIO interrupt | ADXL345 |
| `P1.29` | GPIO output | Status LED |

More details are available in [`docs/pin-configuration.md`](docs/pin-configuration.md).

---

## Communication Interfaces

### I²C

I²C0 is used for communication with:

- ADXL345 accelerometer
- 16×2 LCD

The firmware configures:

```text
SDA → P0.27
SCL → P0.28
```

### UART0 — GPS

UART0 is used to receive GPS data.

```text
TXD0 → P0.2
RXD0 → P0.3
Baud Rate → 9600
```

The firmware processes the received NMEA data and extracts latitude and longitude from the GPRMC sentence.

### UART3 — GSM

UART3 is used to communicate with the SIM908 GSM module.

```text
Baud Rate → 9600
```

The firmware uses AT commands to communicate with the GSM module and initiate SMS transmission.

---

## Accelerometer Event Detection

The ADXL345 is configured for a **single-tap interrupt**.

The firmware configures:

- Tap threshold
- Tap duration
- Tap axis
- Interrupt mapping
- Interrupt enable

When the interrupt occurs, the LPC1768 `EINT3_IRQHandler()` clears the interrupt and sets the event flag.

```c
void EINT3_IRQHandler(void)
{
    LPC_GPIOINT->IO0IntClr = (1 << 0);
    tag = 1;
}
```

The main application then detects the event flag and starts the alert sequence.

---

## GPS Processing

The GPS module provides NMEA data through UART0.

The firmware:

1. Waits for the `$` start character.
2. Stores the received sentence.
3. Identifies the GPRMC sentence.
4. Checks the GPS status field.
5. Extracts latitude.
6. Extracts longitude.

Only when the received GPS status is valid does the firmware extract the location information.

---

## GSM Alert

After an event is detected and GPS processing is performed, the firmware communicates with the GSM module using AT commands.

The current firmware sequence includes:

```text
AT
AT+CPIN?
AT+CREG?
AT+CMGF=1
AT+CMGS
```

The location information is then transmitted as part of the SMS sequence.

> **Security note:** Before publishing the source code publicly, replace the emergency phone number currently present in the firmware with a placeholder or configurable value.

---

## LCD Status

The LCD provides basic local system feedback.

### Normal State

```text
Normal
```

### Event Detected

```text
Detected
```

The LCD is controlled through the I²C interface.

---

## Firmware Structure

The current implementation is maintained as a single source file:

```text
src/
└── Emergency_SOS.c
```

The source contains the following functional sections:

```text
Emergency_SOS.c
│
├── Timer Functions
├── I²C Functions
├── ADXL345 Functions
├── LCD Functions
├── GPS Functions
├── GSM Functions
├── GPIO Interrupt Handler
└── Main Application
```

---

## Keil µVision 4 Project

The firmware was developed and tested using **Keil µVision 4**.

The Keil project file is included in the repository:

```text
keil/
└── Emergency_SOS.uvprojx
```

The project can be opened using **Keil µVision 4** with the required LPC17xx device support installed.

---

## Repository Structure

```text
Emergency-SOS-Device/
│
├── README.md
├── .gitignore
│
├── src/
│   └── Emergency_SOS.c
│
├── keil/
│   └── Emergency_SOS.uvprojx
│
├── datasheets/
│   └── Module Datasheets
│
├── docs/
│   ├── pin-configuration.md
│   ├── project-documentation.md
│   └── test-results.md
│
└── images/
    └── Project Photos
```

---

## Testing

The prototype was tested for:

- LPC1768 peripheral initialization
- ADXL345 event detection
- GPIO interrupt operation
- LCD status indication
- GPS UART communication
- GPS location extraction
- GSM UART communication
- SMS alert sequence
- End-to-end event handling

Detailed test information is available in [`docs/test-results.md`](docs/test-results.md).

---

## Development Environment

| Category | Tool / Technology |
|---|---|
| Microcontroller | LPC1768 |
| CPU | ARM Cortex-M3 |
| Programming Language | Embedded C |
| IDE | **Keil µVision 4** |
| Debugging | JTAG / ULINK |
| Communication | I²C, UART |
| Sensor | ADXL345 |
| GPS | NEO-6M |
| GSM | SIM908 |
| Display | 16×2 LCD |

---

## Limitations

- The current firmware is implemented in a single C source file.
- GPS parsing is implemented around the GPRMC sentence format.
- The emergency phone number is currently specified in the firmware and should be replaced before public release.
- The prototype uses jumper-wire connections rather than a custom PCB.
- Event detection is based on the configured ADXL345 single-tap interrupt.

---

## Future Improvements

- Split the firmware into modular `.c` and `.h` driver files.
- Improve GPS NMEA parsing and validation.
- Add configurable emergency contact numbers.
- Add a user cancellation / confirmation mechanism.
- Improve event detection using accelerometer data analysis.
- Add watchdog and fault-recovery mechanisms.
- Design a dedicated PCB for a production-oriented version.
- Improve error handling for GPS and GSM communication failures.

---

## Documentation

- [Pin Configuration](docs/pin-configuration.md)
- [Project Documentation](docs/project-documentation.md)
- [Test Results](docs/test-results.md)

---

## Author

**Chetan Angadi**

Electronics and Communication Engineering

[GitHub](https://github.com/chetan630)
