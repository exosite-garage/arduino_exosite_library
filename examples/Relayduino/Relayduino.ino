//*****************************************************************************
//
// relayduino.ino - Controling the Relayduino from the Cloud with Exosite
//
// Copyright (c) 2012 Exosite LLC.  All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:

//  * Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright 
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of Exosite LLC nor the names of its contributors may
//    be used to endorse or promote products derived from this software 
//    without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
//
//*****************************************************************************

/******************************* USER SETTINGS *******************************/
// Fill in your MAC here! (e.g. {0x90, 0xA2, 0xDA, 0x00, 0x22, 0x33}) 
byte macData[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; 

//Fill in your CIK here! (https://portals.exosite.com -> Add Device)
char cikData[] = "0000000000000000000000000000000000000000";  

// Set the maximum interval we should read and write to Exosite.
unsigned long loop_interval_ms = 10000; // milliseconds

/*****************************************************************************/

#include <SPI.h>
#include <Ethernet.h>
#include <Exosite.h>

// Pin Definitions
#define ANIN1 6   // Analog 1 is connected to Arduino Analog In 6
#define ANIN2 7   // Analog 2 is connected to Arduino Analog In 7
#define ANIN3 0   // Analog 3 is connected to Arduino Analog In 0
const int Analogs[3] = {ANIN1, ANIN2, ANIN3};
const char* Analog_Aliases[3] = {"ANIN1", "ANIN2", "ANIN3"};

#define REL1 2  // Relay 1 is connected to Arduino Digital 2
#define REL2 3  // Relay 2 is connected to Arduino Digital 3 PWM
#define REL3 4  // Relay 3 is connected to Arduino Digital 4
#define REL4 5  // Relay 4 is connected to Arduino Digital 5 PWM
#define REL5 6  // Relay 5 is connected to Arduino Digital 6 PWM
#define REL6 7  // Relay 6 is connected to Arduino Digital 7
#define REL7 8  // Relay 7 is connected to Arduino Digital 8
#define REL8 9  // Relay 8 is connected to Arduino Digital 9 PWM
const int Relays[8] = {REL1, REL2, REL3, REL4, REL5, REL6, REL7, REL8};
//const int PWMAble[4] = {2, 4, 5, 8};
const char* Relay_Aliases[8] = {"REL1", "REL2", "REL3", "REL4", "REL5", "REL6", "REL7", "REL8"};

#define OI1 15 // Opto-Isolated Input 1 is connected to Arduino Analog 1 which is Digital 15
#define OI2 16 // Opto-Isolated Input 2 is connected to Arduino Analog 2 which is Digital 16
#define OI3 17 // Opto-Isolated Input 3 is connected to Arduino Analog 3 which is Digital 17
#define OI4 18 // Opto-Isolated Input 4 is connected to Arduino Analog 4 which is Digital 18
const int Optos[4] = {OI1, OI2, OI3, OI4};
const char* Opto_Aliases[4] = {"OI1", "OI2", "OI3", "OI4"};

//global variables    

class EthernetClient client;
Exosite exosite(cikData, &client);

unsigned long last_millis = 0;

const char* readString = "REL1&REL2&REL3&REL4&REL5&REL6&REL7&REL8";
String writeString;
String returnString;
String tempString;
int index = 0;
int lastIndex = -1;
int pinNumber = 0;

/*==============================================================================
 * setup
 *
 * Arduino setup function.
 *=============================================================================*/
void setup()
{
  Serial.begin(115200);
  Serial.println(F("Boot"));

  Ethernet.begin(macData);
  delay(1000);
  
  for(int i = 0; i < 8; i++){
    pinMode(Relays[i], OUTPUT);
  }
}

/*==============================================================================
 * loop 
 *
 * Arduino loop function.
 *=============================================================================*/
void loop()
{
  Serial.println(F("------Loop Start------"));
  
  writeString = "";
  returnString = "";
  index = 0;
  lastIndex = -1;
  
  // Setup Values to Write
  for(int i = 0; i < 4; i++){
    writeString += String(Opto_Aliases[i]) + "=" + digitalRead(Optos[i]) + "&";
  }
  for(int i = 0; i < 3; i++){
    writeString += String(Analog_Aliases[i]) + "=" + analogRead(Analogs[i]) + "&";
  }
  
  Serial.print(F("Connecting to Exosite..."));

  if(exosite.writeRead(writeString, readString, returnString)){
    Serial.println(F("Success"));
    for(;;){
      index = returnString.indexOf("=", lastIndex+1);
      if(index != 0){
        tempString = returnString.substring(lastIndex+1, index);
        
        pinNumber = getRelayPin(tempString);
        if(pinNumber == 0){ //Not a Relay Pin Name
          Serial.print(F("Unknown Alias:"));
          Serial.println(tempString);
          continue;
        }
          
        lastIndex = returnString.indexOf("&", index+1);
        
        if(lastIndex != -1){
          tempString = returnString.substring(index+1, lastIndex);
        }else{
          tempString = returnString.substring(index+1);
        }
        
        if(tempString == "1"){
          digitalWrite(pinNumber, 1);
        }else if(tempString == "0"){
          digitalWrite(pinNumber, 0);
        }else{
          Serial.print(F("Unknown Setting: "));
          Serial.println(tempString);
        }
   
        if(lastIndex == -1)
          break;     
        
      }else{
        Serial.println(F("No Index"));
        break;
      }
    }
  }else{
    Serial.println(F("Error Communicating with Exosite"));
  }
  
  Serial.print("Next Loop to Start In: ");
  Serial.print((millis() - last_millis + loop_interval_ms)/1000);
  Serial.println(" Seconds");

  while(millis() < last_millis + loop_interval_ms){
    if(millis() < last_millis)
      last_millis = millis(); // millis() overflowed and last_millis didn't.
      
    //Serial.println(millis());
  }
  
  last_millis += loop_interval_ms; // This is an unsigned long, it should overflow just the same as millis().
}

int getRelayPin(String name){
  for(int i = 0; i < 8; i++){
    if(name == Relay_Aliases[i])
      return Relays[i];
  }
  
  return 0;
}
