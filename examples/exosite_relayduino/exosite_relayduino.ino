//*****************************************************************************
//
// cloud_read_write.ino - Simple read/write sample for the Exosite Cloud API
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
unsigned long loop_interval_ms = 60000; // milliseconds

/*****************************************************************************/

#include <SPI.h>
#include <Ethernet.h>
#include <Exosite.h>

// Pin Definitions
#define ANIN1 6   // Analog 1 is connected to Arduino Analog In 6
#define ANIN2 7   // Analog 2 is connected to Arduino Analog In 7
#define ANIN3 0   // Analog 3 is connected to Arduino Analog In 0
int Analogs[3] = {ANIN1, ANIN2, ANIN3};

#define REL1 2  // Relay 1 is connected to Arduino Digital 2
#define REL2 3  // Relay 2 is connected to Arduino Digital 3 PWM
#define REL3 4  // Relay 3 is connected to Arduino Digital 4
#define REL4 5  // Relay 4 is connected to Arduino Digital 5 PWM
#define REL5 6  // Relay 5 is connected to Arduino Digital 6 PWM
#define REL6 7  // Relay 6 is connected to Arduino Digital 7
#define REL7 8  // Relay 7 is connected to Arduino Digital 8
#define REL8 9  // Relay 8 is connected to Arduino Digital 9 PWM
int Relays[8] = {REL1, REL2, REL3, REL4, REL5, REL6, REL7, REL8};
int PWMAble[4] = {2, 4, 5, 8};

#define OI1 15 // Opto-Isolated Input 1 is connected to Arduino Analog 1 which is Digital 15
#define OI2 16 // Opto-Isolated Input 2 is connected to Arduino Analog 2 which is Digital 16
#define OI3 17 // Opto-Isolated Input 3 is connected to Arduino Analog 3 which is Digital 17
#define OI4 18 // Opto-Isolated Input 4 is connected to Arduino Analog 4 which is Digital 18
int Optos[4] = {OI1, OI2, OI3, OI4};

//global variables    

class EthernetClient client;
Exosite exosite(cikData, &client);

int readValueCount;
int writeValueCount;

unsigned long last_millis = 0;

char* readAliases[8];
char* readValues[8];
char* writeAliases[8];
char* writeValues[8];

char* varPtr = NULL;

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

  pinMode( 2, OUTPUT);
  pinMode( 3, OUTPUT);
  pinMode( 4, OUTPUT);
  pinMode( 5, OUTPUT);
  pinMode( 6, OUTPUT);
  pinMode( 7, OUTPUT);
  pinMode( 8, OUTPUT);
  pinMode( 9, OUTPUT);


  for(int i = 0; i < 8; i++){
    readAliases[i] = (char*)malloc(sizeof(char) * 5);
    readValues[i] = (char*)malloc(sizeof(char) * 2);
    writeAliases[i] = (char*)malloc(sizeof(char) * 6);
    writeValues[i] = (char*)malloc(sizeof(char) * 5);
    if(readAliases[i] == 0 || readValues[i] == 0 || writeAliases[i] == 0 || writeValues[i] == 0){
      Serial.println(F("Not enough memory!"));
      while(1);
    }
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

  // Prep Read Aliases
  for(int i = 0; i < 8; i++){
    sprintf(readAliases[i], "REL%d", i+1);
  }

  readValueCount = 8;
  writeValueCount = 7;
  
  // Read Values to Write
  for(int i = 0; i < 4; i++){
    sprintf(writeAliases[i], "OI%d", i+1);
    sprintf(writeValues[i], "%d", digitalRead(Optos[i]));
  }
  for(int i = 4; i < 7; i++){
    sprintf(writeAliases[i], "ANIN%d", i+1-4);
    sprintf(writeValues[i], "%d", analogRead(Analogs[i-4]));
  }

  if(exosite.readWrite(writeAliases, writeValues, writeValueCount, readAliases, readValues, readValueCount)){
    for(int i = 0; i < readValueCount; i++){
      varPtr = strstr(readAliases[i], "REL");
      if(varPtr >= 0){
        digitalWrite(atoi(varPtr+3)+1, atoi(readValues[i]));
      }else{
        Serial.println(F("Not an \"REL*\" Alias"));
      }
      Serial.print(readAliases[i]);
      Serial.print(F(": "));
      Serial.println(readValues[i]);
    } 
  }else{
    Serial.println(F("Error Communicating with Exosite"));
  }

  while(millis() < last_millis + loop_interval_ms){
    if(millis() < last_millis)
      last_millis = millis(); // millis() overflowed and last_millis didn't, oops.
  }
  
  last_millis += loop_interval_ms; // This is an unsigned long, it should overflow just the same as millis().
}
