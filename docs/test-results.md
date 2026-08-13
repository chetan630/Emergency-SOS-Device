# Emergency SOS Device — Test Results

## 1. Testing Overview

The Emergency SOS Device was tested as an integrated embedded system to verify communication between the LPC1768 and the connected sensor and communication modules.

The testing focused on:

* ADXL345 event detection
* GPIO interrupt operation
* LCD status indication
* GPS data reception
* GPS location extraction
* GSM module communication
* SMS alert transmission
* Overall event-to-alert sequence

## 2. Functional Test Cases

| Test | Test Condition                         | Expected Result                           | Status |
| ---- | -------------------------------------- | ----------------------------------------- | ------ |
| 1    | Power on the LPC1768 system            | System initializes peripherals            | Tested |
| 2    | No accelerometer event                 | LCD displays `Normal` and LED remains OFF | Tested |
| 3    | Trigger configured ADXL345 event       | GPIO interrupt is generated               | Tested |
| 4    | Event detected                         | LED turns ON and LCD displays `Detected`  | Tested |
| 5    | GPS data available                     | GPS sentence is received through UART0    | Tested |
| 6    | Valid GPRMC sentence received          | Latitude and longitude are extracted      | Tested |
| 7    | GSM module connected                   | AT commands are transmitted through UART3 | Tested |
| 8    | Valid event and GPS location available | SMS alert sequence is initiated           | Tested |

## 3. Event Detection Test

The ADXL345 was configured to generate an interrupt for a single-tap event.

When the interrupt occurs:

```text
ADXL345
   │
   │ Interrupt
   ▼
LPC1768 GPIO
   │
   ▼
EINT3_IRQHandler()
   │
   ▼
tag = 1
   │
   ▼
Alert Sequence
```

The firmware clears the GPIO interrupt and sets the `tag` flag inside the interrupt handler.

## 4. LCD Test

The LCD was tested for system-state indication.

### Normal Operation

```text
Normal
```

### Event Detection

```text
Detected
```

The LCD is controlled through the LPC1768 I²C0 interface.

## 5. GPS Test

The GPS module was tested through UART0.

The firmware waits for an NMEA sentence beginning with `$`, stores the received sentence, and checks the GPRMC status field.

For valid GPS data, the firmware extracts:

* Latitude
* Longitude

The extracted values are then passed to the GSM transmission routine.

## 6. GSM Test

The GSM module was tested through UART3 using AT commands.

The firmware performs the following sequence:

```text
AT
 ↓
AT+CPIN?
 ↓
AT+CREG?
 ↓
AT+CMGF=1
 ↓
AT+CMGS
 ↓
Location Data
 ↓
SMS Transmission
```

The SMS transmission is triggered after the configured accelerometer event and GPS processing sequence.

## 7. End-to-End Test

The complete functional sequence was tested as follows:

```text
Event Trigger
     ↓
ADXL345 Interrupt
     ↓
LPC1768 Interrupt Handler
     ↓
LED ON
     ↓
LCD: "Detected"
     ↓
GPS Location Acquisition
     ↓
Latitude / Longitude Extraction
     ↓
GSM AT Commands
     ↓
SMS Alert
```

The firmware then clears the event flag and ADXL345 interrupt source and returns to the monitoring state.

## 8. Performance Results

| Parameter               |                  Result                  |
| ----------------------- | ---------------------------------------- |
| Event detection latency |                  <300ms                  |
| GPS position accuracy   |                  ~5-15m                  |
| SMS transmission time   |                  <15sec                  |
| GPS UART baud rate      |                 9600 baud                |
| GSM UART baud rate      |                 9600 baud                |
| ADXL345 data rate       |                  100 Hz                  |

## 9. Test Environment

**Controller:** LPC1768 ARM Cortex-M3

**Development Environment:** Keil µVision 4

**Hardware Setup:** LPC1768 development board with external modules connected using jumper wires

**Connected Modules:**

* ADXL345 accelerometer
* NEO-6M GPS
* SIM908 GSM
* 16×2 LCD

## 10. Conclusion

The functional testing verified the main firmware sequence from accelerometer event detection through GPS location acquisition and GSM SMS transmission.

The prototype demonstrates the integration of multiple embedded peripherals and communication interfaces on the LPC1768 platform.
