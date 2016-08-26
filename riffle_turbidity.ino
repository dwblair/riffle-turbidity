#include <TimerOne.h>

#include "LowPower.h"         //https://github.com/rocketscream/Low-Power
#include <SPI.h>
#include <SD.h>

#include <Wire.h>
#include <RTClib.h>           //https://github.com/p-v-o-s/RTC_3231
#include<stdlib.h>


#define sleep_intervals 1 // every interval is 8 seconds

// every 2 minutes -- because each interval is 8 seconds

RTC_DS3231 rtc;

//led
#define led 9

#define chipSelect 7
#define SDpower 6

// This example uses the timer interrupt to blink an LED
// and also demonstrates how to share a variable between
// the interrupt and the main program.

const int turbidity_led = 3;  // the pin with a LED
int ledState = LOW; 
int num_ave = 20;
int time_interval_between_averaging = 20; // (milliseconds)

// note: need to add amplification factor

#define debug 1 // 0: don't print anything out; 1: print out debugging statements


void setup(void)
{
  
  pinMode(turbidity_led, OUTPUT);
  pinMode(10, OUTPUT);
  
  Serial.begin(9600);

  if (! rtc.begin()) {
    if (debug) Serial.println("Couldn't find RTC");
    }

digitalWrite(SDpower,LOW);
  
  pinMode(SDpower,OUTPUT);
    digitalWrite(SDpower,LOW);
    
     if (!SD.begin(chipSelect)) {
  if (debug) Serial.println("Card failed, or not present");

/*
   // indicate SD problem with fast blink
    while(1) {
      digitalWrite(led,HIGH);
      delay(200);
      digitalWrite(led,LOW);
       delay(200);
    }
    */
  }

pinMode(led, OUTPUT);     

}


// The interrupt will blink the LED, and keep
// track of how many times it has blinked.
volatile int flag = 0; //whether we're in the 'on' state

void blinkLED(void)
{
  if (ledState == LOW) {
    ledState = HIGH;
    flag = 1; 
    //blinkCount = blinkCount + 1;  // increase when LED turns on
  } else {
    ledState = LOW;
  }
  digitalWrite(turbidity_led, ledState);

}


// The main program will print the blink count
// to the Arduino Serial Monitor
void loop(void)
{

     
      // read A0
      int val1 = 0;
      for (int i=0; i<num_ave; i++) {
      val1 = val1+measureTurbidity();
      delay(time_interval_between_averaging);
      //int val1 = analogRead(0);
      
      }
      float turbidity_value = float(val1)/float(num_ave);

     
      int val2 = 0; // just have dummy variable for now
      // print to serial

      delay(1000);

      /*
      Serial.print(turbidity_value);
      Serial.print(" ");
      Serial.print(val2);
      Serial.print("\n");
      */
      

     float rtcTemp = rtc.getTempAsFloat();
    
      // write to SD card

      //get the time
  DateTime now = rtc.now();
  long unixNow = now.unixtime();
  
  
  // make a string for assembling the data to log:
  String dataString = "";
  
  // dataString += String(unixNow);
  dataString += now.unixtime();
  dataString += ",";
  dataString += now.year();
  dataString += "-";
  dataString += padInt(now.month(), 2);
  dataString += "-";
  dataString += padInt(now.day(), 2);
  dataString += " ";
  dataString += padInt(now.hour(), 2);
  dataString += ":";
  dataString += padInt(now.minute(), 2);
  dataString += ":";
  dataString += padInt(now.second(), 2);
  dataString += ",";
  char buffer[10];
  dataString += dtostrf(rtcTemp, 5, 2, buffer);
  dataString += ",";
  char buffer2[10];
  dataString += dtostrf(turbidity_value,5,2,buffer2);
  
  if(debug) Serial.println(dataString);
  
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog2.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
   

  //indicate successful write with short blink
  digitalWrite(led, HIGH);   
  delay(40);
  digitalWrite(led, LOW);

  }
  // if the file isn't open, pop up an error:
  else {
    if (debug) Serial.println("error opening datalog.txt");
  }

  
      // wait 
      //delay(100);

     sleep_for_8s_interval(sleep_intervals);

}

int measureTurbidity(void) {


  digitalWrite(10,HIGH); // power the circuit
  
Timer1.initialize(500);

Timer1.attachInterrupt(blinkLED); // blinkLED to run every 0.15 seconds

delay(10);

int val1 = analogRead(0);

delay(10); 

Timer1.detachInterrupt(); 
Timer1.stop();

digitalWrite(10,LOW); // de-power the circuit


return(val1);

}

void sleep_for_8s_interval(int numIntervals) { // will power down for numIntervals * 8 seconds 

for (int i=0;i<numIntervals;i++) {
LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
}
}

String padInt(int x, int pad) {
  String strInt = String(x);
  
  String str = "";
  
  if (strInt.length() >= pad) {
    return strInt;
  }
  
  for (int i=0; i < (pad-strInt.length()); i++) {
    str += "0";
  }
  
  str += strInt;
  
  return str;
}

String int2string(int x) {
  // formats an integer as a string assuming x is in 1/100ths
  String str = String(x);
  int strLen = str.length();
  if (strLen <= 2) {
    str = "0." + str;
  } else if (strLen <= 3) {
    str = str.substring(0, 1) + "." + str.substring(1);
  } else if (strLen <= 4) {
    str = str.substring(0, 2) + "." + str.substring(2);
  } else {
    str = "-9999";
  }
  
  return str;
}

