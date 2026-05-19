# ⚡ LTSpice Simulations

This directory contains the LTSpice simulation files used to model, analyze, and optimize both the power excitation stage and the analog signal conditioning path prior to committing to PCB manufacturing.

---

## 📁 Key Simulation Files

*   📉 **[`fluxgate.asc`](file:///c:/Users/ASUS/Desktop/Fluxgate/Fluxgate-Project-Files/Simulations/LTSpice/fluxgate.asc)**: Main system model containing the MOSFET H-bridge driver, flyback diodes, and dual-coil mutual inductance model.
*   📉 **[`fluxgateACAnaliz.asc`](file:///c:/Users/ASUS/Desktop/Fluxgate/Fluxgate-Project-Files/Simulations/LTSpice/fluxgateACAnaliz.asc)**: AC frequency response analysis of the active integration filter and low-pass output stage.
*   📉 **[`driver.asc`](file:///c:/Users/ASUS/Desktop/Fluxgate/Fluxgate-Project-Files/Simulations/LTSpice/driver.asc)**: Isolated gate-driver simulation modeling half-bridge switching dynamics and bootstrap capacitor charging cycles.
*   📉 **[`Draft1.asc` to `Draft7.asc`](file:///c:/Users/ASUS/Desktop/Fluxgate/Fluxgate-Project-Files/Simulations/LTSpice)**: Incremental design drafts representing the evolution of the synchronous phase detector, low-pass filter coefficients, and feedback networks.

---

## 🎛️ Circuit Modeling & Analysis

LTSpice was used to simulate two primary engineering domains: the **Power Excitation Circuit** and the **Demodulator/Filter Conditioning Path**.

### 1. Excitation Circuit Simulation (`fluxgate.asc`, `driver.asc`)
*   **Purpose:** To verify that the H-bridge generates a clean AC staircase voltage across the drive bobbin without inducing excessive overshoot, rings, or cross-conduction.
*   **The H-Bridge Model:** Utilizes four N-Channel MOSFETs (IRFZ44N) driven by complementary gate pulses with a $40\text{ µs}$ dead-time. The load is modeled as a primary drive inductor ($L_1 = 1.3\text{ mH}$) with a series winding resistance ($R_1 = 3\text{ }\Omega$).
*   **Back-EMF Suppression:** Model includes 1N5400 power diodes. Transient simulation (`.tran 0 0.01 0`) verifies that when the H-Bridge switches off, the inductor's magnetic energy discharges safely through the flyback path into the supply rails, clamping the maximum voltage stress to safe levels.
*   **Bootstrap Capacitor Dynamics:** Modeled the high-side gate-source voltage ($V_{GS}$) during switching. The simulation validated that a $1\text{ nF}$ bootstrap capacitor discharged too quickly, causing the high-side MOSFET to drop out of conduction early. Increasing this value to **$1\text{ µF}$** in simulation ensured the gate voltage remained stable during the entire $1.125\text{ ms}$ pulse width.

```
       +5V Supply
         │
    ┌────┴────┐
   [Q1]      [Q3]
    │──(L1)───│  <-- Winding Inductor (1.3mH)
   [Q2]      [Q4]
    └────┬────┘
         ▼ Ground
```

### 2. Analog Phase-Sensitive Demodulation (`fluxgateACAnaliz.asc`, `Draft7.asc`)
*   **Purpose:** To verify the rectification of the second harmonic ($2f$) signal and the removal of switching spikes.
*   **Demodulator Switching Model:** Implements a voltage-controlled switch component (`SW`) controlled by a square-wave source operating at twice the excitation frequency ($2f = 25\text{ kHz}$). This represents the SN74LVC1G3157 SPDT analog switch.
*   **Frequency Response Analysis (`.ac dec 100 1 10k`):**
    *   Verifies the active integrator's frequency response. The integration region is mathematically defined by:
        $$f_{a} = \frac{1}{2\pi R_f C} = \frac{1}{2\pi \cdot 100\text{ k}\Omega \cdot 10\text{ nF}} \approx 159\text{ Hz}$$
        and the upper cutoff limit is $1591.5\text{ Hz}$. This bandpass-like integration behavior restricts out-of-band high-frequency noise while actively capturing the $2f$ component.
    *   Validates the RC output low-pass filter:
        $$f_{c} = \frac{1}{2\pi R C} = \frac{1}{2\pi \cdot 56\text{ k}\Omega \cdot 100\text{ nF}} \approx 28.42\text{ Hz}$$
        This filter exhibits sharp attenuation above $28.4\text{ Hz}$, which blocks the $50\text{ Hz}$ AC power grid noise from coupling into the analog ADC acquisition rails.
