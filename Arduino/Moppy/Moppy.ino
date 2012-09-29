//Necessary for timer interrupts
#include <TimerOne.h>

//Clears a specified bit
#ifndef cbi
  #define cbi(sfr, bit) (_SFR_BYTE(sfr)&=~_BV(bit))
#endif

//Sets a specified bit
#ifndef sbi
  #define sbi(sfr, bit) (_SFR_BYTE(sfr)|=_BV(bit))
#endif

//Examines a specified bit
//Use only for boolean comparisons
#ifndef ebi
  #define ebi(var, bit) (var&(1<<bit))
#endif

//First and last pin being used for floppies
#define FIRST_PIN 2
#define LAST_PIN 17

//Microsecond resolution for notes
#define RESOLUTION 20

/*
 * Array of maximum track positions for each step-control pin.  Even pins
 * are used for control, so only even numbers need a value here.  3.5" Floppies have
 * 80 tracks, 5.25" have 50.  These should be doubled, because each tick is now
 * half a position (use 158 and 98).
 */
const byte maxPosition[]={0,0,158,0,158,0,158,0,158,0,158,0,158,0,158,0,158,0};

//Array to track the current position of each floppy head
byte currentPosition[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

/*
 * Array of current periods assigned to each pin.  0 = off.  Each period is of the length specified by the RESOLUTION
 * variable above.  i.e. A period of 10 is (RESOLUTION x 10) microseconds long.
 */
unsigned int currentPeriod[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

//Array of current ticks for each pin
unsigned int currentTick[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

/*
 * Bytes to hold the step and direction outputs for all the drives.
 * This isn't an elegant solution, but it is extremely efficient
 * because these variables can be pushed directly to the output
 * registers of the AVR.  Separate variables for "direction" are
 * required because direction must be pushed a measurable amount
 * of time before stepping, otherwise the drives will not see the
 * switched direction.
 */
byte portd=0;
byte portddirection=0;
byte portb=0;
byte portbdirection=0;
byte portc=0;
byte portcdirection=0;

//Holds the last value peeked from serial, used for special commands
byte lastSerialPeek;

//Initialize the board and get ready to run
void setup(){
  
  //Initialize all pins as outputs
  DDRD=DDRD|B11111100;
  DDRB=DDRB|B00111111;
  DDRC=DDRC|B00111111;
  
  //Make sure the power supply stays off
  sbi(portc,4);
  
  //Initialize all the output registers
  PORTD=portd;
  PORTB=portb;
  PORTC=portc;

  //Start the timer and attach the tick function
  Timer1.initialize(RESOLUTION); 
  Timer1.attachInterrupt(tick);

  //Start serial communications
  //Higher bitrate reduces pauses between notes
  Serial.begin(115200);
}

//Listen for serial data
void loop(){
  
  //Don't read from serial until a full message (3 bytes) is available
  if(Serial.available()>2){
    //Peek at the value from serial
    lastSerialPeek=Serial.peek();
    
    //Watch for special control messages
    //100 resets the drives
    if(lastSerialPeek==100){
      resetAll();
      //Flush any remaining messages.
      while(Serial.available()>0){
        Serial.read();
      }
    //126 turns the power supply on
    }else if(lastSerialPeek==126){
      cbi(portc,4);
      PORTC=portc;
      //Wait for it to activate and reset the drives
      delay(1500);
      resetAll();
      while(Serial.available()>0){
        Serial.read();
      }
    //127 turns the power supply off
    }else if(lastSerialPeek==127){
      sbi(portc,4);
      PORTC=portc;
      while(Serial.available()>0){
        Serial.read();
      }
    }else{
      //Read and store the period data for the drives
      currentPeriod[Serial.read()]=(Serial.read()<<8)|Serial.read();
    }
  }
}

//Do the dirty work of actually toggling outputs to move the drive heads
void tick(){
  
  /* 
   * If there is a period set for control pin 2, count the number of
   * ticks that pass, and toggle the pin if the current period is reached.
   */
  if(currentPeriod[2]>0){
    currentTick[2]++;
    if(currentTick[2]>=currentPeriod[2]){
      if(currentPosition[2]>=maxPosition[2]){
        sbi(portd,3);
      } else if(currentPosition[2]<=2){
        cbi(portd,3);
      }

      //Update currentPosition
      if(ebi(portd,3)){
        currentPosition[2]--;
      }else{
        currentPosition[2]++;
      }

      //Pulse the control pin
      if(ebi(portd,2)){
        cbi(portd,2);
        cbi(portddirection,2);
      }else{
        sbi(portd,2);
        sbi(portddirection,2);
      }
      currentTick[2]=0;
    }
  }
  if(currentPeriod[4]>0){
    currentTick[4]++;
    if(currentTick[4]>=currentPeriod[4]){
      if(currentPosition[4]>=maxPosition[4]){
        sbi(portd,5);
      } else if(currentPosition[4]<=2){
        cbi(portd,5);
      }

      if(ebi(portd,5)){
        currentPosition[4]--;
      }else{
        currentPosition[4]++;
      }

      if(ebi(portd,4)){
        cbi(portd,4);
        cbi(portddirection,4);
      }else{
        sbi(portd,4);
        sbi(portddirection,4);
      }
      currentTick[4]=0;
    }
  }
  if(currentPeriod[6]>0){
    currentTick[6]++;
    if(currentTick[6]>=currentPeriod[6]){
      if(currentPosition[6]>=maxPosition[6]){
        sbi(portd,7);
      } else if(currentPosition[6]<=2){
        cbi(portd,7);
      }

      if(ebi(portd,7)){
        currentPosition[6]--;
      }else{
        currentPosition[6]++;
      }

      if(ebi(portd,6)){
        cbi(portd,6);
        cbi(portddirection,6);
      }else{
        sbi(portd,6);
        sbi(portddirection,6);
      }
      currentTick[6]=0;
    }
  }
  if(currentPeriod[8]>0){
    currentTick[8]++;
    if(currentTick[8]>=currentPeriod[8]){
      if(currentPosition[8]>=maxPosition[8]){
        sbi(portb,1);
      } else if(currentPosition[8]<=2){
        cbi(portb,1);
      }

      if(ebi(portb,1)){
        currentPosition[8]--;
      }else{
        currentPosition[8]++;
      }

      if(ebi(portb,0)){
        cbi(portb,0);
        cbi(portbdirection,0);
      }else{
        sbi(portb,0);
        sbi(portbdirection,0);
      }
      currentTick[8]=0;
    }
  }
  if(currentPeriod[10]>0){
    currentTick[10]++;
    if(currentTick[10]>=currentPeriod[10]){
      if(currentPosition[10]>=maxPosition[10]){
        sbi(portb,3);
      } else if(currentPosition[10]<=2){
        cbi(portb,3);
      }

      if(ebi(portb,3)){
        currentPosition[10]--;
      }else{
        currentPosition[10]++;
      }

      if(ebi(portb,2)){
        cbi(portb,2);
        cbi(portbdirection,2);
      }else{
        sbi(portb,2);
        sbi(portbdirection,2);
      }
      currentTick[10]=0;
    }
  }
  if(currentPeriod[12]>0){
    currentTick[12]++;
    if(currentTick[12]>=currentPeriod[12]){
      if(currentPosition[12]>=maxPosition[12]){
        sbi(portb,5);
      } else if(currentPosition[12]<=2){
        cbi(portb,5);
      }

      if(ebi(portb,5)){
        currentPosition[12]--;
      }else{
        currentPosition[12]++;
      }

      if(ebi(portb,4)){
        cbi(portb,4);
        cbi(portbdirection,4);
      }else{
        sbi(portb,4);
        sbi(portbdirection,4);
      }
      currentTick[12]=0;
    }
  }
  if(currentPeriod[14]>0){
    currentTick[14]++;
    if(currentTick[14]>=currentPeriod[14]){
      if(currentPosition[14]>=maxPosition[14]){
        sbi(portc,1);
      } else if(currentPosition[14]<=2){
        cbi(portc,1);
      }

      if(ebi(portc,1)){
        currentPosition[14]--;
      }else{
        currentPosition[14]++;
      }

      if(ebi(portc,0)){
        cbi(portc,0);
        cbi(portcdirection,0);
      }else{
        sbi(portc,0);
        sbi(portcdirection,0);
      }
      currentTick[14]=0;
    }
  }
  if(currentPeriod[16]>0){
    currentTick[16]++;
    if(currentTick[16]>=currentPeriod[16]){
      if(currentPosition[16]>=maxPosition[16]){
        sbi(portc,3);
      } else if(currentPosition[16]<=2){
        cbi(portc,3);
      }

      if(ebi(portc,3)){
        currentPosition[16]--;
      }else{
        currentPosition[16]++;
      }

      if(ebi(portc,2)){
        cbi(portc,2);
        cbi(portcdirection,2);
      }else{
        sbi(portc,2);
        sbi(portcdirection,2);
      }
      currentTick[16]=0;
    }
  }
  
  //Set directions only first
  PORTD=portddirection;
  PORTB=portbdirection;
  PORTC=portcdirection;
  
  //Then set everything
  PORTD=portd;
  PORTB=portb;
  PORTC=portc;
  
  //Keep direction variables in sync with main variables
  portddirection=portd;
  portbdirection=portb;
  portcdirection=portc;
}

//Resets all the pins
void resetAll(){
  
  //Set all the pins to reverse
  PORTD=B11111100;
  PORTB=B00111111;
  PORTC=B00111111;
  
  //Toggle all the step pins at the same time
  for(byte s=0;s<80;s++){
    PORTD=B10101000;
    PORTB=B00101010;
    PORTC=B00101010;
    PORTD=B11111100;
    PORTB=B00111111;
    PORTC=B00111111;
    delay(5);
  }
  
  //Reset the current position array
  for (byte p=FIRST_PIN;p<=LAST_PIN;p+=2){
    currentPosition[p] = 0; // We're reset.
  }
  
  //Reset the outputs to forward
  PORTD=portd=portddirection=B00000000;
  PORTB=portb=portbdirection=B00000000;
  PORTC=portc=portcdirection=B00000000;
}

