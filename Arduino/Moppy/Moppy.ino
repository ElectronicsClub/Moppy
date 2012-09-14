#include <TimerOne.h>

boolean firstRun = true; // Used for one-run-only stuffs;

//First pin being used for floppies, and the last pin.  Used for looping over all pins.
#define FIRST_PIN 2
#define LAST_PIN 17
#define RESOLUTION 40 //Microsecond resolution for notes

//Sound production modes:
//MOV moves the heads through the full extent of their travel.
//It is showier, but produces muddy sound with lots of tranny noise.
#define MOV true
//VIB moves the heads back and forth over only one track.
//IT is louder and produces clearer sound.
#define VIB false

/*NOTE: Many of the arrays below contain unused indexes.  This is 
 to prevent the Arduino from having to convert a pin input to an alternate
 array index and save as many cycles as possible.  In other words information 
 for pin 2 will be stored in index 2, and information for pin 4 will be 
 stored in index 4.*/

/*An array of maximum track positions for each step-control pin.  Even pins
 are used for control, so only even numbers need a value here.  3.5" Floppies have
 80 tracks, 5.25" have 50.  These should be doubled, because each tick is now
 half a position (use 158 and 98).
 */
const byte maxPosition[] = {
  0,0,158,0,158,0,158,0,158,0,158,0,158,0,158,0,158,0
};

//Array to track the current position of each floppy head.  (Only even indexes (i.e. 2,4,6...) are used)
byte currentPosition[] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

/*Array to keep track of state of each pin.  Even indexes track the control-pins for toggle purposes.  Odd indexes
 track direction-pins.  LOW = forward, HIGH=reverse
 */
boolean currentState[] = {
  0,0,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW
};

//Array to keep track of which sound mode is used on each pin.
boolean mode[] = {
  0,0,VIB,0,VIB,0,VIB,0,VIB,0,VIB,0,VIB,0,VIB,0,VIB,0,VIB
};

//Current period assigned to each pin.  0 = off.  Each period is of the length specified by the RESOLUTION
//variable above.  i.e. A period of 10 is (RESOLUTION x 10) microseconds long.
unsigned int currentPeriod[] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

//Current tick
unsigned int currentTick[] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 
};

byte lastSerialPeek;
byte modeControl;

//Setup pins (Even-odd pairs for step control and direction
void setup(){
  pinMode(13, OUTPUT);// Pin 13 has an LED connected on most Arduino boards
  pinMode(2, OUTPUT); // Step control 1
  pinMode(3, OUTPUT); // Direction 1
  pinMode(4, OUTPUT); // Step control 2
  pinMode(5, OUTPUT); // Direction 2
  pinMode(6, OUTPUT); // Step control 3
  pinMode(7, OUTPUT); // Direction 3
  pinMode(8, OUTPUT); // Step control 4
  pinMode(9, OUTPUT); // Direction 4
  pinMode(10, OUTPUT); // Step control 5
  pinMode(11, OUTPUT); // Direction 5
  pinMode(12, OUTPUT); // Step control 6
  pinMode(13, OUTPUT); // Direction 6
  pinMode(14, OUTPUT); // Step control 7
  pinMode(15, OUTPUT); // Direction 7
  pinMode(16, OUTPUT); // Step control 8
  pinMode(17, OUTPUT); // Direction 8
  
  pinMode(18, OUTPUT); // Power supply control
  digitalWrite(18, HIGH); //Disable the power supply

  Timer1.initialize(RESOLUTION); // Set up a timer at the defined resolution
  Timer1.attachInterrupt(tick); // Attach the tick function

  Serial.begin(9600);
}

void loop(){
  //The first loop, reset all the drives, and wait 2 seconds...
  if (firstRun){
    firstRun = false;
    resetAll();
    delay(2000);
  }

  //Only read if we have 
  if (Serial.available() > 2){
    lastSerialPeek=Serial.peek();
    // Watch for special control messages.
    // 100 resets the drives
    if (lastSerialPeek == 100) {
      resetAll();
      // Flush any remaining messages.
      while(Serial.available() > 0){
        Serial.read();
      }
    //126 turns the power supply on
    }else if(lastSerialPeek == 126){
      digitalWrite(18, LOW);
      // Flush any remaining messages.
      while(Serial.available() > 0){
        Serial.read();
      }
    //127 turns the power supply off
    }else if(lastSerialPeek == 127){
      digitalWrite(18, HIGH);
      // Flush any remaining messages.
      while(Serial.available() > 0){
        Serial.read();
      }
    //200-series messages put the drives in vibrate mode
    }else if(lastSerialPeek > 100 && lastSerialPeek < 110){
      modeControl=(lastSerialPeek-100)*2;
      if(modeControl>=FIRST_PIN && modeControl<=LAST_PIN && modeControl%2==0){
        mode[modeControl]=VIB;
        if(currentPosition[modeControl]>6){
          currentState[modeControl+1] = HIGH;
          digitalWrite(modeControl+1,HIGH);
        }else if(currentPosition[modeControl]<2){
          currentState[modeControl+1] = LOW;
          digitalWrite(modeControl+1,LOW);
        }
      }
      // Flush any remaining messages.
      while(Serial.available() > 0){
        Serial.read();
      }
    //300-series messages put the drives in move mode
    }else if(lastSerialPeek > 110 && lastSerialPeek < 120){
      modeControl=(lastSerialPeek-110)*2;
      if(modeControl>=FIRST_PIN && modeControl<=LAST_PIN && modeControl%2==0){
        mode[modeControl]=MOV;
      }
      // Flush any remaining messages.
      while(Serial.available() > 0){
        Serial.read();
      }
    }else{
      currentPeriod[Serial.read()] = (Serial.read() << 8) | Serial.read();
    }
  }
}

