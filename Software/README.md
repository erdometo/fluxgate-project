# 💻 Software, GUI & Analytics

This directory contains the Python-based real-time graphical user interfaces (GUIs), data plotting dashboards, and Matlab analytical scripts used to process, log, and visualize the digitized Fluxgate sensor telemetry.

---

## 📁 Key Software Files

*   🐍 **[`compass.py`](file:///c:/Users/ASUS/Desktop/Fluxgate/Fluxgate-Project-Files/Software/Python_GUI_Demo/compass.py)**: Python client that maps the parsed magnetic field intensity to a graphical rotating compass needle (`needle.png` and `dial.png`).
*   🐍 **[`pyqtgraph_float_v2x2_yesim.py`](file:///c:/Users/ASUS/Desktop/Fluxgate/Fluxgate-Project-Files/Software/Python_GUI_Demo/pyqtgraph_float_v2x2_yesim.py)**: Real-time, multi-channel oscilloscope client built using **PyQtGraph** for rendering the high-speed (16.5 kSps) 24-bit ADC data.
*   🐍 **[`udp_plot_pyqt.py`](file:///c:/Users/ASUS/Desktop/Fluxgate/Fluxgate-Project-Files/Software/Python_GUI_Demo/udp_plot_pyqt.py)**: Original Python plotting script for UDP data streams.
*   🐍 **[`udp_plot_pyqt_2.py`](file:///c:/Users/ASUS/Desktop/Fluxgate/Fluxgate-Project-Files/Software/Python_GUI_Demo/udp_plot_pyqt_2.py)**: Networked plotting client designed to bind to a local UDP socket and visualize wireless telemetry streamed over Wi-Fi by the RP2040 (updated version).
*   🐍 **[`fluxgateDB.py`](file:///c:/Users/ASUS/Desktop/Fluxgate/Fluxgate-Project-Files/Software/Python_GUI_Demo/fluxgateDB.py)**: Data logging engine that logs the parsed magnetic readings into a database structure for long-term drift studies.
*   📊 **[`m1.m`, `m2.m`](file:///c:/Users/ASUS/Desktop/Fluxgate/Fluxgate-Project-Files/Software/Python_GUI_Demo)**: Matlab scripts for importing logged datasets, executing Fast Fourier Transforms (FFT), and computing the signal-to-noise ratio (SNR) and harmonic distortion of the sensor.

---

## 🖥️ Graphical Dashboards & APIs

The software stack implements three main processing and filtering pathways to display sensor data:

### 1. Real-Time Fast Fourier Transform (FFT) Analyzer (`pyqtgraph_float_v2x2_yesim.py`)
This script acts as a high-speed, GPU-accelerated **digital spectrum analyzer** used to run in **DSP Mode** (bypassing physical filters).
*   **Performance Engine:** Utilizes **PyQtGraph** for rendering, which delegates processing directly to the GPU via OpenGL/Qt graphics contexts. This allows the GUI to plot 16,500 samples/sec continuously at 60fps without lag or UI freezing.
*   **Spectral Demodulation & FFT Math:** 
    *   Reads and decodes the gapless 1.5MBaud UART stream.
    *   Saves raw AC data points into rolling buffer arrays of size $N = 8192$.
    *   Computes the Discrete Fourier Transform (DFT) via the Fast Fourier Transform (FFT) algorithm on the CPU:
        $$X[k] = \sum_{n=0}^{N-1} x[n] \cdot e^{-j \frac{2\pi}{N} k n}, \quad k = 0, 1, \dots, N-1$$
        where $x[n]$ is the discrete time-domain input sequence, $N = 8192$, and $k$ represents the frequency bin.
    *   The complex magnitude of each frequency bin is calculated as:
        $$|X[k]| = \sqrt{\text{Re}(X[k])^2 + \text{Im}(X[k])^2}$$
    *   Plots the rolling magnitude spectrum on a logarithmic scale (`p2.setLogMode(False, True)`) using the decibel (dB) scale representation:
        $$\text{Magnitude}_{\text{dB}}[k] = 20 \log_{10} \left( |X[k]| + \epsilon \right)$$
        where $\epsilon = 10^{-10}$ is a regularization constant added to prevent singular $\log(0)$ errors.
*   **Digital Lock-In Amplification:**
    *   **Excitation Frequency:** Given the RP2040 triple-phase excitation cycle ($0.8\text{ ms}$ positive + $0.4\text{ ms}$ neutral + $0.8\text{ ms}$ negative = $2.0\text{ ms}$ total period), the fundamental drive frequency is $f_0 = \frac{1}{2\text{ ms}} = 500\text{ Hz}$.
    *   **Demodulation Math:** The fluxgate effect concentrates the external magnetic field intensity in the second harmonic ($2f_0 = 1000\text{ Hz}$ or $1\text{ kHz}$). The digital lock-in process multiplies the digitized sensor output $x[n]$ with orthogonal sine and cosine reference signals at the second harmonic frequency:
        $$I[n] = x[n] \cdot \sin(4\pi f_0 n T_s)$$
        $$Q[n] = x[n] \cdot \cos(4\pi f_0 n T_s)$$
        where $T_s = \frac{1}{f_s}$ is the sampling period (for $f_s \approx 16.5\text{ kSps}$, $T_s \approx 60.6\text{ µs}$).
    *   **Filtering & Vector Extraction:** The in-phase $I[n]$ and quadrature $Q[n]$ products are passed through a digital low-pass filter (moving average or convolution) to extract the static DC terms $I_{\text{filt}}$ and $Q_{\text{filt}}$. The reconstructed magnetic field magnitude $R$ and phase angle $\theta$ are then derived as:
        $$R = \sqrt{I_{\text{filt}}^2 + Q_{\text{filt}}^2}$$
        $$\theta = \arctan2(Q_{\text{filt}}, I_{\text{filt}})$$
        This dual-phase lock-in structure allows the GUI to extract magnetic intensity with massive noise rejection, capturing both magnitude and phase orientation while bypassing all physical analog filter drift and component tolerances.

### 2. Time-Domain Moving Average Smoothing (`udp_plot_pyqt.py` and `udp_plot_pyqt_2.py`)
These scripts are designed for time-domain visualization of both raw and filtered signals.
*   **Signal Reconstruction:** Decodes UDP packets received over Wi-Fi, separating the primary channel and secondary OPAMP channels.
*   **Moving Average Filter Math:** Applies a digital **Moving Average Filter** in real-time using a NumPy convolution window of $M = 1000$ samples:
    $$y[n] = \frac{1}{M}\sum_{k=0}^{M-1} x[n-k]$$
*   **Discrete Time Convolution:** This summation is mathematically equivalent to the linear convolution of the input sequence $x[n]$ with a rectangular impulse response $h[n]$:
    $$y[n] = (x * h)[n] = \sum_{k=-\infty}^{\infty} x[k] \cdot h[n-k]$$
    where the filter's discrete impulse response $h[n]$ is defined by:
    $$h[n] = \begin{cases} \frac{1}{M}, & 0 \le n < M \\ 0, & \text{otherwise} \end{cases}$$
    ```python
    window_size = 1000
    weights = np.ones(window_size) / window_size
    filtered_data = np.convolve(data, weights, mode='valid')
    ```
*   **Frequency Response & Notch Characteristics:** The frequency-domain transfer function $H(e^{j\omega})$ of this moving average filter is a Dirichlet kernel (sinc-like function):
    $$H(e^{j\omega}) = \frac{1}{M} \sum_{n=0}^{M-1} e^{-j\omega n} = e^{-j\omega \frac{M-1}{2}} \cdot \frac{\sin\left(\frac{\omega M}{2}\right)}{M \sin\left(\frac{\omega}{2}\right)}$$
    This transfer function provides deep low-pass filtering and places periodic notches (infinite attenuation) at integer multiples of $f_{\text{notch}} = \frac{f_s}{M}$. For $f_s \approx 16.5\text{ kSps}$ and $M = 1000$, the first notch occurs at exactly $16.5\text{ Hz}$, which effectively filters out high-frequency switching ripples, transient spikes, and electromagnetic noise from the excitation H-bridge, providing a pristine, readable time-domain baseline.

*   **Wireless Ingestion (UDP Sockets):** To isolate the sensor from electromagnetic coupling (EMI) radiating from the host computer's USB ports, the sensor streams data over Wi-Fi. The scripts initialize a UDP receiver socket:
    ```python
    import socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(('0.0.0.0', 6001))
    ```
    It listens for incoming packets, extracts the 24-bit binary ADC payloads, and updates the PyQtGraph widget dynamically.

### 3. Graphical Compass UI
*   **Implementation:** `compass.py` integrates a lightweight PyQt interface that loads two image assets: `dial.png` (a 360-degree compass card) and `needle.png` (a red indicator needle).
*   **Function:** By converting the incoming digitized voltage stream into a magnetic field vector (scaled in microteslas, $µT$), the script computes the relative magnetic heading and applies a rotation matrix to `needle.png` in real-time, providing an intuitive, interactive display of the sensor's direction.

---

## 🚀 How to Run the GUIs

### Prerequisites
Install Python 3.8+ and the required packages:
```bash
pip install PyQt5 pyqtgraph numpy pyserial
```

### Running the USB Serial Plotter
1. Ensure the hardware is connected via USB and recognized by your system (check Device Manager on Windows for the active COM port, e.g., `COM3`).
2. Run the PyQtGraph script:
   ```bash
   python pyqtgraph_float_v2x2_yesim.py
   ```

### Running the Wireless UDP Plotter
1. Ensure the Pico W and your computer are connected to the same local network.
2. Update the `LOCAL_IP` in `udp_plot_pyqt_2.py` or `udp_plot_pyqt.py` to match your computer's IP address.
3. Start the UDP server script:
   ```bash
   python udp_plot_pyqt_2.py
   # or
   python udp_plot_pyqt.py
   ```
4. Power on the sensor. The data stream will automatically render on the screen.
