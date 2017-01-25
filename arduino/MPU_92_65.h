#ifndef MPU_92_65_H
#define MPU_92_65_H

#define MPU9250_ADDRESS	0x68
#define   SMPLRT_DIV	0x19
#define   CONFIG	0x1A
#define   GYRO_CONFIG	0x1B
#define     GYRO_FULL_SCALE_MASK     0x18
#define     GYRO_FULL_SCALE_250_DPS  0x00
#define     GYRO_FULL_SCALE_500_DPS  0x08
#define     GYRO_FULL_SCALE_1000_DPS 0x10
#define     GYRO_FULL_SCALE_2000_DPS 0x18
#define   ACCEL_CONFIG	0x1C
#define     ACCEL_FULL_SCALE_MASK 0x18
#define     ACCEL_FULL_SCALE_2_G  0x00
#define     ACCEL_FULL_SCALE_4_G  0x08
#define     ACCEL_FULL_SCALE_8_G  0x10
#define     ACCEL_FULL_SCALE_16_G 0x18
#define   ACCEL_CONFIG2	0x1D
#define   INT_PIN_CFG	0x37
#define   INT_ENABLE	0x38
#define   ACCEL_XOUT_H	0x3B
#define   ACCEL_XOUT_L	0x3C
#define   ACCEL_YOUT_H	0x3D
#define   ACCEL_YOUT_L	0x3E
#define   ACCEL_ZOUT_H	0x3F
#define   ACCEL_ZOUT_L	0x40
#define   TEMP_OUT_H	0x41
#define   TEMP_OUT_L	0x42
#define   GYRO_XOUT_H	0x43
#define   GYRO_XOUT_L	0x44
#define   GYRO_YOUT_H	0x45
#define   GYRO_YOUT_L	0x46
#define   GYRO_ZOUT_H	0x47
#define   GYRO_ZOUT_L	0x48
#define   PWR_MGMT_1	0x6B  // Device defaults to the SLEEP mode
#define   PWR_MGMT_2	0x6C

#define AK8963_ADDRESS 0x0C
#define   AK8963_ST1	0x02  // data ready status bit 0
#define   AK8963_XOUT_L	0x03  // data
#define   AK8963_XOUT_H	0x04
#define   AK8963_YOUT_L	0x05
#define   AK8963_YOUT_H	0x06
#define   AK8963_ZOUT_L	0x07
#define   AK8963_ZOUT_H	0x08
#define   AK8963_ST2	0x09  // Data overflow bit 3 and data read error status bit 2
#define   AK8963_CNTL	0x0A
#define     AK8963_CNTL_MODE_OFF      0x00
#define     AK8963_CNTL_MODE_SINGLE   0x01
#define     AK8963_CNTL_MODE_8_HZ     0x02
#define     AK8963_CNTL_MODE_100_HZ   0x06
#define     AK8963_CNTL_MODE_SELFTEST 0x08
#define     AK8963_CNTL_MODE_FUSE_ROM 0x0f
#define     AK8963_CNTL_SCALE_16_BITS 0x10  // else 14 bits
#define   AK8963_ASTC	0x0C  // Self test control
#define   AK8963_ASAX	0x10  // Fuse ROM x-axis sensitivity adjustment value
#define   AK8963_ASAY	0x11  // Fuse ROM y-axis sensitivity adjustment value
#define   AK8963_ASAZ	0x12  // Fuse ROM z-axis sensitivity adjustment value

class MPU_92_65
{
public:
    void init();
    void read_accel(int16_t[3]);
    void read_gyro(int16_t[3]);
    uint8_t read_mag(int16_t[3]);

    // These take effect when calling init.
    int gyro_full_scale = GYRO_FULL_SCALE_250_DPS;
    int accel_full_scale = ACCEL_FULL_SCALE_16_G;
    int mag_cntl = AK8963_CNTL_SCALE_16_BITS | AK8963_CNTL_MODE_100_HZ;

    // This is updated by init().
    float mag_cal_scale[3];

private:
    void init_MPU9250();
    void init_AK8963();
};

#endif
