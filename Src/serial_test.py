import math
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib import style
import serial
import struct
import time
import madgwick_py.madgwickahrs as Madgwick
# import madgwick
import madgwick_py.quaternion as Q
import numpy as np
from scipy.signal import butter, lfilter, freqz

# matplotlib style
style.use('fivethirtyeight')

# sensitivities
SENSITIVITY_ACCELEROMETER_2  = 0.000061
SENSITIVITY_ACCELEROMETER_4  = 0.000122
SENSITIVITY_ACCELEROMETER_8  = 0.000244
SENSITIVITY_ACCELEROMETER_16 = 0.000732
SENSITIVITY_GYROSCOPE_245    = 0.00875
SENSITIVITY_GYROSCOPE_500    = 0.0175
SENSITIVITY_GYROSCOPE_2000   = 0.07
SENSITIVITY_MAGNETOMETER_4   = 0.00014
SENSITIVITY_MAGNETOMETER_8   = 0.00029
SENSITIVITY_MAGNETOMETER_12  = 0.00043
SENSITIVITY_MAGNETOMETER_16  = 0.00058
sens = (SENSITIVITY_ACCELEROMETER_2, SENSITIVITY_ACCELEROMETER_2, SENSITIVITY_ACCELEROMETER_2,
        SENSITIVITY_GYROSCOPE_245, SENSITIVITY_GYROSCOPE_245, SENSITIVITY_GYROSCOPE_245,
        SENSITIVITY_MAGNETOMETER_4, SENSITIVITY_MAGNETOMETER_4, SENSITIVITY_MAGNETOMETER_4)


def to_rad(x):
    return (3.14159265 / 180) * x

def to_deg(x):
    # if x < 0:
    #     return 180/math.pi * x + 360
    return 180/math.pi * x


class Driver():

    def __init__(self):
        self.ser = serial.Serial('/dev/cu.usbmodem14103', 1152000, timeout=1)
        self.xAccOffset = 0
        self.yAccOffset = 0
        self.zAccOffset = 0
        self.xGyroOffset = 0
        self.yGyroOffset = 0
        self.zGyroOffset = 0
        self.xMagOffset = 0
        self.yMagOffset = 0
        self.zMagOffset = 0

    def continuousRead(self, duration):

        self.ser.write("data")
        data = []
        start = time.time()
        offsets = (self.xAccOffset, self.yAccOffset, 0,
                   self.xGyroOffset, self.yGyroOffset, self.zGyroOffset,
                   self.xMagOffset, self.yMagOffset, self.zMagOffset)

        while (time.time() - start < duration):
            newData = self.ser.read(288)        # 9 * 2 * 16 = 288 bytes
            (xAcc, yAcc, zAcc, xGyro, yGyro, zGyro, xMag, yMag, zMag) = struct.unpack('<hhhhhhhhh', newData[0:18])

            # multiply each value that was read by its scaling factor
            raw = (tuple(a * b for a,b in zip((xAcc, yAcc, zAcc, xGyro, yGyro, zGyro, xMag, yMag, zMag), sens)))

            # subtract each value by its offset
            data.append(tuple(a - b for a,b in zip(raw, offsets)))

        self.ser.write("stop")
        print(self.ser.read(1000))
        return data

    def calibrate(self):
        print("Starting calibration")

        print("Performing 5-second read for gyro and accel calibration.")
        data = self.continuousRead(5)

        allxAcc = [d[0] for d in data]
        allYAcc = [d[1] for d in data]
        allZacc = [d[2] for d in data]

        allxGyro = [d[3] for d in data]
        allyGyro = [d[4] for d in data]
        allzGyro = [d[5] for d in data]

        self.xAccOffset = sum(allxAcc)/len(allxAcc)
        self.yAccOffset = sum(allYAcc)/len(allYAcc)
        self.zAccOffset = sum(allZacc)/len(allZacc)
        self.xGyroOffset = sum(allxGyro)/len(allxGyro)
        self.yGyroOffset = sum(allyGyro)/len(allyGyro)
        self.zGyroOffset = sum(allzGyro)/len(allzGyro)

        # print("Preparing for magnetometer calibration. Start moving IMU slowly in figure 8's to cover a sphere.")
        # time.sleep(2)
        # print("Performing magnetometer calibration")
        # data = self.continuousRead(15)
        
        # allXMag = [d[6] for d in data]
        # allYMag = [d[7] for d in data]
        # allZMag = [d[8] for d in data]

        # self.xMagOffset = (max(allXMag) + min(allXMag))/2
        # self.yMagOffset = (max(allYMag) + min(allYMag))/2
        # self.zMagOffset = (max(allZMag) + min(allZMag))/2

        self.xMagOffset = -0.09093
        self.yMagOffset = 0.21447999999999998
        self.zMagOffset = -0.52108

        offsets = (self.xAccOffset, self.yAccOffset, self.zAccOffset,
            self.xGyroOffset, self.yGyroOffset, self.zGyroOffset,
            self.xMagOffset, self.yMagOffset, self.zMagOffset)
        print("offsets:")
        print(offsets)
        
        print("Done calibrating. Put the device back down.")
        time.sleep(2)


