//*****************************************************************************
//
// exosite.cpp - Prototypes for the Exosite Cloud API
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
#include "Exosite.h"

#define serverName          "m2.exosite.com"
#define ACTIVATOR_VERSION   F("2.1")

// Select a Debug Level: 
//#define EXOSITEDEBUG 1
//#define EXOSITEDEBUG 2
//#define EXOSITEDEBUG 3

/*==============================================================================
* Exosite
*
* constructor for Exosite class
*=============================================================================*/
Exosite::Exosite(char *_cik, Client *_client)
{
  strncpy(cik, _cik, 41);
  client = _client;
}

Exosite::Exosite(String _cik, Client *_client)
{
  _cik.toCharArray(cik, 41);
  client = _client;
}

/*==============================================================================
* writeRead
*
* One step read and write to Exosite using char arrays.
*=============================================================================*/
boolean Exosite::writeRead(char* writeString, char* readString, char** returnString){
  ret = false;
  stringPos = 0;
  DataRx= false;
  RxLoop = true;
  timeout_time = 0;
  time_now = 0;
  timeout = 3000; // 3 seconds
  varPtr = aliasList;

  Serial.print(F("Connecting to Exosite..."));

  if (client->connect(serverName,80)) {
    client->flush();
    Serial.println(F("Connected"));

    // Send request using Exosite basic HTTP API
    client->print(F("POST /onep:v1/stack/alias?"));
    client->print(readString);
    client->println(F(" HTTP/1.1"));
    client->println(F("Host: m2.exosite.com"));
    client->print(F("User-Agent: Exosite-Activator/"));
    client->print(ACTIVATOR_VERSION);
    client->print(F(" Arduino/"));
    client->println(ARDUINO);
    client->print(F("X-Exosite-CIK: ")); 
    client->println(cik);
    client->println(F("Accept: application/x-www-form-urlencoded; charset=utf-8"));
    client->println(F("Content-Type: application/x-www-form-urlencoded; charset=utf-8"));
    client->print(F("Content-Length: "));
    client->println(strlen(writeString)); //calculate length
    client->println();
    client->println(writeString);
    // Read from the nic or the IC buffer overflows with no warning and goes out to lunch
    timeout_time = millis()+ timeout;

    #if EXOSITEDEBUG > 1
      Serial.println(F("Sent"));
    #endif
    
    while ((timeout_time > time_now) && RxLoop) {
      if (client->available()) {
        if (!DataRx)
          DataRx= true;
        
        c = client->read();
        rxdata[stringPos] = c;

        #if EXOSITEDEBUG > 2
          Serial.print(c);
        #endif
        
        stringPos += 1;
      } else {
        #if EXOSITEDEBUG > 1
          Serial.println("No More Data");
        #endif
        rxdata[stringPos] = 0;

        if (DataRx) {
          DataRx = false;
          RxLoop = false;

            #if EXOSITEDEBUG > 1
              Serial.println("HTTP Response:");
              Serial.println(rxdata);
            #endif
  
          if (strstr(rxdata, "HTTP/1.1 200 OK")) {
            #ifdef EXOSITEDEBUG
              Serial.println(F("HTTP Status: 200"));
            #endif
  
            ret = true;
            varPtr = strstr(rxdata, "\r\n\r\n") + 4;

            *returnString = (char*) realloc(*returnString, (rxdata + stringPos + 1) - varPtr);

            if(*returnString == 0)
              break;

            strncpy(*returnString, varPtr, (rxdata + stringPos + 1) - varPtr);
          }else if(strstr(rxdata, "HTTP/1.1 204 No Content")){
            #ifdef EXOSITEDEBUG
              Serial.println(F("HTTP Status: 204"));
            #endif
  
            ret = true;
          } else {
            #ifdef EXOSITEDEBUG
              Serial.println(F("Warning Unknown Response: "));

              varPtr = strstr(rxdata, "\n");
              *varPtr = '\0';

              Serial.println(rxdata);
            #endif
          }  
        }
      }
      time_now = millis();
    }

    if(timeout_time <= time_now){
      Serial.println(F("Error: HTTP Response Timeout"));
    }
  }else{
    Serial.println("Error: Can't Open Connection to Exosite.");
  }

  client->stop();

  #ifdef EXOSITEDEBUG
    Serial.println(F("End Char ReadWrite"));
  #endif

  return ret;
}

/*==============================================================================
* writeRead
*
* One step read and write to Exosite using Arduino String objects.
*=============================================================================*/
boolean Exosite::writeRead(String writeString, String readString, String &returnString){
  char *writeCharString, *readCharString, *returnCharString;
  writeCharString = (char*)malloc(sizeof(char) * writeString.length()+1);
  readCharString = (char*)malloc(sizeof(char) * readString.length()+1);
  returnCharString = (char*)malloc(sizeof(char) * 32);

  if(writeCharString == 0 || readCharString == 0 || returnCharString == 0){
    Serial.println(F("Not Enough Ram! Failing!"));
    while(1);
  }

  writeString.toCharArray(writeCharString, writeString.length()+1);
  readString.toCharArray(readCharString, readString.length()+1);

  if(this->writeRead(writeCharString, readCharString, &returnCharString)){
    returnString = returnCharString;
    ret = true;
  }else{
    Serial.println(F("Error Communicating with Exosite"));
    ret = false;
  }
  free(writeCharString);
  free(readCharString);
  free(returnCharString);

  return ret;
}

/*==============================================================================
* DEPRECIATED METHODS
*=============================================================================*/

/*==============================================================================
* sendToCloud
*
* send data to cloud
*=============================================================================*/
int Exosite::sendToCloud(String res, int value){
  String readString = "";
  String returnString = "";
  String writeString;
  writeString = res + "=" + value;

  if(this->writeRead(writeString, readString, returnString)){
    return 1;
  }else{
    Serial.println(F("Error Communicating with Exosite"));
    return 0;
  }
}


/*==============================================================================
* readFromCloud
*
* read data from cloud
*=============================================================================*/
int Exosite::readFromCloud(String readString ,String* returnString){
  String writeString = "";

  if(this->writeRead(writeString, readString, *returnString)){
    return 1;
  }else{
    Serial.println(F("Error Communicating with Exosite"));
    return 0;
  }
}
