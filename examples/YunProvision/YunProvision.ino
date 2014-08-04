//*****************************************************************************
//
// YunProvision - A simple example showing how to use the Exosite library
//                on the Arduino Yún with provisioning.
//
// Copyright (c) 2013 Exosite LLC.  All rights reserved.
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

#include <EEPROM.h>
#include <SPI.h>
#include <Bridge.h>
#include <YunClient.h>
#include <Process.h>
#include <Exosite.h>

/*==============================================================================
* Configuration Variables
*
* Change these variables to your own settings.
*=============================================================================*/
// Use these variables to customize what datasources are read and written to.
String readString = "command&uptime";
String writeString = "uptime=";
String returnString;

// Number of Errors before we try a reprovision.
const unsigned char reprovisionAfter = 3;

/*==============================================================================
* End of Configuration Variables
*=============================================================================*/
class YunClient client;
Exosite exosite(&client);

unsigned char errorCount = reprovisionAfter;  // Force Provision On First Loop
char macString[18];  // Used to store a formatted version of the MAC Address

/*==============================================================================
* setup
*
* Arduino setup function.
*=============================================================================*/
void setup(){  
  Bridge.begin();
  Serial.begin(115200);
  Serial.println("Boot");
  
  getYunMAC(macString, 18);
  
  // Print Some Useful Info
  Serial.print(F("MAC Address: "));
  Serial.println(macString);
}

/*==============================================================================
* loop 
*
* Arduino loop function.
*=============================================================================*/
void loop(){
  // Check if we should reprovision.
  if(errorCount >= reprovisionAfter){
    if(exosite.provision("exosite", "ard-generic", macString)){
      errorCount = 0;
    }
  }
  
  //Write to "uptime" and read from "uptime" and "command" datasources.
  if ( exosite.writeRead(writeString+String(millis()), readString, returnString)){
    Serial.println("OK");
    Serial.println(returnString);
    errorCount = 0;
  }else{
    Serial.println("Error");
    errorCount++;
  }

  delay(5000);
}

void getYunMAC(char* MACptr, byte bufSize) {
  Process p;		// Create a process and call it "p"
  p.runShellCommand("/sbin/ifconfig -a | /bin/grep HWaddr |/bin/grep wlan0 | /usr/bin/awk '{print $5}'");

  for(int i = 0; i < (bufSize-1) && p.available() > 0; i++) {
    MACptr[i] = p.read();
    MACptr[i+1] = 0;
  }
}
