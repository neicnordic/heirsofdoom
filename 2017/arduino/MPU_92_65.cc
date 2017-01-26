// References:
//   https://cdn.sparkfun.com/assets/learn_tutorials/5/5/0/MPU9250REV1.0.pdf
//   https://cdn.sparkfun.com/assets/learn_tutorials/5/5/0/MPU-9250-Register-Map.pdf
//   http://www.luisllamas.es/2016/09/usar-arduino-con-los-imu-de-9dof-mpu-9150-y-mpu-9250/

#include <Arduino.h>
#include <Wire.h>
#include "MPU_92_65.h"

void i2c_request(uint8_t addr, uint8_t reg, int len)
{
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom(addr, (uint8_t)len);
}

uint8_t i2c_get_uint8(uint8_t addr, uint8_t reg)
{
    i2c_request(addr, reg, 1);
    return Wire.read();
}

uint16_t i2c_read_uint16()
{
    uint16_t xH = Wire.read();
    uint16_t xL = Wire.read();
    return (xH << 8) | xL;
}

void i2c_read_int16_triple(int16_t *buf)
{
    for (int i = 0; i < 3; ++i) buf[i] = (int16_t)i2c_read_uint16();
}

void i2c_write_uint8(uint8_t addr, uint8_t reg, uint8_t x)
{
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.write(x);
    Wire.endTransmission();
}

void MPU_92_65::init()
{
    Wire.begin();
    //Wire.setClock(1000000);
    init_MPU9250();
    init_AK8963();
}

// Taken from initMPU9250 from MPU-9250_Breakout.
void MPU_92_65::init_MPU9250()
{
    // wake up device
    i2c_write_uint8(MPU9250_ADDRESS, PWR_MGMT_1, 0x00); // Clear sleep mode bit (6), enable all sensors 
    delay(100); // Wait for all registers to reset

    // get stable time source
    i2c_write_uint8(MPU9250_ADDRESS, PWR_MGMT_1, 0x01);  // Auto select clock source to be PLL gyroscope reference if ready else
    delay(200);

    // Configure Gyro and Thermometer
    // Disable FSYNC and set thermometer and gyro bandwidth to 41 and 42 Hz, respectively; 
    // minimum delay time for this setting is 5.9 ms, which means sensor fusion update rates cannot
    // be higher than 1 / 0.0059 = 170 Hz
    // DLPF_CFG = bits 2:0 = 011; this limits the sample rate to 1000 Hz for both
    // With the MPU9250, it is possible to get gyro sample rates of 32 kHz (!), 8 kHz, or 1 kHz
    i2c_write_uint8(MPU9250_ADDRESS, CONFIG, 0x03);

    // Set sample rate = gyroscope output rate/(1 + SMPLRT_DIV)
    i2c_write_uint8(MPU9250_ADDRESS, SMPLRT_DIV, 0x04);  // Use a 200 Hz rate; a rate consistent with the filter update rate 
    // determined inset in CONFIG above

    uint8_t c = i2c_get_uint8(MPU9250_ADDRESS, GYRO_CONFIG); // get current GYRO_CONFIG register value
    // c = c & ~0xE0; // Clear self-test bits [7:5] 
    c &= ~0x02; // Clear Fchoice bits [1:0] 
    c &= ~GYRO_FULL_SCALE_MASK;
    c |= gyro_full_scale;
    // c =| 0x00; // Set Fchoice for the gyro to 11 by writing its inverse to bits 1:0 of GYRO_CONFIG
    i2c_write_uint8(MPU9250_ADDRESS, GYRO_CONFIG, c ); // Write new GYRO_CONFIG value to register

    // Set accelerometer full-scale range configuration
    c = i2c_get_uint8(MPU9250_ADDRESS, ACCEL_CONFIG); // get current ACCEL_CONFIG register value
    // c = c & ~0xE0; // Clear self-test bits [7:5] 
    c &= ~ACCEL_FULL_SCALE_MASK;
    c |= accel_full_scale;
    i2c_write_uint8(MPU9250_ADDRESS, ACCEL_CONFIG, c); // Write new ACCEL_CONFIG register value

    // Set accelerometer sample rate configuration
    // It is possible to get a 4 kHz sample rate from the accelerometer by choosing 1 for
    // accel_fchoice_b bit [3]; in this case the bandwidth is 1.13 kHz
    c = i2c_get_uint8(MPU9250_ADDRESS, ACCEL_CONFIG2); // get current ACCEL_CONFIG2 register value
    c &= ~0x0F; // Clear accel_fchoice_b (bit 3) and A_DLPFG (bits [2:0])  
    c |= 0x03;  // Set accelerometer rate to 1 kHz and bandwidth to 41 Hz
    i2c_write_uint8(MPU9250_ADDRESS, ACCEL_CONFIG2, c); // Write new ACCEL_CONFIG2 register value
    // The accelerometer, gyro, and thermometer are set to 1 kHz sample rates, 
    // but all these rates are further reduced by a factor of 5 to 200 Hz because of the SMPLRT_DIV setting

    // Configure Interrupts and Bypass Enable
    // Set interrupt pin active high, push-pull, hold interrupt pin level HIGH until interrupt cleared,
    // clear on read of INT_STATUS, and enable I2C_BYPASS_EN so additional chips 
    // can join the I2C bus and all can be controlled by the Arduino as master
    i2c_write_uint8(MPU9250_ADDRESS, INT_PIN_CFG, 0x22);
    i2c_write_uint8(MPU9250_ADDRESS, INT_ENABLE, 0x01);  // Enable data ready (bit 0) interrupt
    delay(100);

    // Old code. Stopped working.
    //   i2c_write_uint8(MPU9250_ADDRESS, ACCEL_CONFIG, ACCEL_FULL_SCALE_16_G);
    //   i2c_write_uint8(MPU9250_ADDRESS, GYRO_CONFIG, GYRO_FULL_SCALE_2000_DPS);
    //   i2c_write_uint8(MPU9250_ADDRESS, INT_PIN_CFG, 0x02);
}

