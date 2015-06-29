//This is the standalone version of ZoomControl. 
//For more info: 
//https://github.com/Craven112/ZoomControl
//
//Made by: Cristian Deenen
//For questions: CDeenen@outlook.com
//
//Required libraries: 
//USBH_MIDI https://github.com/YuuichiAkagawa/USBH_MIDI
//USB-Host_Shield_2.0 https://github.com/felis/USB_Host_Shield_2.0
//
//Inspired by: 
//https://www.youtube.com/watch?v=enK6Y30dAYs
//https://github.com/vegos/ArduinoMIDI
///////////////////////////////////////////////////////////////////////////////


#include <SPI.h>
#include <Usb.h>
#include <usbhub.h>
#include <usbh_midi.h>


USB  Usb;
USBH_MIDI  Midi(&Usb);

int number = 0; //number to send over USB

int ledTuner = 7;
int ledPower = 6;
int pinExpR = 9; //ring control pin (volume)
int pinExpT = 8; //tip control pin

int btnPrevPatch = A0;
int btnNextPatch = A1;
int btnPrevBank = A2;
int btnNextBank = A3;
int btnTuner = A5;

unsigned long timer=0;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Setup
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup()
{
pinMode(btnNextPatch,INPUT);pinMode(btnPrevPatch,INPUT);pinMode(btnNextBank,INPUT);pinMode(btnPrevBank,INPUT);pinMode(btnTuner,INPUT); //buttons
pinMode(ledPower,OUTPUT); pinMode(ledTuner,OUTPUT); //leds
pinMode(pinExpR,OUTPUT);pinMode(pinExpT,OUTPUT); //control pins

digitalWrite(pinExpT,HIGH);digitalWrite(pinExpR,LOW); //set control pins

  if (Usb.Init() == -1)
  {while(1);}
 
//blink leds at startup
digitalWrite(ledPower,HIGH); delay(250);
digitalWrite(ledPower,LOW); delay(250);
digitalWrite(ledPower,HIGH); delay(250);
digitalWrite(ledPower,LOW); delay(250);
digitalWrite(ledPower,HIGH);


}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Main Loop
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop()
{ 
  
    if (digitalRead(btnPrevPatch)==HIGH) {
      number--;
      if (number<0) {number=99;}
      SendMIDI(number);
      timer = millis();
      while (digitalRead(btnPrevPatch)==HIGH) {
        if (millis() - timer > 1000) {
      while(digitalRead(btnPrevPatch)==HIGH) {
        if (millis() - timer > 250) {
          number--;
           if (number<0) {number=99;}
            SendMIDI(number);
            timer = millis();
        }}}}
        delay(100);
    }
    if (digitalRead(btnNextPatch)==HIGH) {
      number++;
      if (number>99) {number=0;}
      SendMIDI(number);
      timer = millis();
      while (digitalRead(btnNextPatch)==HIGH) {
        if (millis() - timer > 1000) {
      while(digitalRead(btnNextPatch)==HIGH) {
        if (millis() - timer > 250) {
          number++;
           if (number>99) {number=0;}
            SendMIDI(number);
            timer = millis();
        }}}}
        delay(100);
    }
    if (digitalRead(btnPrevBank)==HIGH) {
      number=number-10;
      if (number<0) {number = number+100;}
      SendMIDI(number);
      timer = millis();
      while (digitalRead(btnPrevBank)==HIGH) {
        if (millis() - timer > 1000) {
      while (digitalRead(btnPrevBank)==HIGH) {
        if (millis() - timer > 250) {
          number = number-10;
          if (number<0) {number = number+100;}
          SendMIDI(number);
          timer = millis();
      }}}}
      delay(100);
    }
      
         if (digitalRead(btnNextBank)==HIGH) {
      number=number+10;
      if (number>99) {number = number-100;}
      SendMIDI(number);
      timer = millis();
      while (digitalRead(btnNextBank)==HIGH) {
        if (millis() - timer > 1000) {
      while (digitalRead(btnNextBank)==HIGH) {
        if (millis() - timer > 250) {
          number = number+10;
          if (number>99) {number = number-100;}
          SendMIDI(number);
          timer = millis();
      }}}}
      delay(100);
    } 
    
    if (digitalRead(btnTuner)==HIGH) { 
            tuner();      
        }

}



void tuner() { 

  digitalWrite(ledTuner,HIGH); //turn tuner led on
  digitalWrite(pinExpT,LOW); //set tip control pin to low, simulating footswitch press
  digitalWrite(pinExpR,LOW); //set ring control pin to low, simulating footswitch press
  delay(250); //delay to ensure registration of press
  digitalWrite(pinExpT,HIGH); //turn high to end the footwitch press
  while(1) {
    if (digitalRead(btnTuner)==LOW) {
      break;
    }
   }
   while (1) { //stay in loop until break
      
     if (digitalRead(btnTuner)==HIGH) { //if button is pressed
       digitalWrite(ledTuner,LOW); //turn off tuner led
       digitalWrite(pinExpT,LOW); //simulate footswitch press
       delay(250); //delay to ensure registration of press
       digitalWrite(pinExpT,HIGH); //return pin to high for normal operation
       break; //exit while loop
       }
     delay(10); //delay for stability
   }
}

void SendMIDI(byte number) //send program change over USB
{
  Usb.Task();
  if( Usb.getUsbTaskState() == USB_STATE_RUNNING ) //if USB is running, continue
  { 
    byte Message[2];                 // Construct the midi message (2 bytes)
    Message[0]=0xC0;                 // 0xC0 is for Program Change (Change to MIDI channel 0)
    Message[1]=number;               // patch [0-99]
    Midi.SendData(Message);          // Send the message
    delay(10); 
  }
}
