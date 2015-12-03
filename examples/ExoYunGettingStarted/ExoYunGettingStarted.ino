//*****************************************************************************
//
// ExoYunGettingStarted 
//   This example is can be used with the "Arduino Yun Compatible"
//   device available in portals.exosite.com.  It uses provisioning
//   to activate with Exosite with it's MAC Address instead of using a hard-coded CIK.
//   By default it does two things, one is reads the A0 analog value and sends
//   this to a dataport called A0.  It also makes a read request for a dataport 
//   called D13 and sets the Digital pin 13 to 1 or 0 based on that value, which  
//   controls the LED on the YUN board.  
//   
//   Developers can feel free to create new dataports to read and write to on the 
//   Exosite platform for their device's client and also then add the code below to 
//   interact with those dataports.
//
//   Tested with Arduino Yun
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
#include <BridgeClient.h>
#include <Exosite.h>
#include <Process.h>
//#include <Console.h>

/*==============================================================================
* Configuration Variables
*
* Change these variables to your own settings.
*=============================================================================*/
// Use these variables to customize what datasources are read and written to.
const String readString = "d13&msg";

// Number of Errors before we try a reprovision.
const unsigned char reprovisionAfter = 3;
/*==============================================================================
* End of Configuration Variables
*=============================================================================*/

class BridgeClient client;
Exosite exosite(&client);

unsigned char errorCount = reprovisionAfter;  // Force Provision On First Loop
char macString[18];  // Used to store a formatted version of the MAC Address

String tempString;
int index = 0;
int lastIndex = -1;

int analog0Pin = A0;    // select the input pin for the potentiometer
int ledPin = 13;      // select the pin for the LED


/*==============================================================================
* setup
*
* Arduino setup function.
*=============================================================================*/
void setup() {

  Serial.begin(115200);
  Serial.println(F("Boot"));
  delay(1000);

  
  Serial.println("\r\n\r\nStarting Yun WiFi...");
  pinMode(ledPin, OUTPUT);
  digitalWrite(13, LOW);
  Bridge.begin();
  digitalWrite(13, HIGH);  // Led on pin 13 turns on when the bridge is ready
  
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
void loop() {
  String writeString = "";
  String returnString = "";
  index = 0;
  lastIndex = -1;

  
  // Check if we should reprovision.
  if (errorCount >= reprovisionAfter) {
    Serial.print('Attemp to reprovision using identifier ');
    Serial.println(macString);
    if (exosite.provision("exosite", "arduinoyun-generic", macString)) {
      errorCount = 0;
    }
  }
  
  String uptime_str = String(millis()/1000);
  writeString += "uptime="+ uptime_str;

  //get average of Analog 0 pin for a number of readings
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
        
        //Handle response for pin D13 to control LED
        if (alias == "d13"){
          if(tempString == "1"){
            digitalWrite(ledPin, 1);
            Serial.println("turn on LED");
          }else if(tempString == "0"){
            digitalWrite(ledPin, 0);
            Serial.println("turn off LED");
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
    Serial.println("Error");
    errorCount++;
  }

  delay(2000);
}

void getYunMAC(char* MACptr, byte bufSize) {
  Process p;    // Create a process and call it "p"
  p.runShellCommand("/sbin/ifconfig -a | /bin/grep HWaddr |/bin/grep wlan0 | /usr/bin/awk '{print $5}'");

  for(int i = 0; i < (bufSize-1) && p.available() > 0; i++) {
    MACptr[i] = p.read();
    MACptr[i+1] = 0;
  }
}