void MPU_92_65::init_AK8963()
{
    // Extract the factory calibration for magnetometer.
    i2c_write_uint8(AK8963_ADDRESS, AK8963_CNTL, 0x00); // Power down magnetometer  
    delay(10);
    i2c_write_uint8(AK8963_ADDRESS, AK8963_CNTL, 0x0F); // Enter Fuse ROM access mode
    delay(10);
    i2c_request(AK8963_ADDRESS, AK8963_ASAX, 3);  // Read the x-, y-, and z-axis calibration values
    for (int i = 0; i < 3; ++i) {
	uint8_t raw = Wire.read();
	mag_cal_scale[i] = 1.0 + (float)(raw - 128)/256.0;
    }
    i2c_write_uint8(AK8963_ADDRESS, AK8963_CNTL, 0x00); // Power down magnetometer  
    delay(10);

    // Configure the magnetometer for continuous read and highest resolution
    // set Mscale bit 4 to 1 (0) to enable 16 (14) bit resolution in CNTL register,
    // and enable continuous mode data acquisition Mmode (bits [3:0]), 0010 for 8 Hz and 0110 for 100 Hz sample rates
    i2c_write_uint8(AK8963_ADDRESS, AK8963_CNTL, mag_cntl); // Set magnetometer data resolution and sample ODR
    delay(10);

    // Old code. Stopped working.
    //   i2c_write_uint8(AK8963_ADDRESS, AK8963_ASTC, 0x00);
    //   i2c_write_uint8(AK8963_ADDRESS, AK8963_CNTL, 0x16);
}

void MPU_92_65::read_accel(int16_t a[3])
{
    i2c_request(MPU9250_ADDRESS, ACCEL_XOUT_H, 6);
    i2c_read_int16_triple(a);
}

void MPU_92_65::read_gyro(int16_t omega[3])
{
    i2c_request(MPU9250_ADDRESS, 0x43, 6);
    i2c_read_int16_triple(omega);
}

uint8_t MPU_92_65::read_mag(int16_t h[3])
{
    while (!(i2c_get_uint8(AK8963_ADDRESS, AK8963_ST1) & 1));
    i2c_request(AK8963_ADDRESS, AK8963_XOUT_L, 7);
    for (int i = 0; i < 3; ++i) {
	uint16_t hL = Wire.read();
	uint16_t hH = Wire.read();
	h[i] = (hH << 8) | hL;
    }
    return Wire.read(); // Status, indicates overflow.
}
