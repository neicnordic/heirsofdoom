// vim: ts=4 sw=4 expantab
//#include "Makeblock.h"
#include "MeOrion.h"  // either this include or Makeblock.h, depending on version of the Makeblock library

MeUltrasonicSensor us(PORT_3); // note, electronics may only be connected to a subset of the ports
Me7SegmentDisplay sd(PORT_4);
MeLimitSwitch switch1(PORT_7,SLOT2); // note, the switches may only work on slot2 of the RJ25 adapters
MeDCMotor mr(M2);
MeDCMotor ml(M1);

void setup()
{
    Serial.begin(9600);
}

void loop()
{
    // read from ultrasonic sensor
    double distance = us.distanceCm();
    Serial.println(5);
    static int first = 1;
    int speed = 255;

    // test micro switch
    /*if (switch1.touched()) {
        sd.display(1);
        //delay(1000);
    } else {
        sd.display(0);
        //delay(1000);
    }
    */
    // display 2016 on 7-segment module
    //sd.display((int)distance);

    if (first) {
        ml.run(speed);
        //sd.display(100);
        mr.run(speed);
        delay(50);
    }

    if ((distance < 15.0 && distance > 0.0)) {
        ml.run(-speed);
        //sd.display(-100);
        mr.run(speed);
        delay(230);
    
    } else if (switch1.touched()) {
        ml.run(-0.6*speed);
        mr.run(speed);
        delay(23);
    } else if (!switch1.touched()) {
        ml.run(speed);
        mr.run(0.8*speed);
        delay(23);
        mr.run(speed);
    
    } else {
        ml.run(speed);
        //sd.display(100);
        mr.run(speed);
        delay(50);
        //ml.stop();
        //mr.stop();
    }
    first = 0;
}
