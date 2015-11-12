//*****************************************************************************
//
// YunTimestamp - This is the simplest example, all it does is read the
//                current time as a unix timestamp from
//                m2.exosite.com/timestamp. This example is useful for
//                debugging connectivity issues.
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

#include <EEPROM.h>
#include <SPI.h>
#include <Bridge.h>
#include <Process.h>
#include <YunClient.h>
#include <Exosite.h>

/*==============================================================================
* No Configuration Variables
*=============================================================================*/

unsigned long time;
char macString[18];  // Used to store a formatted version of the MAC Address

class YunClient client;
Exosite exosite(&client);


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

  Serial.print(F("Unique Identifier: "));
  Serial.println(macString);
  
  Serial.print(F("Exo Arduino Lib Ver: "));
  Serial.println(ACTIVATOR_VERSION);
}

/*==============================================================================
* loop
*
* Arduino loop function.
*=============================================================================*/
void loop(){
  //Write to "uptime" and read from "uptime" and "command" datasources.
  Serial.println("---- Check Time ----");
  if ((time = exosite.time()) != 0){
    Serial.println("OK");
    Serial.print("Got Timestamp: ");
    Serial.println(time);
  }else{
    Serial.println("Error, Couldn't Get Timestamp");
  }

  delay(5000);
}


/*==============================================================================
* getYunMac
*
* Send bash command to Linux side to read WiFi MAC address.
*=============================================================================*/
void getYunMAC(char* MACptr, byte bufSize) {
  Process p;    // Create a process and call it "p"
  p.runShellCommand("/sbin/ifconfig -a | /bin/grep HWaddr |/bin/grep wlan0 | /usr/bin/awk '{print $5}'");

  for(int i = 0; i < (bufSize-1) && p.available() > 0; i++) {
    MACptr[i] = p.read();
    MACptr[i+1] = 0;
  }
}
