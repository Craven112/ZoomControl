//This is code to control your Zoom B3/G3 using an Arduino and USB host shield. 
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

const int NUMBER_OF_FIELDS = 6; // how many comma separated fields we expect
int fieldIndex = 0;            // the current field being received
int values[NUMBER_OF_FIELDS];   // array holding values for all the fields

int number = 0; //number to send over USB
int patchOld = 999; //to check if patch has changed
int patchA; //number of patchA
int patchB; //number of patchB
int patchC; //number of patchC
int currentPatch; //Selected patch [1,2,3]

int patchSend=1; //patch to send back to android [1,2,3]
int BTcheck = 0; //check if android has received song change
int BTcheck2 = 0; //check if android has received patch change
int index; //song number
int indexBack; //song number that's sent back to android
int selectedPatch; //selected number to send over USB

int bounce = 200; //delay to debounce buttons
int tunerCheck=0; //check if tuner is on
int now; //time to decide when BT message must be sent again

int volume; //output volulme [0-255]

int ledA = 2;
int ledB = 3;
int ledC = 4;
int ledTuner = 7;
int ledPowerRed = 6;
int ledPowerGreen = 5;
int pinExpR = 9; //ring control pin (volume)
int pinExpT = 8; //tip control pin

int btnA = A0;
int btnB = A1;
int btnC = A2;
int btnDown = A3;
int btnUp = A4;
int btnTuner = A5;
int returnCheck = 0;

unsigned long timer=0;
int tuner2 = 1;
int numberOld = -1;
int failSafe = 0;
int failsafeA = 0;
int failsafeB = 1;
int failsafeC = 2;

