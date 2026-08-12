# Emergency SOS Device — Project Documentation

## 1. Project Overview

The Emergency SOS Device is an embedded safety system built around the **LPC1768 ARM Cortex-M3 microcontroller**.

The system monitors an ADXL345 accelerometer for a configured single-tap event. When the interrupt is detected, the LPC1768 activates the alert sequence, retrieves the current GPS location, and sends an SMS through a GSM module.

A 16×2 LCD and status LED provide local indication of the system state.

## 2. System Components

### Main Controller

* LPC1768 ARM Cortex-M3 microcontroller
* Keil µVision-based firmware development
* Register-level peripheral programming

### Sensors and Modules

* **ADXL345** — accelerometer and event detection
* **NEO-6M GPS** — location acquisition
* **SIM908 GSM** — SMS alert transmission
* **16×2 LCD** — system status display
* **Status LED** — event indication

## 3. System Operation

The firmware follows the following sequence:

```text
System Initialization
        │
        ▼
Initialize LPC1768 Peripherals
        │
        ├── Timer0
        ├── UART0 → GPS
        ├── I²C0
        ├── ADXL345
        ├── LCD
        └── GPIO Interrupt
        │
        ▼
      Normal
        │
        ▼
Monitor ADXL345 Interrupt
        │
        ├───────────────┐
        │               │
     No Event       Event Detected
        │               │
        │               ▼
        │          LED ON + LCD
        │               │
        │               ▼
        │          Read GPS Data
        │               │
        │               ▼
        │        Extract Location
        │               │
        │               ▼
        │         Send GSM SMS
        │               │
        └───────────────┘
                │
                ▼
          Return to Normal
```

## 4. Initialization

After system startup, the firmware initializes the required LPC1768 peripherals.

The initialization includes:

1. Timer0
2. UART0 for GPS
3. I²C0
4. ADXL345
5. ADXL345 single-tap configuration
6. 16×2 LCD
7. GPIO interrupt
8. Status LED GPIO

The GPIO interrupt is then enabled through the NVIC.

## 5. Event Detection

The ADXL345 is configured for a single-tap interrupt.

The firmware configures:

* Tap threshold
* Tap duration
* Tap axis selection
* Interrupt mapping
* Interrupt enable

The ADXL345 interrupt is connected to an LPC1768 GPIO interrupt input.

When the interrupt occurs, `EINT3_IRQHandler()` clears the GPIO interrupt and sets the global `tag` flag.

```c
void EINT3_IRQHandler(void)
{
    LPC_GPIOINT->IO0IntClr = (1<<0);
    tag = 1;
}
```

The main loop checks this flag to determine whether an event has been detected.

## 6. Alert Sequence

When `tag == 1`, the firmware:

1. Turns on the status LED.
2. Displays `"Detected"` on the LCD.
3. Waits for the configured delay.
4. Reads GPS data.
5. Extracts latitude and longitude.
6. Sends an SMS using the GSM module.
7. Clears the event flag.
8. Clears the ADXL345 interrupt source.

If no event is detected, the system displays `"Normal"` and keeps monitoring the accelerometer.

## 7. GPS Location Acquisition

The GPS module communicates with the LPC1768 through **UART0 at 9600 baud**.

The firmware waits for the beginning of an NMEA sentence and searches for the `$` character.

It then stores the received sentence and checks the GPRMC status field.

A valid status is represented by:

```text
A = Valid
V = Invalid
```

When the GPS data is considered valid, the firmware extracts latitude and longitude from the received sentence.

## 8. GSM SMS Transmission

The GSM module is controlled through **UART3 at 9600 baud**.

The firmware sends AT commands to:

1. Check communication with the GSM module.
2. Check SIM status.
3. Check network registration.
4. Configure SMS text mode.
5. Select the destination number.
6. Transmit the GPS location information.

The SMS transmission is completed using the GSM module's SMS command sequence.

## 9. LCD Status Display

The 16×2 LCD is connected through the I²C interface.

The firmware displays:

```text
Normal
```

during normal monitoring.

When an event is detected:

```text
Detected
```

is displayed.

## 10. Communication Interfaces

| Interface      | Purpose                       |
| -------------- | ----------------------------- |
| I²C0           | ADXL345 and LCD communication |
| UART0          | GPS communication             |
| UART3          | GSM communication             |
| GPIO Interrupt | ADXL345 event notification    |
| GPIO           | Status LED                    |

## 11. Firmware Structure

The current implementation is contained in a single C source file:

```text
src/
└── Emergency_SOS.c
```

The source contains separate function groups for:

* Timer functions
* I²C communication
* ADXL345 configuration
* LCD control
* GPS processing
* GSM communication
* GPIO interrupt handling
* Main application logic

This keeps the original project implementation intact while documenting the functional sections separately.

## 12. Hardware Prototype

The system was implemented using a general LPC1768 development board with the external modules connected using jumper wires.

No custom PCB was designed for this project.

The hardware arrangement was intended primarily for firmware development, module interfacing, and functional testing.

## 13. Limitations

The current firmware implementation has the following limitations:

* The project uses a single source file rather than separate driver modules.
* GPS parsing is implemented specifically around the received GPRMC sentence format.
* The GSM destination number is currently specified in the firmware.
* The prototype uses jumper-wire connections rather than a custom PCB.
* The alert logic is based on the configured ADXL345 single-tap interrupt.

## 14. Future Improvements

Possible improvements include:

* Separate peripheral drivers into `.c` and `.h` modules.
* Improve GPS NMEA parsing and validation.
* Add configurable emergency contact numbers.
* Add a user confirmation/cancel mechanism before sending an alert.
* Improve event detection using accelerometer data analysis.
* Add watchdog and fault-recovery mechanisms.
* Design a dedicated PCB for a final hardware version.