/*
Called by the timer inturrupt at the specified resolution.
 */
void tick(){
  /* 
   If there is a period set for control pin 2, count the number of
   ticks that pass, and toggle the pin if the current period is reached.
   */
  if (currentPeriod[2]>0){
    currentTick[2]++;
    if (currentTick[2] >= currentPeriod[2]){
      togglePin(2,3);
      currentTick[2]=0;
    }
  }
  if (currentPeriod[4]>0){
    currentTick[4]++;
    if (currentTick[4] >= currentPeriod[4]){
      togglePin(4,5);
      currentTick[4]=0;
    }
  }
  if (currentPeriod[6]>0){
    currentTick[6]++;
    if (currentTick[6] >= currentPeriod[6]){
      togglePin(6,7);
      currentTick[6]=0;
    }
  }
  if (currentPeriod[8]>0){
    currentTick[8]++;
    if (currentTick[8] >= currentPeriod[8]){
      togglePin(8,9);
      currentTick[8]=0;
    }
  }
  if (currentPeriod[10]>0){
    currentTick[10]++;
    if (currentTick[10] >= currentPeriod[10]){
      togglePin(10,11);
      currentTick[10]=0;
    }
  }
  if (currentPeriod[12]>0){
    currentTick[12]++;
    if (currentTick[12] >= currentPeriod[12]){
      togglePin(12,13);
      currentTick[12]=0;
    }
  }
  if (currentPeriod[14]>0){
    currentTick[14]++;
    if (currentTick[14] >= currentPeriod[14]){
      togglePin(14,15);
      currentTick[14]=0;
    }
  }
  if (currentPeriod[16]>0){
    currentTick[16]++;
    if (currentTick[16] >= currentPeriod[16]){
      togglePin(16,17);
      currentTick[16]=0;
    }
  }
}

void togglePin(byte pin, byte direction_pin){
  if(mode[pin]==MOV){
    //Switch directions if end has been reached
    if (currentPosition[pin] >= maxPosition[pin]) {
      currentState[direction_pin] = HIGH;
      digitalWrite(direction_pin,HIGH);
    } else if (currentPosition[pin] <= 0) {
      currentState[direction_pin] = LOW;
      digitalWrite(direction_pin,LOW);
    }
  }else{
    //Limit travel to 2 tracks, vibrating heads back and forth
    if (currentPosition[pin] >= 6) {
      currentState[direction_pin] = HIGH;
      digitalWrite(direction_pin,HIGH);
    } else if (currentPosition[pin] <= 4) {
      currentState[direction_pin] = LOW;
      digitalWrite(direction_pin,LOW);
    }
  }

  //Update currentPosition
  if (currentState[direction_pin] == HIGH){
    currentPosition[pin]--;
  } else {
    currentPosition[pin]++;
  }

  //Pulse the control pin
  digitalWrite(pin,currentState[pin]);
  currentState[pin] = ~currentState[pin];
}


//
//// UTILITY FUNCTIONS
//

//For a given controller pin, runs the read-head all the way back to 0
void reset(byte pin){
  digitalWrite(pin+1,HIGH); // Go in reverse
  for (byte s=0;s<maxPosition[pin];s+=2){ //Half max because we're stepping directly (no toggle)
    digitalWrite(pin,HIGH);
    digitalWrite(pin,LOW);
    delay(5);
  }
  currentPosition[pin] = 0; // We're reset.
  digitalWrite(pin+1,LOW);
  currentPosition[pin+1] = 0; // Ready to go forward.
}

//Resets all the pins
void resetAll(){
  //Stop all notes (don't want to be playing during/after reset)
  for (byte p=FIRST_PIN;p<=LAST_PIN;p+=2){
    currentPeriod[p] = 0; // Stop playing notes
  }

  // New all-at-once reset
  for (byte s=0;s<80;s++){ // For max drive's position
    for (byte p=FIRST_PIN;p<=LAST_PIN;p+=2){
      digitalWrite(p+1,HIGH); // Go in reverse
      digitalWrite(p,HIGH);
      digitalWrite(p,LOW);
    }
    delay(5);
  }

  for (byte p=FIRST_PIN;p<=LAST_PIN;p+=2){
    currentPosition[p] = 0; // We're reset.
    digitalWrite(p+1,LOW);
    currentState[p+1] = 0; // Ready to go forward.
  }
}