//int BTled = 8;
unsigned long BTnow;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Setup
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup()
{Serial.begin(115200); //start serial

pinMode(btnA,INPUT);pinMode(btnB,INPUT);pinMode(btnC,INPUT);pinMode(btnUp,INPUT);pinMode(btnDown,INPUT);pinMode(btnTuner,INPUT); //buttons
pinMode(ledA,OUTPUT);pinMode(ledB,OUTPUT);pinMode(ledC,OUTPUT);pinMode(ledPowerGreen,OUTPUT); pinMode(ledPowerRed,OUTPUT); pinMode(ledTuner,OUTPUT); //leds
pinMode(pinExpR,OUTPUT);pinMode(pinExpT,OUTPUT); //control pins

digitalWrite(pinExpT,HIGH);digitalWrite(pinExpR,LOW); //set control pins

  if (Usb.Init() == -1)
  {while(1);}
 
//blink leds at startup
digitalWrite(ledA,HIGH); digitalWrite(ledB,HIGH); digitalWrite(ledC,HIGH); digitalWrite(ledPowerRed,HIGH); digitalWrite(ledPowerGreen,HIGH); delay(500);
digitalWrite(ledA,LOW); digitalWrite(ledB,LOW); digitalWrite(ledC,LOW); digitalWrite(ledPowerRed,LOW); digitalWrite(ledPowerGreen,LOW); delay(500);

for (int m=0; m<2; m++) {
  for (int n=2; n<5; n++) {
    digitalWrite(n,HIGH);
    digitalWrite(n-1,LOW);
    delay(100);
  }
  digitalWrite(ledC,LOW);
  delay(100);
  for (int n=4; n>1; n--) {
    digitalWrite(n,HIGH);
    digitalWrite(n+1,LOW);
    delay(100);
  }
  digitalWrite(ledA,LOW);
  delay(100);
}



volume = 100; //set volume to max



digitalWrite(ledPowerRed,HIGH);

if (digitalRead(btnTuner)==HIGH) {returnCheck = 1; standalone();}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Main Loop
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop()
{ 
  readSerial(); //check serial for BT data and act on it
  
  buttons(); //check button status and act on it
  
  //setVolume(); //set the volume
  
  BTstillOn(); //check if BT is (still) connected
  
  tunerSend();
  
  failsafe();
  
 // if (returnCheck == 1) {
  //  standalone();
  //}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//SubGroups
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void buttons()
{
//Up and down buttons/////////////////////////
 if (digitalRead(btnUp)==HIGH) { //if buttonUP is pressed  
   indexBack++; //add 1 to the index to send back
   SendBT(); //send new index
   BTcheck=1; //set to 1 for verification
   while(1) {
    if (digitalRead(btnUp)==LOW) {
      break;
    }
   }
   delay(2*bounce); //debounce delay
 }
 
 if (digitalRead(btnDown)==HIGH) { 
   indexBack--; 
   SendBT(); 
   BTcheck=1; 
      while(1) {
    if (digitalRead(btnDown)==LOW) {
      break;
    }
   }
   delay(2*bounce);
 }
   
//Patch buttons//////////////////////////////
if (digitalRead(btnA)==HIGH) { //if patchA button is pressed
  patchSend = 1; //set to 1 to let android know what patch has been selected
  SendMIDI(patchA); //send patchA over USB
  SendBT2(); //send over BT
  BTcheck2=1; //for verification
  digitalWrite(ledA,HIGH); //turn led patchA on
  digitalWrite(ledB,LOW); //turn led patchB off
  digitalWrite(ledC,LOW); //turn led patchC off
     while(1) {
    if (digitalRead(btnA)==LOW) {
      break;
    }
   }
  delay(2*bounce); //delay to debounce
}

if (digitalRead(btnB)==HIGH) {
  patchSend = 2; 
  SendMIDI(patchB); 
  SendBT2(); 
  BTcheck2=1; 
  digitalWrite(ledA,LOW);
  digitalWrite(ledB,HIGH);
  digitalWrite(ledC,LOW);
     while(1) {
    if (digitalRead(btnB)==LOW) {
      break;
    }
   }
  delay(bounce);
}

if (digitalRead(btnC)==HIGH) {
  patchSend = 3; 
  SendMIDI(patchC); 
  SendBT2(); 
  BTcheck2=1; 
  digitalWrite(ledA,LOW);
  digitalWrite(ledB,LOW);
  digitalWrite(ledC,HIGH);
     while(1) {
    if (digitalRead(btnC)==LOW) {
      break;
    }
   }
  delay(bounce);
}
  
//Verify with android device/////////////////////  
//for song change
if (BTcheck == 1 && index == indexBack) { //if verification is needed & verified
    BTcheck = 0; //turn off
}

if (BTcheck == 1 && index != indexBack) { //if verification is needed & not verified
  if (millis()-now > 100) { //if message has not been sent in 100ms
    now = millis(); //set timer back to 0
    SendBT(); //send data
  }
}

if (BTcheck == 0) { //if verification is not needed
  indexBack = index; //set indexBack to index
}

//For patch change  
if (BTcheck2 == 1 && patchSend == currentPatch) {
  BTcheck2 = 0;
}

if (BTcheck2 == 1 && patchSend != currentPatch) {
  if (millis()-now > 100) {
    now = millis(); 
    SendBT2();
  }
}

if (BTcheck2 == 0) {
  patchSend = currentPatch;
}

//Tuner button/////////////////////////////////// 
 /* if (digitalRead(btnTuner)==HIGH) { 
      timer = millis();
      while (1) {
        if (digitalRead(btnTuner)==LOW) {
            tuner(); break;
          }
          if (millis() - timer > 10000) {digitalWrite(ledPowerGreen,HIGH); digitalWrite(ledPowerRed,HIGH); returnCheck=1;tunerStandalone();}
        }}
*/

if (digitalRead(btnTuner)==HIGH) { //if tuner button is pressed
  tuner(); //go to tuner subgroup
}
  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SendBT() //send song changes over BT
{
  if (indexBack < 1) {indexBack = 100;} //circle through songs
  if (indexBack > 100) {indexBack = 1;}
  
  Serial.print("%"); //header
  Serial.print(indexBack); //index to send
  Serial.println("&"); //end
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SendBT2() //send patch changes over BT
{
  Serial.print("@"); //header
  Serial.print(patchSend); //patch to send
  Serial.println("&"); //end
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SendMIDI(byte number) //send program change over USB
{if (number != numberOld) {
  Usb.Task();
  if( Usb.getUsbTaskState() == USB_STATE_RUNNING ) //if USB is running, continue
  { 
    byte Message[2];                 // Construct the midi message (2 bytes)
    Message[0]=0xC0;                 // 0xC0 is for Program Change (Change to MIDI channel 0)
    Message[1]=number;               // patch [0-99]
    Midi.SendData(Message);          // Send the message
    delay(10); 
  }
  numberOld=number;
} 
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void tuner() { 
  tunerCheck=0; //set check back to 0
  digitalWrite(ledTuner,HIGH); //turn tuner led on
  tuner2=2;
  Serial.print("$"); Serial.print(2); Serial.println("&");
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
     tunerSend();
     BTstillOn();
     readSerialTuner(); //read BT for message to exit tuner     
     if (digitalRead(btnTuner)==HIGH || tunerCheck==1) { //if message from BT or if button is pressed
       digitalWrite(ledTuner,LOW); //turn off tuner led
       tuner2=1;
       Serial.print("$"); Serial.print(1); Serial.println("&");
       digitalWrite(pinExpT,LOW); //simulate footswitch press
       delay(250); //delay to ensure registration of press
       digitalWrite(pinExpT,HIGH); //return pin to high for normal operation
       break; //exit while loop
       }
     delay(10); //delay for stability
   }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void readSerial()
{ 
if( Serial.available()) { //check for serial
  char ch = Serial.read(); //serial message
  if (ch >= '0' && ch <= '9') { // is this an ascii digit between 0 and 9?
    values[fieldIndex] = (values[fieldIndex] * 10) + (ch - '0'); 
  }
  
  else if (ch == ',') { // comma is our separator, so move on to the next field
    if(fieldIndex < NUMBER_OF_FIELDS-1) 
      fieldIndex++;   // increment field index
  }
  
  else { //else identify message
      if (values[0] > 0) { //BT is still on!
        BTnow = millis(); 
      }
      
      if (values[0]==1) {  //song change
        index=values[1]; //set values from message
        patchA=values[2];
        patchB=values[3];
        patchC=values[4];
        volume=values[5];
        selectedPatch=patchA; //set the selected patch to patchA
        currentPatch=1; //reset currentPatch to 1 
        digitalWrite(ledA,HIGH); //turn only led patchA on
        digitalWrite(ledB,LOW);
        digitalWrite(ledC,LOW);
      }
     
      if (values[0] == 2) { //tuner
        tuner(); //go to tuner
      }
      
      if (values[0]==3) { //patch change
        currentPatch=values[1];  //get the current patch
        volume=values[2];
        if (currentPatch == 1) {selectedPatch = patchA;} //set selected patch
        if (currentPatch == 2) {selectedPatch = patchB;} 
        if (currentPatch == 3) {selectedPatch = patchC;}
        digitalWrite(ledA,LOW); //turn leds off
        digitalWrite(ledB,LOW);
        digitalWrite(ledC,LOW);
        digitalWrite(currentPatch+1,HIGH); //turn only led of selected patch on
      } 
      for(int i=0; i <= fieldIndex; i++) {
        values[i] = 0; // set the values to zero, ready for the next message
      }
      fieldIndex = 0;  // ready to start over
    
   
  } 
  if (selectedPatch != patchOld) { //if selected patch is different from the current one, change it 
    SendMIDI(selectedPatch); 
    patchOld = selectedPatch;
  }  
 }  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void readSerialTuner() { //read serial for if tuner button in android is pressed
if (Serial.available()) {
  BTnow = millis(); //BT is still on!
  char ch = Serial.read();
  if(ch >= '0' && ch <= '9') {
    values[fieldIndex] = (values[fieldIndex] * 10) + (ch - '0'); 
  }
  else if (ch == ',') {
    if(fieldIndex < NUMBER_OF_FIELDS-1)
      fieldIndex++;
    }
  else {
    if (values[0] > 0) { //BT is still on!
      BTnow = millis(); 
    }
    if (values[0]==2) {
      tunerCheck=1;
    }  
    for (int i=0; i <= fieldIndex; i++) {
      values[i] = 0;
    }
    fieldIndex = 0;
  } 
} 
}

//////////////////////////
void BTstillOn() {
  //Serial.println(millis()-BTnow);
  if (millis() - BTnow < 1000) {
    digitalWrite(ledPowerRed,LOW);
    digitalWrite(ledPowerGreen,HIGH);
    failSafe=0;
  }
  else {
    digitalWrite(ledPowerGreen,LOW);
    digitalWrite(ledPowerRed,HIGH);
    failSafe=1;
  }
}

//////////////////////////
void setVolume() {
  analogWrite(pinExpR,map(volume,0,100,0,255));
}

//////////////////////////
void tunerSend() {
if (millis() - timer > 1000) {
  Serial.print("$");
  Serial.print(tuner2);
  Serial.println("&");
  timer = millis();
}
}



void failsafe() {
  if (failSafe == 1) {
    if (digitalRead(btnDown)==HIGH || digitalRead(btnUp)==HIGH) {
      digitalWrite(ledPowerGreen,HIGH);
      digitalWrite(ledPowerRed,HIGH);
    patchA = failsafeA;
    patchB = failsafeB;
    patchC = failsafeC;
        selectedPatch=patchA; //set the selected patch to patchA
        currentPatch=1; //reset currentPatch to 1 
        digitalWrite(ledA,HIGH); //turn only led patchA on
        digitalWrite(ledB,LOW);
        digitalWrite(ledC,LOW);
        SendMIDI(patchA);
  }
  }
}













void standalone()
{
  digitalWrite(ledPowerGreen,HIGH);
  digitalWrite(ledPowerRed,HIGH);
SendMIDI(number);
  
  
  while (1) {
    if (digitalRead(btnTuner)==LOW) {delay(100); break;}
  }
  
  
  while(1){
    if (digitalRead(btnA)==HIGH) {
      number--;
      if (number<0) {number=99;}
      SendMIDI(number);
      timer = millis();
      while (digitalRead(btnA)==HIGH) {
        if (millis() - timer > 1000) {
      while(digitalRead(btnA)==HIGH) {
        if (millis() - timer > 250) {
          number--;
           if (number<0) {number=99;}
            SendMIDI(number);
            timer = millis();
        }}}}
        delay(100);
    }
    if (digitalRead(btnB)==HIGH) {
      number++;
      if (number>99) {number=0;}
      SendMIDI(number);
      timer = millis();
      while (digitalRead(btnB)==HIGH) {
        if (millis() - timer > 1000) {
      while(digitalRead(btnB)==HIGH) {
        if (millis() - timer > 250) {
          number++;
           if (number>99) {number=0;}
            SendMIDI(number);
            timer = millis();
        }}}}
        delay(100);
    }
    if (digitalRead(btnC)==HIGH) {
      number=number-10;
      if (number<0) {number = number+100;}
      SendMIDI(number);
      timer = millis();
      while (digitalRead(btnC)==HIGH) {
        if (millis() - timer > 1000) {
      while (digitalRead(btnC)==HIGH) {
        if (millis() - timer > 250) {
          number = number-10;
          if (number<0) {number = number+100;}
          SendMIDI(number);
          timer = millis();
      }}}}
      delay(100);
    }
      
         if (digitalRead(btnDown)==HIGH) {
      number=number+10;
      if (number>99) {number = number-100;}
      SendMIDI(number);
      timer = millis();
      while (digitalRead(btnDown)==HIGH) {
        if (millis() - timer > 1000) {
      while (digitalRead(btnDown)==HIGH) {
        if (millis() - timer > 250) {
          number = number+10;
          if (number>99) {number = number-100;}
          SendMIDI(number);
          timer = millis();
      }}}}
      delay(100);
    } 
    
    if (digitalRead(btnTuner)==HIGH) { 
      timer = millis();
      while (1) {
        if (digitalRead(btnTuner)==LOW) {
            tunerStandalone(); break;
          }
          if (millis() - timer > 10000) {digitalWrite(ledPowerGreen,LOW); returnCheck=0;break;}
        }}

// if (returnCheck == 0) {break;}   
}

}

void tunerStandalone() { 

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
standalone();
}
