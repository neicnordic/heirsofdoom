#include <MeOrion.h>
#include "MPU_92_65.h"

// Instructions
//   OP_DELAY (time : uint16) -> void
//   OP_DRIVE|0b00[revL][revR] (speedL : uint8) (speedR : uint8) -> void
//   OP_SENSE_RAW|0b[en_mag][en_gyro][en_accel][en_sonar] =>
//     time : uint32
//     sonar : uint16		if requested
//     accel : int16 triple	if requested
//     gyro : int16 triple	if requested
//     mag : int16 triple	if requested
//   OP_SENSE_CAL =>
//     time : uint32
//     sonar : uint16
//     accel_x : int16
//     gyro_z : int16
//     yaw : int16		in radians scaled by 2^13
//   OP_CALIBRATE (bias : int16^2) (scale : int16^2)

#define OP_SYNC 0x10
#define OP_DELAY 0x20
#define OP_DRIVE 0x30
#define   OP_DRIVE_REV_L 0x1
#define   OP_DRIVE_REV_R 0x2
#define OP_SENSE_RAW 0x40
#define   OP_SENSE_RAW_SONAR 0x1
#define   OP_SENSE_RAW_ACCEL 0x2
#define   OP_SENSE_RAW_GYRO  0x4
#define   OP_SENSE_RAW_MAG   0x8
#define OP_CALIBRATE 0x50
#define OP_SENSE_CAL 0x60
#define OP_TURN 0x70
#define   OP_TURN_L   0x1
#define   OP_TURN_R   0x2
#define   OP_TURN_ABS 0x4

const float INT16_OF_ANGLE = 32768. / PI;
const float ANGLE_OF_INT16 = PI / 32768.;

const float TURN_SPEED = 160;
const float TURN_SLOWDOWN_ANGLE = PI / 3.;
const float TURN_PRECISION = 0.005 * PI;
const float TURN_MIN_SPEED = 40;

// Hardware
//
MeDCMotor motorL(M1);
MeDCMotor motorR(M2);
MeUltrasonicSensor sonar(PORT_3);
MPU_92_65 mpu;
Me7SegmentDisplay disp(PORT_6);

int boot_time_ms;
int sync_time_ms;

// Magnetometer Calibration
//
float mag_scale[2];
int16_t mag_bias[2];

// Helpers
//
float sub_angle(float phi1, float phi2)
{
    float dphi = phi1 - phi2;
    if (dphi < -PI) return dphi + TWO_PI;
    if (dphi >  PI) return dphi - TWO_PI;
    return dphi;
}

uint16_t recv_uint16()
{
    while (Serial.available() < 2);
    uint8_t xH = Serial.read();
    uint8_t xL = Serial.read();
    return (xH << 8) | xL;
}

void send_uint16(uint16_t x)
{
    Serial.write((x >> 8) & 0xff);
    Serial.write(x & 0xff);
}

void send_uint32(uint32_t x)
{
    Serial.write((x >> 24) & 0xff);
    Serial.write((x >> 16) & 0xff);
    Serial.write((x >> 8) & 0xff);
    Serial.write(x & 0xff);
}

inline int16_t recv_int16() { return recv_uint16(); }
inline void send_int16(int16_t x) { send_uint16(x); }

inline float recv_angle() { return recv_int16() * ANGLE_OF_INT16; }
inline void send_angle(float x) { send_int16(x * INT16_OF_ANGLE); }

void send_int16_triple(int16_t v[3])
{
    send_uint16(v[0]);
    send_uint16(v[1]);
    send_uint16(v[2]);
}

float current_direction()
{
    int16_t vec3[3];
    mpu.read_mag(vec3);
    double mag[2];
    for (int i = 0; i < 2; ++i)
	mag[i] = (vec3[i] - mag_bias[i]) * mag_scale[i];
    return atan2(mag[1], mag[0]);
}

