# 💾 Firmware Guide

This folder contains the source code, project workspace configurations, and build directories for the dual-MCU system powering the Fluxgate magnetometer: the **Texas Instruments MSP430AFE253** and the **Raspberry Pi Pico (RP2040)**.

---

## 📁 Directory Structure

*   📁 **[`MSP430_ADC_to_UART/`](file:///c:/Users/ASUS/Desktop/Fluxgate/Fluxgate-Project-Files/Firmware/MSP430_ADC_to_UART)**: IAR project that configures the SD24 ADC module and transmits data via UART.
*   📁 **[`MSP430_AFE_ADC/`](file:///c:/Users/ASUS/Desktop/Fluxgate/Fluxgate-Project-Files/Firmware/MSP430_AFE_ADC)**: Standard MSP430 C firmware implementing oversampled 24-bit analog data acquisition.
*   📁 **[`MSP430_AFE_Test/`](file:///c:/Users/ASUS/Desktop/Fluxgate/Fluxgate-Project-Files/Firmware/MSP430_AFE_Test)**: Verification routines for validating the MCU clock and peripheral subsystems.
*   📁 **[`MSP430_SigmaDelta_Demo/`](file:///c:/Users/ASUS/Desktop/Fluxgate/Fluxgate-Project-Files/Firmware/MSP430_SigmaDelta_Demo)**: Code Composer Studio (CCS) project template for the Sigma-Delta ADC.
*   📁 **[`MSP430_UART_Demo/`](file:///c:/Users/ASUS/Desktop/Fluxgate/Fluxgate-Project-Files/Firmware/MSP430_UART_Demo)**: Isolated USART test code used to push the baud-rate boundaries.
*   📁 **[`Pico_Firmwares/`](file:///c:/Users/ASUS/Desktop/Fluxgate/Fluxgate-Project-Files/Firmware/Pico_Firmwares)**:
    *   **`pico-custom-firmware-usb`**: C/C++ SDK project routing data via high-speed USB CDC.
    *   **`pico-custom-firmware-udp`**: C/C++ SDK project streaming data over Wi-Fi UDP.

---

## 🧠 MSP430AFE253 Firmware Architecture

The MSP430's primary role is high-speed, high-resolution data acquisition using its integrated **SD24_A 24-bit Sigma-Delta ADC**.

```
    [Analog Input]
          │
          ▼
   [SD24 Channel 0/1] ──► [SD24_ISR (Interrupt)]
                                │ (Read 24-bit Conversion)
                                ▼
                       [USART0 TX Buffer] ──► [1.5 MBaud TX Pin]
```

### Key Configurations
1.  **Watchdog Timer:** Proactively disabled at power-on (`WDTCTL = WDTPW + WDTHOLD`) to prevent unexpected resets during clock configuration.
2.  **Clock System Calibration:**
    *   By default, the MSP430 runs on an internal Digitally Controlled Oscillator (DCO) at $1\text{ MHz}$.
    *   To support high-speed UART transmission without frame errors, the firmware calibrates the DCO to **$12\text{ MHz}$** using factory-stored calibration constants:
        ```c
        BCSCTL1 = CALBC1_12MHZ;
        DCOCTL = CALDCO_12MHZ;
        ```
3.  **High-Speed USART0 Setup:**
    *   Configured in asynchronous UART mode.
    *   Baud rate set to **1,500,000 Baud** (1.5M Baud) using the $12\text{ MHz}$ sub-main clock (SMCLK) as the clock source. This extreme baud rate is required to stream two channels of oversampled 24-bit data in real-time.
4.  **SD24_A Sigma-Delta ADC Initialization:**
    *   Uses the internal $1.2\text{ V}$ bandgap voltage reference.
    *   Configures Channel 0 (Ch0) and Channel 1 (Ch1) in **Group Conversion Mode**, ensuring both channels sample simultaneously.
    *   Inside the SD24 interrupt service routine (`SD24_ISR`), the conversion results are read via the `SD24MEMx` registers, formatted using `SD24LSBACC` to extract the correct bits, split into bytes, and immediately pushed to the USART0 TX buffer.

---

## 🎛️ RP2040 (Raspberry Pi Pico) Firmware Architecture

The RP2040 serves as the master coordinator. It handles two critical tasks:
1.  Generating ultra-stable, complementary PWM drive signals for the excitation circuit.
2.  Capturing the 1.5MBaud UART stream from the MSP430 and forwarding it to the PC.

### 1. Excitation PWM Synthesis
The RP2040 uses its hardware PWM slice to generate two out-of-phase drive signals (`in1` and `in2`) and a global `enable` gate:
*   **Drive Signal 1 (`in1`):** Active-high pulse of $80\text{ µs}$ followed by a low state.
*   **Drive Signal 2 (`in2`):** Phase-shifted by $180\text{°}$, active-high pulse of $80\text{ µs}$.
*   **Dead-time (`enable`):** Introduces a $40\text{ µs}$ dead-time between switching events, disabling the H-bridge to prevent cross-conduction (shoot-through currents) in the MOSFET legs.

### 2. High-Speed Telemetry Ingestion (PIO)
Standard microcontroller hardware UART engines can struggle to ingest continuous, gapless data streams at 1.5M Baud without dropping bits or starving the CPU.
*   The RP2040 solves this by using its integrated **Programmable I/O (PIO) blocks**.
*   A custom PIO state machine is loaded to act as a high-speed, hardware-level UART RX receiver on **GPIO10** (connected to MSP430's TXD pin).
*   The PIO automatically samples the incoming bitstream, decodes the serial frames, and writes the assembled bytes directly into a hardware FIFO.

### 3. PC Data Forwarding Paths
*   **USB Virtual Serial Mode (`pico-custom-firmware-usb`):**
    The firmware initializes a USB CDC (virtual serial port) interface using the **TinyUSB** stack. Bytes read from the PIO FIFO are pushed directly into the USB buffer, which appears on the PC as a Virtual COM port.
*   **Wi-Fi UDP Mode (`pico-custom-firmware-udp`):**
    If using a Raspberry Pi Pico W, the firmware initializes the CYW43439 Wi-Fi chip, connects to the local wireless network, and bundles the incoming ADC byte stream into **UDP Packets**. These packets are streamed wirelessly over the network to the PC's IP address, eliminating all physical connections to the sensor during measurement (greatly reducing electromagnetic interference).
