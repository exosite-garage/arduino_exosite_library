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

#include <SPI.h>
#include <Ethernet.h>
#include <Exosite.h>

//global variables
byte macData[] = { "PUTYOURMACHERE"  }; // <-- Fill in your MAC here! (e.g. {0x90, 0xA2, 0xDA, 0x00, 0x22, 0x33}) 

String cikData = "PUTYOURCIKHERE";      // <-- Fill in your CIK here! (https://portals.exosite.com -> Add Device)

Exosite exosite(&Ethernet, macData, cikData);

/*==============================================================================
* setup
*
* Arduino setup function.
*=============================================================================*/
void setup()
{
  exosite.init();
}

/*==============================================================================
* loop 
*
* Arduino loop function.
*=============================================================================*/
void loop()
{

  String retVal;

  //Read the alias (resource name) "onoff"
  if ( exosite.readFromCloud("onoff", &retVal)) 
  {
    //Send a value + 100 to Exosite, use the alias (resource name) "1"
    exosite.sendToCloud("1", retVal.toInt() + 100);
  }

  delay(3000);
}
