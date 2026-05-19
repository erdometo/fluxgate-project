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
4.  **SD24_A Sigma-Delta ADC Register-Level Setup:**
    *   **Voltage Reference:** Set to use the internal precision $1.2\text{ V}$ bandgap reference.
    *   **Channel Grouping (`SD24GRP`):** Configures Channel 0 (Ch0) and Channel 1 (Ch1) in Group Conversion Mode, ensuring that conversions are initiated simultaneously across both analog channels to prevent phase lag during dual-axis or differential sensing.
    *   **LSB Formatting (`SD24LSBACC`):** Set to format the output alignment, allowing the firmware to extract the lower bits directly from the conversion registers.
    *   **Register Mapping:**
        *   `SD24CTL`: Configures the ADC clock source as SMCLK ($12\text{ MHz}$).
        *   `SD24INCTLx`: Controls input channel routing (Ch0/Ch1).
        *   `SD24GAINx`: Selects analog pre-amplifier gain settings.
        *   `SD24CCTLx`: Sets conversion options (Oversampling Ratio, interrupts).
        *   `SD24MEMx`: Stores raw ADC results.
    *   *Operation:* Inside the high-priority interrupt routine (`SD24_ISR`), the conversion memory registers (`SD24MEMx`) are read, formatted, split into bytes, and immediately written to the USART0 TX buffer to keep the stream gapless.

---

## 🎛️ RP2040 (Raspberry Pi Pico) Firmware Architecture

The RP2040 serves as the master coordinator. It handles two critical tasks:
1.  Generating ultra-stable, complementary PWM drive signals for the excitation circuit.
2.  Capturing the 1.5MBaud UART stream from the MSP430 and forwarding it to the PC.

### 1. Excitation PWM Synthesis
The RP2040 uses its hardware PWM slice to generate two out-of-phase drive signals (`in1` and `in2`) and a global `enable` gate. To prevent high-side gate charge depletion on the LP1111 half-bridge drivers, the excitation waveform requires a specific duty pattern. Instead of a basic symmetrical square wave, the RP2040 generates a triple-phase excitation sequence:
*   **Positive Phase ($0.8\text{ ms}$):** Emits high-speed active pulses of $80\text{ µs}$ on `in1` separated by $40\text{ µs}$ dead-times.
*   **Neutral Recharge Phase ($0.4\text{ ms}$):** The H-bridge is completely disabled (`enable` low). This gives the high-side bootstrap capacitors a dedicated window to fully recharge from the $+5\text{ V}$ supply.
*   **Negative Phase ($0.8\text{ ms}$):** Emits active $80\text{ µs}$ pulses on `in2` separated by $40\text{ µs}$ dead-times.

This triple-phase architecture ($0.8\text{ ms} / 0.4\text{ ms} / 0.8\text{ ms}$) guarantees that the high-side bootstrap gates never collapse, keeping the excitation stage perfectly stable.

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
