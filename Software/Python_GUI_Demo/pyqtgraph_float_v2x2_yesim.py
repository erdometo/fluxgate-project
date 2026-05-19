import serial
import time
from pyqtgraph.Qt import QtGui, QtCore

import numpy as np
import pyqtgraph as pg
#from pyqtgraph.ptime import time
from scipy.fftpack import fft
import struct

###################################################################3
import serial.tools.list_ports as p
N=8192


Nf=2
T =0.002
ch0_on=True  #always on
ch1_on=True
ch2_on=False #True
port = "COM7"
baud = 115200

Xf = np.linspace(0.0,1.0/(2.0*T), N//Nf)
Xt = np.linspace(0.0, N*T, N)
Yt0 = np.linspace(0.0, N*T, N)
Yt1 = np.linspace(0.0, N*T, N)
Yt2 = np.linspace(0.0, N*T, N)
Yf0 = np.linspace(0.0, N*T, N)
Yf1 = np.linspace(0.0, N*T, N)
Yf2 = np.linspace(0.0, N*T, N)

app = QtGui.QApplication([])
p = pg.plot()
p.showGrid(5,5)
p.addLegend()
p.setWindowTitle('KTU dsplab UART Live Data')
p.setLabel("left","Amplitude", units="Volt")
p.setLabel("bottom","Time",units="s")
#p.setXRange(0,N*T)
#p.setYRange(-1.67,1.67)
curve0 = p.plot(pen="r",name="CH0")



p1 = pg.plot()
p1.showGrid(5,5)
p1.addLegend()
p1.setWindowTitle('KTU dsplab UART Live Data')
p1.setLabel("left","Amplitude", units="Volt")
p1.setLabel("bottom","Time",units="s")
curve1 = p1.plot(pen="g",name="CH1")


p2 = pg.plot()
p2.showGrid(10,10)
p2.addLegend()
p2.setWindowTitle('Frequency')
p2.setLabel("left","Amplitude", units="Volt")
p2.setLabel("bottom","Frequency",units="Hz")
#p2.setXRange(0,N*T)
#p2.setYRange(-1.67,1.67)
p2.setLogMode(False, True)
if ch0_on == True:
    curvef0 = p2.plot(pen="r",name="CH0_FFT")
if ch1_on == True:
    curvef1 = p2.plot(pen="g",name="CH1_FFT")
if ch2_on == True:
    curvef2 = p2.plot(pen="m",name="CH2_FFT")


global ii,d0,d1,d2,cum0dc,cum0ac,paz1dc,paz1ac,sal2dc,sal2ac,d0ac,d1ac,d2ac
d0=0.0
d1=0.0
d2=0.0
d0ac=0.0
d1ac=0.0
d1ac=0.0
cum0dc = []
cum0ac = []
paz1dc = []
paz1ac = []
sal2dc = []
sal2ac = []


av0=0.0
av1=0.0
av2=0.0
fdata = []
ii=0
k=0
j=0
Np1=N+1
Nm1=N-1
mode = 0
raw=serial.Serial(port,baud)
raw.reset_input_buffer()
print("in waiting:",raw.in_waiting)

def update():
    global j,d0,d1,d2,av0,av1,av2,cum0dc,cum0ac,paz1dc,paz1ac,sal2dc,sal2ac,d0ac,d1ac,d2ac,ch0ac,ch1ac,ch2ac
    index = 0
    data = []
    ch0 = []
    ch1 = []
    ch2 = []
    data_ch0=np.dtype(int)
    data_ch1=np.dtype(int)
    data_ch2=np.dtype(int)
    try:
        kk=0
        i = 0
        while True:

            inw = raw.in_waiting
            data.extend(raw.read(inw))
            index = index + 1
            if index == 5000:
                index = 0
                inw = raw.in_waiting
                print("data on que:", len(data),"j:",j)
                #print("kuyruk:",inw,"j:",j)

            if len(data) < 14:
                continue

            if data[0] != 0xA5 or data[1] != 0xA5:
                print("packet error, [0]:", data[0], "[13]:", data[1],"index=",index,"\n")
                data.pop(0)
                continue

            data.pop(0)
            data.pop(0)

            data_ch0= struct.unpack('<f',bytes(data[0:4]))
            data_ch1= struct.unpack('<f',bytes(data[4:8]))
            data_ch2= struct.unpack('<f',bytes(data[8:12]))

            data.pop(0)
            data.pop(0)
            data.pop(0)
            data.pop(0)
            data.pop(0)
            data.pop(0)
            data.pop(0)
            data.pop(0)
            data.pop(0)
            data.pop(0)
            data.pop(0)
            data.pop(0)
            d0=d0*0.75+data_ch0[0]*0.25
            d1=d1*0.75+data_ch1[0]*0.25
            d2=d2*0.75+data_ch2[0]*0.25

            ch0.append(d0)
            ch1.append(d1)
            ch2.append(d2)
            j=j+1
            if(j > N):
                mode = 1
                ch0.pop(0)
                ch1.pop(0)
                ch2.pop(0)
         
                j=Np1
                i=i+1
                if i == 20:
                                      
                    if ch0_on == True:
                        Yf0=fft(ch0)
                        c0_data = np.array(ch0, dtype='float')
                        curve0.setData(Xt,ch0)
                        c0_fdata = np.array(abs(Yf0[2:N//Nf]), dtype='float')
                        curvef0.setData(Xf[2:N//Nf],c0_fdata)
                    if ch1_on == True:
                        Yf1=fft(ch1)
                        c1_data = np.array(ch1, dtype='float')
                        curve1.setData(Xt,ch1)
                        c1_fdata = np.array(abs(Yf1[2:N//Nf]), dtype='float')
                        curvef1.setData(Xf[2:N//Nf],c1_fdata)
                    if ch2_on == True:
                        Yf2=fft(ch2)
                        c2_data = np.array(ch2, dtype='float')
                        curve2.setData(Xt,ch2)
                        c2_fdata = np.array(abs(Yf2[2:N//Nf]), dtype='float')
                        curvef2.setData(Xf[2:N//Nf],c2_fdata) 
                    app.processEvents()
                    i=0

                
    except KeyboardInterrupt:
##        exporter = pg.exporters.ImageExporter(view.scene())
##
##        # save to file
##        exporter.export('e54xx.png')
        print("stopping...")
        raw.close()

timer = QtCore.QTimer()
timer.timeout.connect(update)
timer.start()

if __name__ == '__main__':
    import sys
    if (sys.flags.interactive != 1) or not hasattr(QtCore, 'PYQT_VERSION'):
        QtGui.QApplication.instance().exec_()


