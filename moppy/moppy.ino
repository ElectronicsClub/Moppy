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

//Min and max track position for drives
#define MIN_POSITION 0
#define MAX_POSITION 158

//Microsecond resolution for notes
#define RESOLUTION 20

//Array to track the current position of each floppy head
byte currentPosition[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

/*
 * Array of current periods assigned to each pin.  0 = off.
 * Each period is of the length specified by the RESOLUTION
 * variable above.  i.e. A period of 10 is (RESOLUTION x 10)
 * microseconds long.
 */
unsigned int currentPeriod[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

/*
 * Array of current ticks for each pin.  These count up to the
 * currentPeriod values in the above array.  When the period
 * values are reached, the head is stepped.
 */
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
    
    //Watch for special control messages
    //100 resets the drives
    if(Serial.peek()==100){
      resetAll();
      while(Serial.available()>0){
        Serial.read();
      }
    //126 turns the power supply on
    }else if(Serial.peek()==126){
      cbi(portc,4);
      PORTC=portc;
      //Wait for it to activate
      delay(500);
      //Reset the drives
      resetAll();
      while(Serial.available()>0){
        Serial.read();
      }
    //127 turns the power supply off
    }else if(Serial.peek()==127){
      resetAll();
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
      if(currentPosition[2]>=MAX_POSITION){
        sbi(portd,3);
        sbi(portddirection,3);
      }else if(currentPosition[2]<=MIN_POSITION){
        cbi(portd,3);
        cbi(portddirection,3);
      }

      //Update currentPosition
      if(ebi(portd,3)){
        currentPosition[2]--;
      }else{
        currentPosition[2]++;
      }

      //Pulse the step pin
      if(ebi(portd,2)){
        cbi(portd,2);
      }else{
        sbi(portd,2);
      }
      currentTick[2]=0;
    }
  }
  if(currentPeriod[4]>0){
    currentTick[4]++;
    if(currentTick[4]>=currentPeriod[4]){
      if(currentPosition[4]>=MAX_POSITION){
        sbi(portd,5);
        sbi(portddirection,5);
      }else if(currentPosition[4]<=MIN_POSITION){
        cbi(portd,5);
        cbi(portddirection,5);
      }

      if(ebi(portd,5)){
        currentPosition[4]--;
      }else{
        currentPosition[4]++;
      }

      if(ebi(portd,4)){
        cbi(portd,4);
      }else{
        sbi(portd,4);
      }
      currentTick[4]=0;
    }
  }
  if(currentPeriod[6]>0){
    currentTick[6]++;
    if(currentTick[6]>=currentPeriod[6]){
      if(currentPosition[6]>=MAX_POSITION){
        sbi(portd,7);
        sbi(portddirection,7);
      }else if(currentPosition[6]<=MIN_POSITION){
        cbi(portd,7);
        cbi(portddirection,7);
      }

      if(ebi(portd,7)){
        currentPosition[6]--;
      }else{
        currentPosition[6]++;
      }

      if(ebi(portd,6)){
        cbi(portd,6);
      }else{
        sbi(portd,6);
      }
      currentTick[6]=0;
    }
  }
  if(currentPeriod[8]>0){
    currentTick[8]++;
    if(currentTick[8]>=currentPeriod[8]){
      if(currentPosition[8]>=MAX_POSITION){
        sbi(portb,1);
        sbi(portbdirection,1);
      }else if(currentPosition[8]<=MIN_POSITION){
        cbi(portb,1);
        cbi(portbdirection,1);
      }

      if(ebi(portb,1)){
        currentPosition[8]--;
      }else{
        currentPosition[8]++;
      }

      if(ebi(portb,0)){
        cbi(portb,0);
      }else{
        sbi(portb,0);
      }
      currentTick[8]=0;
    }
  }
  if(currentPeriod[10]>0){
    currentTick[10]++;
    if(currentTick[10]>=currentPeriod[10]){
      if(currentPosition[10]>=MAX_POSITION){
        sbi(portb,3);
        sbi(portbdirection,3);
      }else if(currentPosition[10]<=MIN_POSITION){
        cbi(portb,3);
        cbi(portbdirection,3);
      }

      if(ebi(portb,3)){
        currentPosition[10]--;
      }else{
        currentPosition[10]++;
      }

      if(ebi(portb,2)){
        cbi(portb,2);
      }else{
        sbi(portb,2);
      }
      currentTick[10]=0;
    }
  }
  if(currentPeriod[12]>0){
    currentTick[12]++;
    if(currentTick[12]>=currentPeriod[12]){
      if(currentPosition[12]>=MAX_POSITION){
        sbi(portb,5);
        sbi(portbdirection,5);
      }else if(currentPosition[12]<=MIN_POSITION){
        cbi(portb,5);
        cbi(portbdirection,5);
      }

      if(ebi(portb,5)){
        currentPosition[12]--;
      }else{
        currentPosition[12]++;
      }

      if(ebi(portb,4)){
        cbi(portb,4);
      }else{
        sbi(portb,4);
      }
      currentTick[12]=0;
    }
  }
  if(currentPeriod[14]>0){
    currentTick[14]++;
    if(currentTick[14]>=currentPeriod[14]){
      if(currentPosition[14]>=MAX_POSITION){
        sbi(portc,1);
        sbi(portcdirection,1);
      }else if(currentPosition[14]<=MIN_POSITION){
        cbi(portc,1);
        cbi(portcdirection,1);
      }

      if(ebi(portc,1)){
        currentPosition[14]--;
      }else{
        currentPosition[14]++;
      }

      if(ebi(portc,0)){
        cbi(portc,0);
      }else{
        sbi(portc,0);
      }
      currentTick[14]=0;
    }
  }
  if(currentPeriod[16]>0){
    currentTick[16]++;
    if(currentTick[16]>=currentPeriod[16]){
      if(currentPosition[16]>=MAX_POSITION){
        sbi(portc,3);
        sbi(portcdirection,3);
      }else if(currentPosition[16]<=MIN_POSITION){
        cbi(portc,3);
        cbi(portcdirection,3);
      }

      if(ebi(portc,3)){
        currentPosition[16]--;
      }else{
        currentPosition[16]++;
      }

      if(ebi(portc,2)){
        cbi(portc,2);
      }else{
        sbi(portc,2);
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
  
  //Interrupts must be disabled while resetting, otherwise bad things happen
  cli();
  
  //Set all the pins to reverse
  PORTD=B10101000;
  PORTB=B00101010;
  PORTC=B00101010;
  delay(5);
  
  //Toggle all the step pins at the same time
  for(byte s=0;s<80;s++){
    PORTD=B11111100;
    PORTB=B00111111;
    PORTC=B00111111;
    delay(100);
    PORTD=B10101000;
    PORTB=B00101010;
    PORTC=B00101010;
    delay(100);
  }
  
  //Reset all of the status arrays
  for (byte p=FIRST_PIN;p<=LAST_PIN;p+=2){
    currentPosition[p]=0;
    currentPeriod[p]=0;
    currentTick[p]=0;
  }
  
  //Reset the outputs to forward
  PORTD=portd=portddirection=B00000000;
  PORTB=portb=portbdirection=B00000000;
  PORTC=portc=portcdirection=B00000000;
  
  //Re-enable interrupts
  sei();
}
