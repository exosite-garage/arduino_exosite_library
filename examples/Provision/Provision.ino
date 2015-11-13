//*****************************************************************************
//
// Provision
// This example shows how to use Exosite's provision API to activate a device
// as a type of product (vendor, model, unique serial number).  This is used 
// to activate and get a CIK instead of using a generic hard-coded CIK.
// http://docs.exosite.com/provision/device/                   
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
#include <YunClient.h>
#include <Process.h>
#include <Exosite.h>

/*==============================================================================
* Configuration Variables
*
* Change these variables to your own settings.
*=============================================================================*/



/*==============================================================================
* End of Configuration Variables
*=============================================================================*/
class YunClient client;
Exosite exosite(&client);

char macString[18];  // Used to store a formatted version of the MAC Address
char ipString[18];  // Used to store a formatted version of the MAC Address

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
  getYunIP(ipString, 18);
  
  // Print Some Useful Info
  Serial.print(F("MAC Address: "));
  Serial.println(macString);
  Serial.print(F("IP Address: "));
  Serial.println(ipString);
}

/*==============================================================================
* loop 
*
* Arduino loop function.
*=============================================================================*/
void loop(){
  
  /* 
     Note: Devices must use a vendor id, model id, and a unique pre-whitelisted serial 
     number which is pre-determined by the vendor creating the model. 
     This example assumes the Yun is being used which has a MAC Address included.
     Other Arduino based platforms, like using the Ethernet Shield, do not have a pre-programmed 
     MAC Address.  Some example uses of Exosite may provide a unique identifier to program
     into your code / sketch. 
     For this example, the Yun's MAC address will be used for the unique serial number
     http://docs.exosite.com/provision/device/
     */
  
  Serial.print(F("Attemp to reprovision using identifier: "));
  Serial.println(macString);
  
  // Check if we should reprovision.
  if(exosite.provision("exosite", "arduinoyun-generic", macString)){
    Serial.println("Activation success!");
  }else{
    Serial.println("Activation attempt unsuccessful");
  }
  
  /* 
     Note: Once a device activates successfully, all other attemps to activate will 
     fail, returning a 409 response.  If a Device is re-enabled, than the activation 
     call will work the next time and get the new CIK.
  */
  delay(5000);
}

void getYunMAC(char* MACptr, byte bufSize) {
  Process p;    // Create a process and call it "p"
  p.runShellCommand("/sbin/ifconfig -a | /bin/grep HWaddr |/bin/grep wlan0 | /usr/bin/awk '{print $5}'");

  for(int i = 0; i < (bufSize-1) && p.available() > 0; i++) {
    MACptr[i] = p.read();
    MACptr[i+1] = 0;
  }
}


void getYunIP(char* IPptr, byte bufSize) {
  Process d;    // Create a process and call it "d"
  d.runShellCommand("/sbin/ifconfig wlan0 | /bin/grep 'inet addr:' | cut -d: -f2 | /usr/bin/awk '{ print $1}'  ");

  for(int i = 0; i < (bufSize-1) && d.available() > 0; i++) {
    IPptr[i] = d.read();
    IPptr[i+1] = 0;
  }
}