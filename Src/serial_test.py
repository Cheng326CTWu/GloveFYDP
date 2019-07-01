import serial
import struct
import time

ser = serial.Serial('/dev/cu.usbmodem14103', 1152000, timeout=1)

ser.write("data")
data = []
start = time.time()
while (time.time() - start < 3):
    newData = ser.read(144)
    print(newData)

ser.write("stop")
print(ser.read(1000))
