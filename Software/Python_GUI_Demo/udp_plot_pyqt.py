import pyqtgraph as pg
from pyqtgraph import QtWidgets, QtCore
import socket
import threading
import struct
import numpy as np
import time

# Set pyqtgraph options
pg.setConfigOption('background', 'black')  # Set background to white
pg.setConfigOption('foreground', 'y')  # Set grid lines color to black

# Create a plot and data
app = QtWidgets.QApplication([])
win = pg.GraphicsLayoutWidget(title="UDP data plot")
p = win.addPlot(title="Updating plot")
curve = p.plot(pen='r')  # Set curve color to blue
data = np.array([], dtype=np.uint16)

# Initialize counter and timestamp
num_samples = 0
start_time = time.time()

def update():
    global data, curve, num_samples, start_time
    curve.setData(data[-8192:])  # Plot only the most recent data points
    elapsed_time = time.time() - start_time
    bitrate = (num_samples*32) / elapsed_time  # Compute bitrate in bits per second
    p.setTitle(f'Samples received: {num_samples} samples in total, Bitrate: {bitrate:.2f} bits/s')

def receive_data():
    global data, num_samples
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(('0.0.0.0', 6001))

    while True:
        packet, addr = sock.recvfrom(1024)
        for i in range(0, len(packet), 4):
            const_value = struct.unpack('>H', packet[i:i+2])[0]
            if const_value == 0xA5A5:
                channel_data = struct.unpack('>H', packet[i+2:i+4])[0]
                data = np.append(data, channel_data)
                num_samples += 1

# Start a new thread for receiving data
threading.Thread(target=receive_data).start()

# Set a timer to update the plot
timer = QtCore.QTimer()
timer.timeout.connect(update)
timer.start(250)

# Start the QApplication
QtWidgets.QApplication.instance().exec_()
