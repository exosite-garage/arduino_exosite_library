//*****************************************************************************
//
// ExoEthernetGettingStarted - This example is can be used with the "Arduino 
//   Compatible Device" device available in portals.exosite.com.  It uses 
//   provisioning to activate with Exosite instead of using a hard-coded CIK.
//   By default it does two things, one is reads the A0 analog   value and sends
//   this to a dataport called A0.  It also makes a read request for a dataport 
//   called D2 and sets the Digital pin 2 to 1 or 0 based on that value.
//   
//   Developers can feel free to create new dataports to read and write to on the 
//   Exosite platform for their device's client and also then add the code below to 
//   interact with those dataports.
//
//   Tested with Arduino UNO SMD R2 and Arduino Ethernet Shield
//
// Copyright (c) 2015 Exosite LLC.  All rights reserved.
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

#include <SPI.h>
#include <EEPROM.h>
#include <Ethernet.h>
#include <Exosite.h>

/*==============================================================================
* Configuration Variables
*
* Change these variables to your own settings.
*=============================================================================*/
byte macData[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 };        // <-- Fill in your Ethernet shield's MAC address here.

// Enter your Unique Identifier code here
char unique_id[11] = "0024A3D07D"; // <-- Generated from Adding your device in portals.exosite.com

// Use these variables to customize what datasources are read and written to.
const String readString = "d2";

// Number of Errors before we try a reprovision.
const unsigned char reprovisionAfter = 3;
/*==============================================================================
* End of Configuration Variables
*=============================================================================*/

unsigned char errorCount = reprovisionAfter;  // Force Provision On First Loop
char macString[18];  // Used to store a formatted version of the MAC Address

class EthernetClient client;
Exosite exosite(&client);

String tempString;
int index = 0;
int lastIndex = -1;


/*==============================================================================
* setup
*
* Arduino setup function.
*=============================================================================*/
void setup() {
  Serial.begin(115200);
  Serial.println(F("Boot"));

  pinMode(2, OUTPUT); //assume using D2 as output to control remotely
  
  // Create MAC Address String in the format FF:FF:FF:FF:FF:FF
  snprintf(macString, 18, "%02X:%02X:%02X:%02X:%02X:%02X",
           macData[0], macData[1], macData[2], macData[3], macData[4], macData[5]);

  Serial.println(F("setting up Ethernet Connection..."));
  
  // Setup Ethernet Connection
  if (Ethernet.begin(macData) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    for (;;)
      ;
  }
  

  // Print Some Useful Info
  Serial.print(F("MAC Address: "));
  Serial.println(macString);

  Serial.print(F("IP Address: "));
  Serial.println(Ethernet.localIP());

  Serial.print(F("Unique Identifier: "));
  Serial.println(unique_id);
  
  Serial.print(F("Exo Arduino Lib Ver: "));
  Serial.println(ACTIVATOR_VERSION);
}

/*==============================================================================
* loop
*
* Arduino loop function.
*=============================================================================*/
void loop() {
  String writeString = "";
  String returnString = "";
  index = 0;
  lastIndex = -1;

  
  // Check if we should reprovision.
  if (errorCount >= reprovisionAfter) {
    if (exosite.provision("exosite", "ard-generic", unique_id)) {
      errorCount = 0;
    }
  }
    String uptime_str = String(millis()/1000);
  writeString += "uptime="+ uptime_str;

  //GET ANALOG 0 VALUE
  //get average of a number of readings
  int avgValue=0;
  int readings = 0;
  unsigned long avgTot = 0;
  int avgCnt = 0;

  while(readings<10)
  {
    avgTot += analogRead(A0);
    avgCnt += 1;
    avgValue = avgTot/avgCnt;
    readings++;
  }
  String analog0_str = String(avgValue);
  writeString += "&a0="+ analog0_str;

  
  //Make Write and Read request to Exosite Platform
  Serial.println("---- Do Read and Write ----");
  if (exosite.writeRead(writeString, readString, returnString)) {
    Serial.println("OK");
    Serial.print("Returned: ");
    Serial.println(returnString);
    Serial.println("Parse out dataport alias values");
    errorCount = 0;
    for(;;){
      index = returnString.indexOf("=", lastIndex+1);
      if(index != 0){
        String alias = "";
        tempString = returnString.substring(lastIndex+1, index);
        Serial.print(F("Alias: "));
        Serial.println(tempString);
        lastIndex = returnString.indexOf("&", index+1);
        alias = tempString;
        if(lastIndex != -1){
          tempString = returnString.substring(index+1, lastIndex);
        }else{
          tempString = returnString.substring(index+1);
        }
        
        
        if (alias == "d2"){
          if(tempString == "1"){
            digitalWrite(2, 1);
            Serial.println("set Digital2 to 1");
          }else if(tempString == "0"){
            digitalWrite(2, 0);
            Serial.println("set Digital2 to 0");
          }else{
            Serial.print(F("Unknown Setting: "));
            Serial.println(tempString);
          }
        } else if (alias == "msg"){
          Serial.print("Message: ");
          Serial.println(tempString);
        } else {
          Serial.println("Unknown Alias Dataport");
        }
   
        if(lastIndex == -1)
          break;     
        
      }else{
        Serial.println(F("No Index"));
        break;
      }
    }
    
  } else {
    Serial.println("No Connection");
    errorCount++;
  }

  delay(2000);
}