if __name__ == "__main__":

    # init driver, calibrate, and read for 5 seconds
    driver = Driver()
    driver.calibrate()
    data = driver.continuousRead(30)

    allxAcc = [d[0] for d in data]
    allYAcc = [d[1] for d in data]
    allZacc = [d[2] for d in data]

    allxGyro = [d[3] for d in data]
    allyGyro = [d[4] for d in data]
    allzGyro = [d[5] for d in data]

    allXMag = [d[6] for d in data]
    allYMag = [d[7] for d in data]
    allZMag = [d[8] for d in data]

    plt.figure(1)
    plt.title("Accelerations (g)")
    plt.plot([i for i in range(len(allxAcc))], allxAcc, '-o')
    plt.plot([i for i in range(len(allYAcc))], allYAcc, '-x')
    plt.plot([i for i in range(len(allZacc))], allZacc, '-')

    plt.figure(2)
    plt.title("Gyro (dps)")
    plt.plot([i for i in range(len(allxGyro))], allxGyro, '-o')
    plt.plot([i for i in range(len(allyGyro))], allyGyro, '-x')
    plt.plot([i for i in range(len(allzGyro))], allzGyro, '-')

    plt.figure(3)
    plt.title("Mag")
    plt.plot([i for i in range(len(allXMag))], allXMag, '-o')
    plt.plot([i for i in range(len(allYMag))], allYMag, '-x')
    plt.plot([i for i in range(len(allZMag))], allZMag, '-')
    
    # get pitch roll and yaw of filtered data with downloaded madgwick filter
    madgwick = Madgwick.MadgwickAHRS(sampleperiod=0.004, quaternion=Q.Quaternion(1, 0, 0, 0), beta=0.06)
    pitch = []
    roll = []
    yaw = []
    for i in range(len(data)):
        for j in range(10):
            madgwick.update(
                [to_rad(allxGyro[i]), to_rad(allyGyro[i]), to_rad(allzGyro[i])],
                [allxAcc[i], allYAcc[i], allZacc[i]],
                data[6:9])
        (r, p, y) = madgwick.quaternion.to_euler123()
        pitch.append(to_deg(p))
        roll.append(to_deg(r))
        yaw.append(to_deg(y))

    # # get pitch roll and yaw of filtered data with madgwick filter converted from C
    # pitch = []
    # roll = []
    # yaw = []
    # for i in range(len(data)):
    #     for j in range(10):
    #         madgwick.MadgwickQuaternionUpdate(
    #             filteredXAcc[i], filteredYAcc[i], filteredZAcc[i],
    #             to_rad(filteredXGyro[i]), to_rad(filteredYGyro[i]), to_rad(filteredZGyro[i]),
    #             data[i][6], data[i][7], data[i][8],
    #             0.004
    #         )
    #     quat = Q.Quaternion(*(madgwick.q))
    #     (r, p, y) = quat.to_euler_angles()
    #     pitch.append(180/math.pi * p)
    #     roll.append(180/math.pi * r)
    #     yaw.append(180/math.pi * y)

    # # get pitch roll and yaw from acceleration and magnetic field
    # # taken from https://engineering.stackexchange.com/questions/3348/calculating-pitch-yaw-and-roll-from-mag-acc-and-gyro-data
    # pitch = []
    # roll = []
    # yaw = []

    # for i in range(len(data)):
    #     p = 180 * math.atan2(filteredXAcc[i], math.sqrt(filteredYAcc[i]*filteredYAcc[i] + filteredZAcc[i]*filteredZAcc[i]))
    #     r = 180 * math.atan2(filteredYAcc[i], math.sqrt(filteredXAcc[i]*filteredXAcc[i] + filteredZAcc[i]*filteredZAcc[i]))
    #     y = 180 * math.atan2(filteredZAcc[i], math.sqrt(filteredXAcc[i]*filteredXAcc[i] + filteredZAcc[i]*filteredZAcc[i]))

    #     pitch.append(p)
    #     roll.append(r)
    #     yaw.append(y)

    # # low pass filter the pitch roll and yaw
    # order = 6
    # fs = 25       # sample rate, Hz
    # cutoff = 1    # desired cutoff frequency of the filter, Hz

    # # Get the filter coefficients so we can check its frequency response.
    # b, a = butter_lowpass(cutoff, fs, order)

    # pitch = butter_lowpass_filter(pitch, cutoff, fs, order)
    # roll = butter_lowpass_filter(roll, cutoff, fs, order)
    # yaw = butter_lowpass_filter(yaw, cutoff, fs, order)

    # plot pitch roll and yaw
    plt.figure(4)
    plt.title("Pitch (degrees)")
    plt.plot([i for i in range(len(pitch))], pitch, '-o')

    plt.figure(5)
    plt.title("Roll (degrees)")
    plt.plot([i for i in range(len(roll))], roll, '-x')

    plt.figure(6)
    plt.title("Yaw (degrees)")
    plt.plot([i for i in range(len(yaw))], yaw, '-')

    plt.show()
