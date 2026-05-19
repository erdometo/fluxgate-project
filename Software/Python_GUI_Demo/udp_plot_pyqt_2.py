import pyqtgraph as pg
from pyqtgraph.Qt import QtWidgets, QtCore
import socket
import threading
import struct
import numpy as np
import time
import tkinter as tk
from PIL import ImageTk, Image
import math

# Set pyqtgraph options
pg.setConfigOption('background', 'black')  # Set background to white
pg.setConfigOption('foreground', 'y')  # Set grid lines color to black

# Create a plot and data
app = QtWidgets.QApplication([])
win = pg.GraphicsLayoutWidget(show=True, title="1:RAW,  2:DSP,  3:OPAMP") # Set the title here

p = win.addPlot(title="FluxgateEffect")
curve = p.plot(pen='r')  # Set curve color to red
data = np.array([], dtype=np.uint32)

win.nextRow()
p2 = win.addPlot(title="MovingAverage")
curve2 = p2.plot(pen='b')  # Set curve color to blue

win.nextRow()
p3 = win.addPlot(title="AnalogAverage(OPAMP)")
curve3 = p3.plot(pen='g')  # Set curve color to blue
data2 = np.array([], dtype=np.uint32)

# Initialize counter and timestamp
num_samples = 0
start_time = time.time()

# Moving average filter parameters
window_size = 1000
weights = np.ones(window_size) / window_size

# Create a new Tkinter window
window = tk.Tk()
window.title("Compass Dial")

# Load the compass image
compass_image = Image.open("dial.png")
compass_image = compass_image.resize((900, 900), Image.ANTIALIAS)
compass_photo = ImageTk.PhotoImage(compass_image)

# Create a canvas to display the compass image
canvas = tk.Canvas(window, width=900, height=900)
canvas.create_image(450, 450, image=compass_photo)
canvas.pack()
global deg
deg=45

# Function to update the compass needle position
def update_compass(heading):
    # Convert heading to radians
    angle = math.radians(heading)
    
    # Calculate the coordinates of the needle
    x = 450 + 800 * math.sin(angle)
    y = 450 - 800 * math.cos(angle)
    
    # Draw a line representing the needle
    canvas.delete("needle")
    canvas.create_line(450, 450, x, y, width=2, fill="red", tags="needle")
    
def update():
    global data, curve,curve2,curve3, num_samples, start_time, deg        
    elapsed_time = time.time() - start_time
    bitrate = (num_samples*72) / (elapsed_time+np.finfo(float).eps)  # Compute bitrate in bits per second

    # Apply moving average filter to data if it has enough elements
    filtered_data = np.array([10000])  # Initialize filtered_data as an empty array
    if len(data) >= window_size:
        filtered_data = np.convolve(data, weights, mode='valid')

    win.setWindowTitle(f'Samples received: {num_samples} samples in total, Bitrate: {bitrate:.2f} bits/s')
    #RawData
    curve.setData(data)
    #moving average
    curve2.setData(filtered_data)  
    #OPAMP çıkışı
    curve3.setData(data2) 
    
    # Calculate the average of the previous values
    avg_previous = sum(filtered_data[-2000:]) / len(filtered_data[-2000:])

    # Determine the north pole direction based on the current value compared to the average of previous values
    if filtered_data[-1] > avg_previous+8000:
        deg +=0.3
        update_compass(deg)
    if filtered_data[-1] < avg_previous-8000:
        deg -=0.3
        update_compass(deg)

    # Reset num_samples and start_time every 10 seconds
    if elapsed_time > 10:
        num_samples = 0
        start_time = time.time()

    
def receive_data():
    global data, num_samples, data2
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(('0.0.0.0', 6001))
    buffer = []
    max_value_diff = 10000000000000000  # max allowed difference between consecutive data values
    last_value = 0  # Keep track of last value
    last_value2 = 0 

    while True:
        packet, addr = sock.recvfrom(1024)
        buffer += packet
        while len(buffer) >=12:
            if len(buffer) < 12:   ##bir bloktan küçükse yeni paket bekle
                continue
            if (buffer[0] != 0xA5 and buffer[1] != 0xA5  and buffer[10] !=0xA5 and buffer[11] != 0xA5 ) : ## senkronizasyon, birer birer kayarak
                buffer = buffer[1:]
                continue  
            channel_data = buffer[2] + ( 2**8*buffer[3]) + (2**16 + buffer[4]) + (2**24*buffer[5])
            channel_data -= 2**23  #  bias
            channel2_data = buffer[6] + (2**8*buffer[7]) + (2**16*buffer[8]) +( 2**24*buffer[9] )
            channel2_data -= 2**31 #  bias
            
            # Ignore the value if difference from last one is too big
            if abs(channel_data - last_value & channel2_data - last_value2 ) <= max_value_diff :
                last_value = channel_data
                last_value2 = channel2_data
                data = np.append(data, channel_data)
                data2 = np.append(data2, channel2_data)
                num_samples += 1
            
            if len(data) > 20000:
                data = data[-20000:]  # Keep the last 20000 element
            if len(data2) > 20000:
                data2 = data2[-20000:]  # Keep the last 20000 element
            buffer = buffer[10:]  # Moving by 10 bytes as sync words are repeated after every 10 bytes


# Start a new thread for receiving data
threading.Thread(target=receive_data).start()

# Set a timer to update the plot
timer = QtCore.QTimer()
timer.timeout.connect(update)
timer.start(5)

# Start the QApplication
QtWidgets.QApplication.instance().exec_()
# Run the Tkinter event loop
window.mainloop()