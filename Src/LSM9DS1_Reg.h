/*
 * LSM9DS1_Reg.h
 *
 *  Created on: May 11, 2019
 *      Author: tejasvi
 */

#ifndef LSM9DS1_REG_H_
#define LSM9DS1_REG_H_

// ----------------------- Common registers
#define CTRL_REG4 0x1E

#define CTRL_REG10 0x24     // self-test register
#define CTRL_REG10_ENABLE_SELF_TEST 0x05     // enable both acceleration and gyro self test
#define CTRL_REG10_DISABLE_SELF_TEST 0x00

// -------------------------- Accelerometer control registers
#define CTRL_REG5_XL 0x1F

#define CTRL_REG6_XL 0x20
#define CTRL_REG6_XL_ODR_MASK 0xE0
#define CTRL_REG6_XL_FS_MASK 0x18
#define CTRL_REG6_XL_AA_BW_MASK 0x07
#define CTRL_REG6_XL_ODR_POWER_DOWN 0x00

// Accelerometer ODRs
#define CTRL_REG6_XL_ODR_10 0x20
#define CTRL_REG6_XL_ODR_50 0x40
// ... and so on, add more from Table 68 in datasheet if desired

// Accelerometer full scale
#define CTRL_REG6_XL_FS_2G 0x00     // default
#define CTRL_REG6_XL_FS_4G 0x10
#define CTRL_REG6_XL_FS_8G 0x18
#define CTRL_REG6_XL_FS_16G 0x08

// Accelerometer anti-aliasing filter bandwidth
#define CTRL_REG6_XL_AA_BW_105  0x06        // 105 Hz anti-aliasing bandwidth - just above the Nyquist for 50 Hz

// Gyro control register
#define CTRL_REG1_G 0x10
#define CTRL_REG1_G_ODR_MASK 0xE0
#define CTRL_REG1_G_FS_MASK 0x18
#define CTRL_REG1_G_ODR_POWER_DOWN 0x00
#define CTRL_REG1_G_ODR_14p9 0x20
#define CTRL_REG1_G_ODR_59p5 0x40
// ... and so on for ODRs, add more from Table 46 in datasheet if desired
#define CTRL_REG1_G_FS_500_DPS 0x08

// Magnetometer control registers
#define CTRL_REG1_M 0x20
#define CTRL_REG1_M_ODR_MASK 0x1A
#define CTRL_REG1_M_ODR_p625 0x00
#define CTRL_REG1_M_ODR_40 0x60
#define CTRL_REG2_M 0x21
#define CTRL_REG3_M 0x22
#define CTRL_REG4_M 0x23
#define CTRL_REG5_M 0x24


// accelerometer and gyro software reset
#define CTRL_REG8 0x22
#define CTRL_REG8_RESET_VAL 0x05

// magnetometer software reset
#define CTRL_REG2_M_RESET_VAL 0x0C

// gyro data register
#define OUT_X_G 0x18

// magnetometer data register
#define OUT_X_L_M 0x28

// Sensitivities, depending on the full scale setting of each sensor, as defined in Table 3 of datasheet
// copied from https://github.com/sparkfun/SparkFun_LSM9DS1_Arduino_Library/blob/master/src/SparkFunLSM9DS1.cpp
#define SENS_ACCELEROMETER_2  0.000061
#define SENS_ACCELEROMETER_4  0.000122
#define SENS_ACCELEROMETER_8  0.000244
#define SENS_ACCELEROMETER_16 0.000732
#define SENS_GYROSCOPE_245    0.00875
#define SENS_GYROSCOPE_500    0.0175
#define SENS_GYROSCOPE_2000   0.07
#define SENS_MAGNETOMETER_4   0.00014
#define SENS_MAGNETOMETER_8   0.00029
#define SENS_MAGNETOMETER_12  0.00043
#define SENS_MAGNETOMETER_16  0.00058

#endif /* LSM9DS1_REG_H_ */
