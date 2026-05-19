# 💻 Software, GUI & Analytics

This directory contains the Python-based real-time graphical user interfaces (GUIs), data plotting dashboards, and Matlab analytical scripts used to process, log, and visualize the digitized Fluxgate sensor telemetry.

---

## 📁 Key Software Files

*   🐍 **[`compass.py`](file:///c:/Users/ASUS/Desktop/Fluxgate/Fluxgate-Project-Files/Software/Python_GUI_Demo/compass.py)**: Python client that maps the parsed magnetic field intensity to a graphical rotating compass needle (`needle.png` and `dial.png`).
*   🐍 **[`pyqtgraph_float_v2x2_yesim.py`](file:///c:/Users/ASUS/Desktop/Fluxgate/Fluxgate-Project-Files/Software/Python_GUI_Demo/pyqtgraph_float_v2x2_yesim.py)**: Real-time, multi-channel oscilloscope client built using **PyQtGraph** for rendering the high-speed (16.5 kSps) 24-bit ADC data.
*   🐍 **[`udp_plot_pyqt_2.py`](file:///c:/Users/ASUS/Desktop/Fluxgate/Fluxgate-Project-Files/Software/Python_GUI_Demo/udp_plot_pyqt_2.py)**: Networked plotting client designed to bind to a local UDP socket and visualize wireless telemetry streamed over Wi-Fi by the RP2040.
*   🐍 **[`fluxgateDB.py`](file:///c:/Users/ASUS/Desktop/Fluxgate/Fluxgate-Project-Files/Software/Python_GUI_Demo/fluxgateDB.py)**: Data logging engine that logs the parsed magnetic readings into a database structure for long-term drift studies.
*   📊 **[`m1.m`, `m2.m`](file:///c:/Users/ASUS/Desktop/Fluxgate/Fluxgate-Project-Files/Software/Python_GUI_Demo)**: Matlab scripts for importing logged datasets, executing Fast Fourier Transforms (FFT), and computing the signal-to-noise ratio (SNR) and harmonic distortion of the sensor.

---

## 🖥️ Graphical Dashboards & APIs

The software stack implements three main processing pathways to display telemetry:

### 1. High-Performance Oscilloscope (PyQtGraph)
Microcontroller telemetry operating at **16.5 kSps** can easily freeze standard plotting libraries (like Matplotlib) due to rendering lag.
*   **Solution:** `pyqtgraph_float_v2x2_yesim.py` implements a **PyQtGraph-based GUI**. PyQtGraph delegates rendering directly to the system's GPU via OpenGL/Qt graphics contexts, enabling smooth 60fps rendering of thousands of data points.
*   **Parsing:** Unpacks the incoming high-speed binary/ASCII stream from the RP2040, reconstructs the 24-bit integer values, and plots them in a rolling time-domain window.

### 2. Networked Wi-Fi Client (UDP Sockets)
To isolate the sensor from electromagnetic coupling (EMI) radiating from the computer's USB port, the sensor can stream data over Wi-Fi.
*   **Implementation:** `udp_plot_pyqt_2.py` initializes a standard Python socket:
    ```python
    import socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((LOCAL_IP, PORT))
    ```
*   **Streaming:** Listens for incoming UDP datagrams sent by the RP2040, extracts the binary ADC payloads, and updates the PyQtGraph widget dynamically.

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
2. Update the `LOCAL_IP` in `udp_plot_pyqt_2.py` to match your computer's IP address.
3. Start the UDP server script:
   ```bash
   python udp_plot_pyqt_2.py
   ```
4. Power on the sensor. The data stream will automatically render on the screen.
