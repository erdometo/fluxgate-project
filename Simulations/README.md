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
*   **Gate Driver Substitution Limitation:** 
    Because a SPICE model for the specific **Harris/Intersil/Renesas HIP4082** full-bridge gate driver was unavailable in standard LTSpice libraries, the simulation was modeled using **two LT1162 half-bridge gate drivers** connected in a complementary configuration to form a complete full H-bridge excitation stage.
*   **H-Bridge Load Impedance Modeling:** 
    The H-bridge utilizes four N-channel MOSFETs (IRFZ44N/AP2300 series) driven by complementary gate pulses with a $40\text{ µs}$ dead-time. The load is modeled as a primary excitation inductor in series with its winding resistance:
    $$Z_{\text{load}}(s) = sL_1 + R_1$$
    where the primary excitation coil parameters are defined as $L_1 = 1.3\text{ mH}$ and series winding resistance $R_1 = 3\text{ }\Omega$.
*   **Back-EMF Suppression:** 
    The circuit model includes high-current 1N5400 flyback power diodes. Transient simulation (`.tran 0 0.01 0`) verifies that when the H-Bridge switches off, the inductor's magnetic energy discharges safely through the flyback path into the supply rails, clamping the maximum voltage stress to safe levels.
*   **Bootstrap Capacitor Dynamics:** 
    The gate-source bootstrap charging process is simulated. A bootstrap capacitor $C_{Bst}$ must supply the gate charge $Q_{gate}$ without dropping below a tolerable gate voltage dip $\Delta V_{Bst}$:
    $$C_{Bst} = \frac{Q_{gate}}{\Delta V_{Bst}}$$
    For $Q_{gate} = 11\text{ nC}$ (AP2300/IRFZ44N class) and $\Delta V_{Bst} = 0.1\text{ V}$, the absolute minimum capacitance is $110\text{ nF}$. The initial physical footprint error populated a $1\text{ nF}$ capacitor, which resulted in complete gate-source voltage collapse within microseconds in simulation. Populating a $1\text{ µF}$ capacitor stabilized the high-side gate-drive voltage, keeping the channel open for a maximum of $1.125\text{ ms}$ under load.

```
       +5V Supply
         │
    ┌────┴────┐
   [Q1]      [Q3]
    │──(L1)───│  <-- Winding Inductor (1.3mH, 3Ω)
   [Q2]      [Q4]
    └────┬────┘
         ▼ Ground
```

### 2. Analog Phase-Sensitive Demodulation (`fluxgateACAnaliz.asc`, `Draft7.asc`)
*   **Purpose:** To verify the rectification of the second harmonic ($2f$) signal and the removal of switching spikes.
*   **Demodulator Switching Model:** Implements a voltage-controlled switch component (`SW`) controlled by a square-wave source operating at twice the excitation frequency ($2f = 25\text{ kHz}$). This represents the SN74LVC1G3157 SPDT analog switch.
*   **Active Integrator Frequency Response:**
    The active integrator (with feedback resistor $R_f = 100\text{ k}\Omega$ and feedback capacitor $C = 10\text{ nF}$) has a parallel RC feedback path. The closed-loop transfer function is given by:
    $$H(s) = -\frac{Z_f(s)}{Z_{in}(s)} = -\frac{R_f}{R_{in} (1 + s R_f C)} = -\frac{R_f / R_{in}}{1 + s R_f C}$$
    *   **Low-Frequency Limit (Pole Frequency):**
        $$f_L = \frac{1}{2\pi R_f C} = \frac{1}{2\pi \cdot 100\text{ k}\Omega \cdot 10\text{ nF}} \approx 159.15\text{ Hz}$$
        Below this frequency ($f < f_L$), the capacitor behaves as an open circuit, and the integrator acts as a standard inverting amplifier with a fixed DC gain of $-R_f / R_{in} = -10$ (for $R_{in} = 10\text{ k}\Omega$).
    *   **High-Frequency Limit (Unity-Gain Frequency):**
        $$f_H = \frac{1}{2\pi R_{in} C} = \frac{1}{2\pi \cdot 10\text{ k}\Omega \cdot 10\text{ nF}} \approx 1591.55\text{ Hz}$$
        Above $f_H$, the gain drops below $0\text{ dB}$, limiting high-frequency noise amplification. The integration region is thus strictly bounded inside the bandpass-like region of $[159.2\text{ Hz}, 1591.5\text{ Hz}]$.
    *   **Transient Time Integration Limit:**
        For perfect integration of transient magnetic pulses without saturation, the integration time window $T$ must span at least five time constants of the feedback loop:
        $$T \ge 5 \tau = 5 R_f C = 5 \cdot 100\text{ k}\Omega \cdot 10\text{ nF} = 5\text{ ms}$$
        This time-domain limit corresponds to a minimum integration frequency of:
        $$f_{\text{min}} = \frac{1}{T} \le 200\text{ Hz}$$
*   **RC Output Low-Pass Filter Transfer Function:**
    The filtered signal is further smoothed using a passive RC low-pass filter:
    $$H_{\text{LPF}}(s) = \frac{1}{1 + s R_2 C_2}$$
    where $R_2 = 56\text{ k}\Omega$ and $C_2 = 100\text{ nF}$.
    *   **Cut-Off Frequency:**
        $$f_c = \frac{1}{2\pi R_2 C_2} = \frac{1}{2\pi \cdot 56\text{ k}\Omega \cdot 100\text{ nF}} \approx 28.42\text{ Hz}$$
    *   **AC Grid Noise Rejection:**
        This low cut-off frequency provides heavy attenuation for frequencies at and above $50\text{ Hz}$ (specifically, $\approx -10\text{ dB}$ at $50\text{ Hz}$ and $\approx -22\text{ dB}$ at $100\text{ Hz}$), which serves as a powerful shield rejecting local $50\text{ Hz}$ AC mains power grid radiation.

