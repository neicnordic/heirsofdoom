/*************************************************************************
* File Name          : neicblueteamside.ino
* Author             : Neic Blue team
* Version            : V1.0.0
* Date               : 20160127
* Description        : Neic TEAM challange 
**************************************************************************/
#include <Makeblock.h>
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Wire.h>

MeUltrasonicSensor ultraSensor(PORT_3); //Ultrasonic module can ONLY be connected to port 3, 4, 6, 7, 8 of base shield.
Me7SegmentDisplay disp(PORT_6);
MeLimitSwitch limitSwitch2(PORT_8,SLOT2);
//port 8 
MeDCMotor motor1(M1);
MeDCMotor motor2(M2);
uint8_t motorSpeed = 255;
uint8_t turnSpeed = 200;

boolean turn=false;
boolean adjust=false;
#define filterSamples   26              // filterSamples should  be an odd number, no smaller than 3
int sensSmoothArray1 [filterSamples];   // array for holding raw sensor values for sensor1 

int CM,smoothData1 ;

void setup()
{

 Serial.begin(9600);
    Serial.println("Start.");
    delay(1000);
 
    motor1.run(motorSpeed);
    motor2.run(-motorSpeed);   
}


void loop()
{
        
        CM = ultraSensor.distanceCm();
        smoothData1 = digitalSmooth(CM, sensSmoothArray1);
        
	disp.display(CM);



if(!turn){
  if(limitSwitch2.touched()){
   motor1.run(turnSpeed);
   motor2.run(turnSpeed);
   delay(320);
  }
  else{
  
       if( (CM < 13) )
       {
       motor1.run(motorSpeed);
       motor2.run(0.5 * -motorSpeed);   
     
       }
       else if ((CM >15) || (CM< 1) )
       {
         motor1.run(0.5 * motorSpeed);
         motor2.run(-motorSpeed);
       }
       else {
       motor1.run(motorSpeed);
       motor2.run(-motorSpeed);   
       }
  }
       
}



 
        

}



int digitalSmooth(int rawIn, int *sensSmoothArray){     // "int *sensSmoothArray" passes an array to the function - the asterisk indicates the array name is a pointer
  int j, k, temp, top, bottom;
  long total;
  static int i;
 // static int raw[filterSamples];
  static int sorted[filterSamples];
  boolean done;

  i = (i + 1) % filterSamples;    // increment counter and roll over if necc. -  % (modulo operator) rolls over variable
  sensSmoothArray[i] = rawIn;                 // input new data into the oldest slot

  // Serial.print("raw = ");

  for (j=0; j<filterSamples; j++){     // transfer data array into anther array for sorting and averaging
    sorted[j] = sensSmoothArray[j];
  }

  done = 0;                // flag to know when we're done sorting              
  while(done != 1){        // simple swap sort, sorts numbers from lowest to highest
    done = 1;
    for (j = 0; j < (filterSamples - 1); j++){
      if (sorted[j] > sorted[j + 1]){     // numbers are out of order - swap
        temp = sorted[j + 1];
        sorted [j+1] =  sorted[j] ;
        sorted [j] = temp;
        done = 0;
      }
    }
  }

/*
  for (j = 0; j < (filterSamples); j++){    // print the array to debug
    Serial.print(sorted[j]); 
    Serial.print("   "); 
  }
  Serial.println();
*/

  // throw out top and bottom 15% of samples - limit to throw out at least one from top and bottom
  bottom = max(((filterSamples * 15)  / 100), 1); 
  top = min((((filterSamples * 85) / 100) + 1  ), (filterSamples - 1));   // the + 1 is to make up for asymmetry caused by integer rounding
  k = 0;
  total = 0;
  for ( j = bottom; j< top; j++){
    total += sorted[j];  // total remaining indices
    k++; 
    // Serial.print(sorted[j]); 
    // Serial.print("   "); 
  }

//  Serial.println();
//  Serial.print("average = ");
//  Serial.println(total/k);
  return total / k;    // divide by number of samples
}


