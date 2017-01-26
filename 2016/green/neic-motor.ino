// Silver medal winner in the NEIC Allhands 2016 competition.

// 100% programmed by Ulf Tigerstedt <ulf.tigerstedt@csc.fi>
// Other participants: 
// Jaakko Leinonen
// Diba
// Dmytro Karpenko
// Pål Pettersen
// Dejan (but he was mostly not present)
// Pontus 


// Idea is to traverse the track by moving counterclockwise, find out when the robot is approaching a wall and then turn 90 degrees left by calling the turnright() function

// Note: Needs a hit on the reset button after poweron. The ultrasonic sensor won't work properly without the reset.

#include <SoftwareSerial.h>

#include <Wire.h>

#include <Makeblock.h>





MeDCMotor motorR(PORT_1);
MeDCMotor motorL(PORT_2);

uint8_t motorSpeed = 100;
MeUltrasonicSensor ultraSensor(PORT_3); 





void setup() {
  // put your setup code here, to run once:
  //delay(1000);
  motorL.run(0); // value: between -255 and 255.
  motorR.run(0); // value: between -255 and 255
}

void loop() {
  // put your main code here, to run repeatedly:
  //forward();

  
  //delay(1000);
  
  while(ultrasound()) forward();
  delay(200);
  turnright();
  delay(200);
 
       
}

int ultrasound() {  
  float len;
  len=ultraSensor.distanceCm();
  if (len < 3.0) turnleft();
  if (len > 20.0) return(true);
  return(false);
}

void turnleft() {
  motorR.run(150);
  motorL.run(0);
  delay(443);
  motorL.run(0);
}
void turnright() {
  motorR.run(-150);
  motorL.run(0);
  delay(443);
  motorR.run(0);
}

void forward() {
  motorL.run(-100);
  motorR.run(-95);
  delay(250);
  motorL.run(0);
  motorR.run(0);
}


