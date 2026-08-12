# LPC1768 Pin Configuration

## Overview

The Emergency SOS Device uses an LPC1768 ARM Cortex-M3 microcontroller as the main controller. The external modules are connected to the LPC1768 development board using jumper wires.

The firmware uses I²C, UART, GPIO interrupt, and GPIO output interfaces for communication and control.

## Pin Configuration

| LPC1768 Pin | Function                 | Connected Module   | Interface      |
| ----------- | ------------------------ | ------------------ | -------------- |
| P0.27       | SDA0                     | ADXL345 / 16×2 LCD | I²C0           |
| P0.28       | SCL0                     | ADXL345 / 16×2 LCD | I²C0           |
| P0.2        | TXD0                     | GPS module         | UART0          |
| P0.3        | RXD0                     | GPS module         | UART0          |
| P0.0        | External interrupt input | ADXL345 interrupt  | GPIO Interrupt |
| P1.29       | LED output               | Status LED         | GPIO           |

## Module Interfaces

### ADXL345 Accelerometer

The ADXL345 is connected to the LPC1768 through the I²C0 interface.

* **SDA:** P0.27
* **SCL:** P0.28
* **Interrupt:** P0.0
* **Communication:** I²C
* **Interrupt type:** Rising-edge GPIO interrupt

The firmware configures the ADXL345 for single-tap interrupt detection. When the configured interrupt occurs, the LPC1768 interrupt handler sets an event flag for the main application.

### 16×2 LCD

The LCD is interfaced through an I²C interface.

* **SDA:** P0.27
* **SCL:** P0.28
* **I²C address:** `0x27`
* **Communication:** I²C

The LCD is used to display the system state, including normal operation and detected-event status.

### GPS Module

The GPS module is connected through UART0.

* **TXD0:** P0.2
* **RXD0:** P0.3
* **Baud rate:** 9600
* **Communication:** UART

The firmware reads NMEA GPS data and processes the GPRMC sentence to obtain latitude and longitude information.

### GSM / SIM908 Module

The GSM module is controlled through UART3.

* **Communication:** UART3
* **Baud rate:** 9600
* **Purpose:** SMS alert transmission

The firmware sends AT commands to configure the GSM module and transmit an SMS containing the retrieved location information.

### Status LED

A GPIO output is used to indicate the detected-event state.

* **Pin:** P1.29
* **Mode:** GPIO output
* **ON:** Event detected
* **OFF:** Normal operation

## Communication Summary

| Module            | Interface      | LPC1768 Peripheral |
| ----------------- | -------------- | ------------------ |
| ADXL345           | I²C            | I²C0               |
| 16×2 LCD          | I²C            | I²C0               |
| GPS               | UART           | UART0              |
| GSM / SIM908      | UART           | UART3              |
| ADXL345 Interrupt | GPIO Interrupt | EINT3              |
| Status LED        | GPIO           | GPIO1              |

## Hardware Setup

The prototype was assembled using a general LPC1768 development board and external modules connected using jumper wires. No custom PCB was designed for this project.

> **Note:** The pin assignments documented here are based on the current firmware source. Physical wiring should be verified against the actual prototype before making changes to the hardware documentation.
