# 🔌 Hardware & PCB Design

This folder contains the KiCad schematics and PCB layout files for the Fluxgate Sensor Development and Testing Board. The hardware combines the master controller, excitation power electronics, high-speed 24-bit ADC, and a low-noise analog signal processing path into a single 150mm x 100mm PCB.

---

## 📁 Directory Directory

*   📁 **[`Excitation_Circuit_V1/`](file:///c:/Users/ASUS/Desktop/Fluxgate/Fluxgate-Project-Files/Hardware/Excitation_Circuit_V1)**: The initial KiCad prototype.
*   📁 **[`Excitation_Circuit_Final/`](file:///c:/Users/ASUS/Desktop/Fluxgate/Fluxgate-Project-Files/Hardware/Excitation_Circuit_Final)**: The revised KiCad project with improvements in the bootstrap driver and ground isolation.
*   📁 **[`KiCad_Development_Board_JLCPCB/`](file:///c:/Users/ASUS/Desktop/Fluxgate/Fluxgate-Project-Files/Hardware/KiCad_Development_Board_JLCPCB)**: Design optimized for JLCPCB's automated surface mount technology (SMT) assembly service. Contains BOM and component positioning files.

---

## ⚡ Hardware Architecture & Circuit Sub-systems

The Fluxgate Development Board contains four main hardware sub-systems:

### 1. Power Management & Low-Noise Rails
Since magnetic sensors are highly sensitive to power supply noise, a robust voltage regulation and reference system is implemented:
*   **Main Regulator:** AMS1117-3.3 produces the digital 3.3V supply.
*   **Negative Rail Generator:** ME7660C charge-pump voltage converter generates a low-noise $-3.3\text{ V}$ rail from the $3.3\text{ V}$ input.
*   **Precision Analog References:** High-stability AN431 shunt regulators ($20\text{ ppm/°C}$ temp coefficient) generate ultra-stable $+2.5\text{ V}$ and $-2.5\text{ V}$ reference rails for the low-noise op-amps, preventing temperature-induced offset drifts.

```
       [ 5V USB Input ]
              │
      ┌───────┴───────┐
      ▼               ▼
[AMS1117-3.3V]   [ME7660C]
      │               │
  [ +3.3V ]       [ -3.3V ]
      │               │
  [ AN431 ]       [ AN431 ]
      │               │
  [ +2.5V ]       [ -2.5V ]
```

### 2. Excitation Drive (H-Bridge)
To saturate the ferromagnetic core in both directions:
*   **Drivers:** Two half-bridge drivers (LP1111) are configured to form a full H-bridge.
*   **Switching Elements:** Low $R_{DS(on)}$ AP2300 or IRFZ44N N-channel MOSFETs.
*   **Protection:** 1N5400 flyback diodes protect the MOSFETs against massive back-EMF spikes (up to 200A instantaneous rating) when the coil current reverses.
*   **Bootstrap Optimization:** During initial testing, the high-side bootstrap capacitor was initially selected as $1\text{ nF}$, which restricted the maximum gate-on time, leading to switching failures at lower frequencies. This was optimized and corrected to **$1\text{ µF}$** in the final hardware, allowing stable gate-on periods up to $1.125\text{ ms}$.

### 3. Low-Noise Analog Signal Processing
The induced voltage from the pickup (sensing) coil undergoes analog preprocessing to extract the second harmonic component ($2f$):
*   **Differential Input Preamp:** Low-noise TP2311 or LT149x op-amps configured as a $1:2$ gain difference amplifier. By reading the sensing coil differentially, **Common-Mode Noise** coupled from the excitation drive is rejected.
*   **Synchronous Demodulator:** An SN74LVC1G3157 analog SPDT switch acts as a phase-sensitive rectifier. It is toggled by the RP2040 at twice the excitation frequency ($2f$), demodulating the second harmonic directly to DC.
*   **Active Integrator:** A low-pass integrator with an integration bandwidth configured between **159 Hz and 1591 Hz** ($\text{BW} = 1/(2\pi R_f C)$ with $R_f = 100\text{ k}\Omega, C = 10\text{ nF}$).
*   **Output Filter:** An RC low-pass filter with a cutoff frequency $f_c = 28.42\text{ Hz}$ ($C = 100\text{ nF}, R = 56\text{ k}\Omega$) filters out the $50\text{ Hz}$ mains grid noise and lingering switching ripples.

### 4. Precision Digitization & Processing
*   **MSP430AFE253:** Reads the filtered DC level (or the pre-demodulated high-speed AC signal) using its internal **24-bit Sigma-Delta ADC**. It runs on a $12\text{ MHz}$ internal DCO and transmits over UART at **1.5M Baud**.
*   **RP2040 (Raspberry Pi Pico):** Controls H-Bridge timing and routes the MSP430's digitized telemetry to a computer via USB virtual serial or a UDP socket over Wi-Fi.

---

## 📋 Component Checklist (BOM)

If assembling the board manually, ensure the following core components are ready:

| Designator | Component Name | Package | Description |
| :--- | :--- | :--- | :--- |
| **U5** | Raspberry Pi Pico | Module | Master controller and PWM driver |
| **U4** | MSP430AFE253IPW | TSSOP-24 | 24-Bit Sigma-Delta ADC / High-Speed DSP |
| **U1, U2** | LP1111 (or HIP4082) | SOIC-8 | Half-Bridge Bootstrap MOSFET Gate Drivers |
| **Q1 - Q4** | AP2300 (or IRFZ44N) | SOT-23 / TO-220 | Excitation H-Bridge Switching MOSFETs |
| **U3, U8, U9** | TP2311 (or LT1492) | SOIC-8 | $12\text{ nV}/\sqrt{\text{Hz}}$ Low-Noise Precision Op-Amps |
| **U12** | SN74LVC1G3157 | SOT-363 | High-Speed SPDT Analog Switch (Demodulator) |
| **U10, U11** | AN431 | SOT-23 | High-Precision Shunt Voltage Regulator (2.5V) |
| **U6** | ME7660C | SOIC-8 | Switched-Capacitor Negative Voltage Converter |
| **U7** | AMS1117-3.3 | SOT-223 | $3.3\text{ V}$ Low-Dropout Linear Regulator |
| **D1 - D4** | 1N5400 (or 1N4148) | DO-201AD | 200A Flyback Back-EMF Protection Diodes |