void turn(float phi_stop, int dir)
{
    float last_dphi = 0.0;
    if (dir == 0) {
	float phi = current_direction();
	float dphi = sub_angle(phi_stop, phi);
	dir = dphi < 0? -1 : 1;
    }
    float dr = dir * TURN_SPEED;
    for (;;) {
	float phi = current_direction();
	float dphi = sub_angle(phi_stop, phi);
	disp.display((int)(RAD_TO_DEG * dphi));
	if (abs(dphi) < TURN_SLOWDOWN_ANGLE) {
	    dr = TURN_SPEED * dphi / TURN_SLOWDOWN_ANGLE;
	    if (fabs(dphi) < TURN_PRECISION || last_dphi * dphi < 0)
		break;
	    if (fabs(dr) < TURN_MIN_SPEED)
		dr = dr < 0? -TURN_MIN_SPEED : TURN_MIN_SPEED;
	}
	motorL.run(-dr);
	motorR.run(+dr);
	delay(50);
	last_dphi = dphi;
    }
    motorL.run(0);
    motorR.run(0);
    delay(200);
}

// Main Program
//
void setup()
{
    Serial.begin(115200);
    disp.init();
    disp.set(BRIGHTNESS_2);

    for (int i = 0; i < 8; ++i) {
	disp.display((uint8_t)((i - 1) % 4), 0x20);
	disp.display((uint8_t)(i % 4), 0x21);
	delay(50);
    }
#ifdef DISPLAY_REVISION
    disp.display((uint16_t)DISPLAY_REVISION);
#endif

    mpu.init();
    sync_time_ms = boot_time_ms = millis();
}

void loop()
{
    if (Serial.available() > 0) {
	uint8_t insn = Serial.read();
	disp.display((uint8_t)0, insn >> 4);
	disp.display((uint8_t)1, (insn & 0xf) | 0x10);
	disp.display((uint8_t)2, 0x20);
	disp.display((uint8_t)3, 0x20);
	switch (insn & 0xf0) {
	    case OP_SYNC:
		sync_time_ms = millis();
		send_uint32(sync_time_ms - boot_time_ms);
		break;
	    case OP_DELAY: {
		sync_time_ms += recv_uint16();
		int t = millis();
		if (sync_time_ms > t) {
		    while (sync_time_ms > t + 300) {
			disp.display(1e-3 * (float)(sync_time_ms - t), 1);
			delay(200);
			t = millis();
		    }
		    delay(max(0, sync_time_ms - t));
		}
		break;
	    }
	    case OP_DRIVE: {
		int speedL = (uint8_t)Serial.read();
		int speedR = (uint8_t)Serial.read();
		motorL.run(insn & OP_DRIVE_REV_L? -speedL : speedL);
		motorR.run(insn & OP_DRIVE_REV_R? -speedR : speedR);
		break;
	    }
	    case OP_SENSE_RAW: { // => (t, l?, a?, ω?, H?)
		int16_t vec3[3];
		send_uint32(millis() - boot_time_ms);
		if (insn & OP_SENSE_RAW_SONAR) {
		    send_uint16((int)(10.0 * sonar.distanceCm()));
		}
		if (insn & OP_SENSE_RAW_ACCEL) {
		    mpu.read_accel(vec3);
		    send_int16_triple(vec3);
		}
		if (insn & OP_SENSE_RAW_GYRO) {
		    mpu.read_gyro(vec3);
		    send_int16_triple(vec3);
		}
		if (insn & OP_SENSE_RAW_MAG) {
		    mpu.read_mag(vec3);
		    send_int16_triple(vec3);
		}
		break;
	    }
	    case OP_CALIBRATE: {
		mag_bias[0] = recv_int16();
		mag_bias[1] = recv_int16();
		mag_scale[0] = 1.0 + ldexp((float)recv_int16(), -15);
		mag_scale[1] = 1.0 + ldexp((float)recv_int16(), -15);
		break;
	    }
	    case OP_SENSE_CAL: { // => (t, l, ax, ωz, φ)
		int16_t vec3[3];
		send_uint32(millis() - boot_time_ms);
		send_uint16((int)(10.0 * sonar.distanceCm()));
		mpu.read_accel(vec3);
		send_int16(vec3[0]);
		mpu.read_gyro(vec3);
		send_int16(vec3[2]);
		send_angle(current_direction());
		break;
	    }
	    case OP_TURN: {
		float phi_stop = recv_angle();
		int force_dir =
		    (insn & OP_TURN_L)? -1 :
		    (insn & OP_TURN_R)? +1 : 0;
		turn(phi_stop, force_dir);
		break;
	    }
	    default:
		disp.display((uint8_t)2, 0x21);
		disp.display((uint8_t)3, 0x21);
		break;
	}
	disp.display((uint8_t)0, insn >> 4);
	disp.display((uint8_t)1, insn & 0xf);
    }
}
