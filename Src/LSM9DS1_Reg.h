/*
 * LSM9DS1_Reg.h
 *
 *  Created on: May 11, 2019
 *      Author: tejasvi
 */

#ifndef LSM9DS1_REG_H_
#define LSM9DS1_REG_H_

// Accelerometer control register
#define CTRL_REG6_XL 0x20
#define CTRL_REG6_XL_ODR_MASK 0xE0
#define CTRL_REG5_XL_ODR_POWER_DOWN 0x00
#define CTRL_REG5_XL_ODR_10 0x20
#define CTRL_REG5_XL_ODR_50 0x40
// ... and so on, add more from Table (xx) in datasheet if desired

// Gyro control register
#define CTRL_REG1_G 0x10
#define CTRL_REG1_G_ODR_MASK 0xE0
#define CTRL_REG1_G_ODR_POWER_DOWN 0x00
#define CTRL_REG1_G_ODR_14p9 0x20
#define CTRL_REG1_G_ODR_59p5 0x40
// ... and so on, add more from Table 46 in datasheet if desired

// Magnetometer control register
#define CTRL_REG1_M 0x20
#define CTRL_REG1_M_ODR_MASK 0x18
#define CTRL_REG1_M_ODR_p625 0x00
#define CTRL_REG1_M_ODR_40 0x60


// accelerometer and gyro software reset
#define CTRL_REG8 0x22
#define CTRL_REG8_RESET_VAL 0x05

// magnetometer software reset
#define CTRL_REG2_M 0x21
#define CTRL_REG2_M_RESET_VAL 0x0C

// gyro data register
#define OUT_X_G 0x18

// magnetometer data register
#define OUT_X_L_M 0x28

// Sensitivities, assuming that the FS bits in the control registers are set to lowest value (00)
#define ACCEL_SENS 0.061f        // = 4g/2^16 * 1000 [mg/LSB]
#define GYRO_SENS 0.0075F        // = 245*2 / 2^16 [dps/LSB]
#define MAG_SENSE 0.122          // [mGauss/LSB]

#endif /* LSM9DS1_REG_H_ */
