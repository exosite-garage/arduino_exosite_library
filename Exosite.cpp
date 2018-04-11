//*****************************************************************************
//
// exosite.cpp - Prototypes for the Exosite Cloud API
//
// Copyright (c) 2012-2017 Exosite LLC.  All rights reserved.
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

#include "Exosite.h"

/*==============================================================================
* Exosite
*
* constructor for Exosite class
*=============================================================================*/
Exosite::Exosite(Client *_client){
  client = _client;
  #if !defined(ESP8266)
  fetchNVCIK();
  #endif
}

Exosite::Exosite(const char *_cik, Client *_client){
  strncpy(cik, _cik, 41);
  client = _client;
}

Exosite::Exosite(const String _cik, Client *_client){
  _cik.toCharArray(cik, 41);
  client = _client;
}

/*==============================================================================
* begin
*
* cik must be fetched after initialization on ESP8266
*=============================================================================*/
#if defined(ESP8266)
void Exosite::begin(){
  fetchNVCIK();
}
#endif

/*==============================================================================
* setDomain
*
* set a custom domain to contact
*=============================================================================*/
void Exosite::setDomain(const char *domain, int port){
  serverName = domain;
  serverPort = port;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//Data Procedutes
/*==============================================================================
* writeRead
*
* One step read and write to Exosite using char arrays.
*=============================================================================*/
boolean Exosite::writeRead(const char* writeString, const char* readString, char** returnString){
  ret = false;
  stringPos = 0;
  DataRx= false;
  RxLoop = true;
  timeout_time = 0;
  time_now = 0;
  timeout = 3000; // 3 seconds
  varPtr = aliasList;

  Serial.print(F("Connecting to Exosite..."));

  if (!client->connected()) {
    Serial.print("No Existing Connection, Opening One...");
    client->stop();
  #ifdef SL_DRIVER_VERSION
    client->sslConnect(serverName,serverPort);
  #else /*CC3200*/
    client->connect(serverName,serverPort);
  #endif /*CC3200*/
  }

  if (client->connected()) {
    client->flush();
    Serial.println(F("Connected"));

    // Send request using Exosite basic HTTP API
    client->print(G("POST /onep:v1/stack/alias?"));
    client->print(readString);
    client->println(G(" HTTP/1.1"));
    client->print(G("Host: "));
    client->println(serverName);
    client->print(G("User-Agent: Exosite-Activator/"));
    client->print(ACTIVATOR_VERSION);
    client->print(G(" Arduino/"));
    client->println(ARDUINO);
    client->print(G("X-Exosite-CIK: "));
    client->println(cik);
    client->println(G("Accept: application/x-www-form-urlencoded; charset=utf-8"));
    client->println(G("Content-Type: application/x-www-form-urlencoded; charset=utf-8"));
    client->print(G("Content-Length: "));
    client->println(strlen(writeString)); //calculate length
    client->println();
    client->println(writeString);
    // Read from the nic or the IC buffer overflows with no warning and goes out to lunch
    timeout_time = millis()+ timeout;

    #if EXOSITEDEBUG > 1
      Serial.println(F("Sent"));
    #endif
    
    while ((timeout_time > time_now) && RxLoop && stringPos < 200) {
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
        #if EXOSITEDEBUG > 4
          Serial.println(F("No More Data"));
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
      client->stop();
    }

    if(stringPos >= 199){
      Serial.println(F("Received too Much Content, Failing"));

        #if EXOSITEDEBUG > 2
          Serial.println("Received So Far");
          Serial.println(rxdata);
          Serial.println("Also Received:");
        #endif

      while (client->available()) {
        c = client->read();

        #if EXOSITEDEBUG > 2
          Serial.write(c);
        #endif
      }

      return false;
    }
  }else{
    Serial.println(F("Error: Can't Open Connection to Exosite."));
  }

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
boolean Exosite::writeRead(const String &writeString, const String &readString, String &returnString){
  #ifdef EXOSITEDEBUGMEM
    Serial.print(getFreeMemory());
    Serial.println(F(" = Free Memory String Start"));
    Serial.println(writeString);
    Serial.println(readString);
  #endif

  char *writeCharString, *readCharString, *returnCharString;
  writeCharString = (char*)malloc(sizeof(char) * writeString.length()+1);
  readCharString = (char*)malloc(sizeof(char) * readString.length()+1);
  returnCharString = (char*)malloc(sizeof(char) * 32);

  #ifdef EXOSITEDEBUGMEM
    Serial.print(getFreeMemory());
    Serial.println(F(" = Free Memory String Chars Allocated"));
    Serial.print(F("Return Address Before: "));
    Serial.println((intptr_t)returnCharString);
  #endif

  if(writeCharString == 0 || readCharString == 0 || returnCharString == 0){
    Serial.println(F("Not Enough Ram! Failing!"));
    while(1);
  }

  writeString.toCharArray(writeCharString, writeString.length()+1);
  readString.toCharArray(readCharString, readString.length()+1);


  #ifdef EXOSITEDEBUGMEM
    Serial.print(getFreeMemory());
    Serial.println(F(" = Free Memory String Start Char Write"));
  #endif

  if(this->writeRead(writeCharString, readCharString, &returnCharString)){

    #ifdef EXOSITEDEBUGMEM
      Serial.print(getFreeMemory());
      Serial.println(F(" = Free Memory String Char Write Finished"));
      Serial.print(F("Return Address After: "));
      Serial.println((intptr_t)returnCharString);
    #endif

    returnString = String(returnCharString);


    #ifdef EXOSITEDEBUGMEM
      Serial.print(getFreeMemory());
      Serial.println(F(" = Free Memory String Chars Copied"));
    #endif

    ret = true;
  }else{
    Serial.println(F("Error Communicating with Exosite"));
    ret = false;
  }
  free(writeCharString);
  free(readCharString);
  free(returnCharString);


  #ifdef EXOSITEDEBUGMEM
    Serial.print(getFreeMemory());
    Serial.println(F(" = Free Memory String Chars Freed"));
  #endif

  return ret;
}

/*==============================================================================
* read
*
* One step read from Exosite using char arrays.
*=============================================================================*/
boolean Exosite::read(const char* readString, char** returnString){
  ret = false;
  stringPos = 0;
  DataRx= false;
  RxLoop = true;
  timeout_time = 0;
  time_now = 0;
  timeout = 3000; // 3 seconds
  varPtr = aliasList;

  Serial.print(F("Connecting to Exosite..."));

  if (!client->connected()) {
    Serial.print("No Existing Connection, Opening One...");
    client->stop();
  #ifdef SL_DRIVER_VERSION
    client->sslConnect(serverName,serverPort);
  #else /*CC3200*/
    client->connect(serverName,serverPort);
  #endif /*CC3200*/
  }

  if (client->connected()) {
    client->flush();
    Serial.println(F("Connected"));

    // Send request using Exosite basic HTTP API
    client->print(G("GET /onep:v1/stack/alias?"));
    client->print(readString);
    client->println(G(" HTTP/1.1"));
    client->print(G("Host: "));
    client->println(serverName);
    client->print(G("User-Agent: Exosite-Activator/"));
    client->print(ACTIVATOR_VERSION);
    client->print(G(" Arduino/"));
    client->println(ARDUINO);
    client->print(G("X-Exosite-CIK: "));
    client->println(cik);
    client->println(G("Accept: application/x-www-form-urlencoded; charset=utf-8"));
    client->println();
    // Read from the nic or the IC buffer overflows with no warning and goes out to lunch
    timeout_time = millis()+ timeout;

    #if EXOSITEDEBUG > 1
      Serial.println(F("Sent"));
    #endif

    while ((timeout_time > time_now) && RxLoop && stringPos < 200) {
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
        #if EXOSITEDEBUG > 4
          Serial.println(F("No More Data"));
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
          } else if(strstr(rxdata, "HTTP/1.1 204 No Content")){
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
      client->stop();
    }

    if(stringPos >= 199){
      Serial.println(F("Received too Much Content, Failing"));

        #if EXOSITEDEBUG > 2
          Serial.println("Received So Far");
          Serial.println(rxdata);
          Serial.println("Also Received:");
        #endif

      while (client->available()) {
        c = client->read();

        #if EXOSITEDEBUG > 2
          Serial.write(c);
        #endif
      }
      return false;
    }
  }else{
    Serial.println(F("Error: Can't Open Connection to Exosite."));
  }

  #ifdef EXOSITEDEBUG
    Serial.println(F("End Char Read"));
  #endif

  return ret;
}
/*==============================================================================
* read
*
* One step read from Exosite using Arduino String objects.
*=============================================================================*/
boolean Exosite::read(const String &readString, String &returnString){
  #ifdef EXOSITEDEBUGMEM
    Serial.print(getFreeMemory());
    Serial.println(F(" = Free Memory String Start"));
    Serial.println(readString);
  #endif

  char *readCharString, *returnCharString;
  readCharString = (char*)malloc(sizeof(char) * readString.length()+1);
  returnCharString = (char*)malloc(sizeof(char) * 32);

  #ifdef EXOSITEDEBUGMEM
    Serial.print(getFreeMemory());
    Serial.println(F(" = Free Memory String Chars Allocated"));
    Serial.print(F("Return Address Before: "));
    Serial.println((intptr_t)returnCharString);
  #endif

  if(readCharString == 0 || returnCharString == 0){
    Serial.println(F("Not Enough Ram! Failing!"));
    while(1);
  }

  readString.toCharArray(readCharString, readString.length()+1);


  #ifdef EXOSITEDEBUGMEM
    Serial.print(getFreeMemory());
    Serial.println(F(" = Free Memory String Start Char Write"));
  #endif

  if(this->read(readCharString, &returnCharString)){

    #ifdef EXOSITEDEBUGMEM
      Serial.print(getFreeMemory());
      Serial.println(F(" = Free Memory String Char Write Finished"));
      Serial.print(F("Return Address After: "));
      Serial.println((intptr_t)returnCharString);
    #endif

    returnString = String(returnCharString);


    #ifdef EXOSITEDEBUGMEM
      Serial.print(getFreeMemory());
      Serial.println(F(" = Free Memory String Chars Copied"));
    #endif

    ret = true;
  }else{
    Serial.println(F("Error Communicating with Exosite"));
    ret = false;
  }
  free(readCharString);
  free(returnCharString);


  #ifdef EXOSITEDEBUGMEM
    Serial.print(getFreeMemory());
    Serial.println(F(" = Free Memory String Chars Freed"));
  #endif

  return ret;
}

/*==============================================================================
* longPoll
*
* Constant read from Exosite using char arrays.
*=============================================================================*/
boolean Exosite::longPoll(const int timeoutRequest, const char* readString, char** returnString, const char*currentTimestamp){
  ret = false;
  stringPos = 0;
  DataRx= false;
  RxLoop = true;
  longPollTimeoutRequest = timeoutRequest*1000; //passes in the timeoutRequest and converts from seconds (user input) to milliseconds
  timeout_time = 0;
  time_now = 0;
  timeout = 3000; // Temporary replacement as client->available() is not suited to handle longPoll
  varPtr = aliasList;

  Serial.print(F("Connecting to Exosite..."));

  if (!client->connected()) {
    Serial.print("No Existing Connection, Opening One...");
    client->stop();
  #ifdef SL_DRIVER_VERSION
    client->sslConnect(serverName,serverPort);
  #else /*CC3200*/
    client->connect(serverName,serverPort);
  #endif /*CC3200*/
  }

  timeout_time = millis()+ timeout;

  while((timeout_time > time_now) && RxLoop){ //checks client->connected() for timeout
    time_now = millis();
    if (client->connected()) {
      client->flush();
      Serial.println(F("Connected"));
      Serial.println("Polling...");

      // Send request using Exosite basic HTTP API
      client->print(G("GET /onep:v1/stack/alias?"));
      client->print(readString);
      client->println(G(" HTTP/1.1"));
      client->print(G("Host: "));
      client->println(serverName);
      client->print(G("User-Agent: Exosite-Activator/"));
      client->print(ACTIVATOR_VERSION);
      client->print(G(" Arduino/"));
      client->println(ARDUINO);
      client->print(G("X-Exosite-CIK: "));
      client->println(cik);
      client->println(G("Accept: application/x-www-form-urlencoded; charset=utf-8"));
      client->print(G("Request-Timeout: "));
      client->println(longPollTimeoutRequest);

      if(strlen(currentTimestamp)>0){
        client->print("If-Modified-Since: ");
        client->println(currentTimestamp);
      }

      client->println();
      // Read from the nic or the IC buffer overflows with no warning and goes out to lunch
      #if EXOSITEDEBUG > 1
       Serial.println(F("Sent"));
      #endif

      while (RxLoop && stringPos < 200) {
        if (client->available()) { //During the longPoll HTTP request, this returns false unless data is updated
          if (!DataRx)
            DataRx= true;

          c = client->read();
          rxdata[stringPos] = c;

          #if EXOSITEDEBUG > 2
            Serial.print(c);
          #endif

          stringPos += 1;
        } else {
          #if EXOSITEDEBUG > 4
            Serial.println(F("No More Data"));
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
            } else if(strstr(rxdata, "HTTP/1.1 204 No Content")){
            #ifdef EXOSITEDEBUG
              Serial.println(F("HTTP Status: 204"));
            #endif
  
            ret = true;
            } else if (strstr(rxdata, "HTTP/1.1 304 Not Modified")) {
            #ifdef EXOSITEDEBUG
              Serial.println(F("HTTP Status: 304"));
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
      }
      if(stringPos >= 199){
        Serial.println(F("Received too Much Content, Failing"));

          #if EXOSITEDEBUG > 2
            Serial.println("Received So Far");
            Serial.println(rxdata);
            Serial.println("Also Received:");
          #endif

        while (client->available()) {
          c = client->read();

          #if EXOSITEDEBUG > 2
            Serial.write(c);
          #endif
        }
        return false;
      }
    }
  }
  if(timeout_time <= time_now){
    Serial.println(F("Error: Can't Open Connection to Exosite, HTTP Response Timeout"));
    client->stop();
  } 
  #ifdef EXOSITEDEBUG
    Serial.println(F("End of Polling"));
  #endif

  return ret;
}
/*==============================================================================
* longPoll
*
* Constant read from Exosite using Arduino String objects.
*=============================================================================*/
boolean Exosite::longPoll(const int timeoutRequest, const String &readString, String &returnString, const String currentTimestamp){
  #ifdef EXOSITEDEBUGMEM
    Serial.print(getFreeMemory());
    Serial.println(F(" = Free Memory String Start"));
    Serial.println(readString);
  #endif

  char *readCharString, *returnCharString, *currentTimestampChar;
  readCharString = (char*)malloc(sizeof(char) * readString.length()+1);
  returnCharString = (char*)malloc(sizeof(char) * 32);
  currentTimestampChar = (char*)malloc(sizeof(char) * currentTimestamp.length()+1);

  #ifdef EXOSITEDEBUGMEM
    Serial.print(getFreeMemory());
    Serial.println(F(" = Free Memory String Chars Allocated"));
    Serial.print(F("Return Address Before: "));
    Serial.println((intptr_t)returnCharString);
  #endif

  if(readCharString == 0 || currentTimestampChar == 0 || returnCharString == 0){
    Serial.println(F("Not Enough Ram! Failing!"));
    while(1);
  }

  readString.toCharArray(readCharString, readString.length()+1);
  currentTimestamp.toCharArray(currentTimestampChar, currentTimestamp.length()+1);


  #ifdef EXOSITEDEBUGMEM
    Serial.print(getFreeMemory());
    Serial.println(F(" = Free Memory String Start Char Write"));
  #endif

  if(this->longPoll(timeoutRequest, readCharString, &returnCharString, currentTimestampChar)){

    #ifdef EXOSITEDEBUGMEM
      Serial.print(getFreeMemory());
      Serial.println(F(" = Free Memory String Char Write Finished"));
      Serial.print(F("Return Address After: "));
      Serial.println((intptr_t)returnCharString);
    #endif

    returnString = String(returnCharString);


    #ifdef EXOSITEDEBUGMEM
      Serial.print(getFreeMemory());
      Serial.println(F(" = Free Memory String Chars Copied"));
    #endif

    ret = true;
  }else{
    Serial.println(F("Error Communicating with Exosite"));
    ret = false;
  }
  free(readCharString);
  free(returnCharString);
  free(currentTimestampChar);


  #ifdef EXOSITEDEBUGMEM
    Serial.print(getFreeMemory());
    Serial.println(F(" = Free Memory String Chars Freed"));
  #endif

  return ret;
}

/*==============================================================================
* write
*
* One step write to Exosite using char arrays.
*=============================================================================*/
boolean Exosite::write(const char* writeString){
  ret = false;
  stringPos = 0;
  DataRx= false;
  RxLoop = true;
  timeout_time = 0;
  time_now = 0;
  timeout = 3000; // 3 seconds
  varPtr = aliasList;

  Serial.print(F("Connecting to Exosite..."));

  if (!client->connected()) {
    Serial.print("No Existing Connection, Opening One...");
    client->stop();
  #ifdef SL_DRIVER_VERSION
    client->sslConnect(serverName,serverPort);
  #else /*CC3200*/
    client->connect(serverName,serverPort);
  #endif /*CC3200*/
  }

  if (client->connected()) {
    client->flush();
    Serial.println(F("Connected"));

    // Send request using Exosite basic HTTP API
    client->print(G("POST /onep:v1/stack/alias?"));
    client->println(G(" HTTP/1.1"));
    client->print(G("Host: "));
    client->println(serverName);
    client->print(G("User-Agent: Exosite-Activator/"));
    client->print(ACTIVATOR_VERSION);
    client->print(G(" Arduino/"));
    client->println(ARDUINO);
    client->print(G("X-Exosite-CIK: "));
    client->println(cik);
    client->println(G("Content-Type: application/x-www-form-urlencoded; charset=utf-8"));
    client->print(G("Content-Length: "));
    client->println(strlen(writeString)); //calculate length
    client->println();
    client->println(writeString);
    // Read from the nic or the IC buffer overflows with no warning and goes out to lunch
    timeout_time = millis()+ timeout;

    #if EXOSITEDEBUG > 1
      Serial.println(F("Sent"));
    #endif

    while ((timeout_time > time_now) && RxLoop && stringPos < 200) {
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
        #if EXOSITEDEBUG > 4
          Serial.println(F("No More Data"));
        #endif
        rxdata[stringPos] = 0;

        if (DataRx) {
          DataRx = false;
          RxLoop = false;

            #if EXOSITEDEBUG > 1
              Serial.println("HTTP Response:");
              Serial.println(rxdata);
            #endif

          if(strstr(rxdata, "HTTP/1.1 204 No Content")){
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
      client->stop();
    }

  }else{
    Serial.println(F("Error: Can't Open Connection to Exosite."));
  }

  #ifdef EXOSITEDEBUG
    Serial.println(F("End Char Write"));
  #endif

  return ret;
}
/*==============================================================================
* write
*
* One step write to Exosite using Arduino String objects.
*=============================================================================*/
boolean Exosite::write(const String &writeString){
  #ifdef EXOSITEDEBUGMEM
    Serial.print(getFreeMemory());
    Serial.println(F(" = Free Memory String Start"));
    Serial.println(writeString);
  #endif

  char *writeCharString;
  writeCharString = (char*)malloc(sizeof(char) * writeString.length()+1);

  #ifdef EXOSITEDEBUGMEM
    Serial.print(getFreeMemory());
    Serial.println(F(" = Free Memory String Chars Allocated"));
    Serial.print(F("Return Address Before: "));
  #endif

  if(writeCharString == 0){
    Serial.println(F("Not Enough Ram! Failing!"));
    while(1);
  }

  writeString.toCharArray(writeCharString, writeString.length()+1);


  #ifdef EXOSITEDEBUGMEM
    Serial.print(getFreeMemory());
    Serial.println(F(" = Free Memory String Start Char Write"));
  #endif

  if(this->write(writeCharString)){

    #ifdef EXOSITEDEBUGMEM
      Serial.print(getFreeMemory());
      Serial.println(F(" = Free Memory String Char Write Finished"));
      Serial.print(F("Return Address After: "));
    #endif



    #ifdef EXOSITEDEBUGMEM
      Serial.print(getFreeMemory());
      Serial.println(F(" = Free Memory String Chars Copied"));
    #endif

    ret = true;
  }else{
    Serial.println(F("Error Communicating with Exosite"));
    ret = false;
  }
  free(writeCharString);

  #ifdef EXOSITEDEBUGMEM
    Serial.print(getFreeMemory());
    Serial.println(F(" = Free Memory String Chars Freed"));
  #endif

  return ret;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//Provisioning Procedures
/*==============================================================================
* Provision
*
* Provisions on Exosite Platform, activate device and get cik.
*=============================================================================*/
boolean Exosite::provision(const char* vendorString, const char* modelString, const char* snString){
  ret = false;
  stringPos = 0;
  DataRx= false;
  RxLoop = true;
  timeout_time = 0;
  time_now = 0;
  timeout = 3000; // 3 seconds
  const char* vendorParameter = "vendor=";
  const char* modelParameter = "&model=";
  const char* snParameter = "&sn=";
  size_t writeStringLen = strlen(vendorString) +
                          strlen(modelString) +
                          strlen(snString) +
                          strlen(vendorParameter) +
                          strlen(modelParameter) +
                          strlen(snParameter) + 
                          1;
  char *writeString = (char*)malloc(sizeof(char) * (writeStringLen));

  // Assemble Parameter String
  varPtr = writeString;
  strcpy(varPtr, vendorParameter);
  varPtr = varPtr + strlen(vendorParameter);
  strcpy(varPtr, vendorString);
  varPtr = varPtr + strlen(vendorString);
  strcpy(varPtr, modelParameter);
  varPtr = varPtr + strlen(modelParameter);
  strcpy(varPtr, modelString);
  varPtr = varPtr + strlen(modelString);
  strcpy(varPtr, snParameter);
  varPtr = varPtr + strlen(snParameter);
  strcpy(varPtr, snString);

  Serial.print(F("Connecting to Exosite (Provision)..."));

  if (!client->connected()) {
    Serial.print("No Existing Connection, Opening One...");
    client->stop();
  #ifdef SL_DRIVER_VERSION
    client->sslConnect(serverName,serverPort);
  #else /*CC3200*/
    client->connect(serverName,serverPort);
  #endif /*CC3200*/
  }

  if (client->connected()) {
    client->flush();
    Serial.println(F("Connected"));

    // Send request using Exosite basic HTTP API
    client->println(G("POST /provision/activate HTTP/1.1"));
    client->print(G("Host: "));
    client->println(serverName);
    client->print(G("User-Agent: Exosite-Activator/"));
    client->print(ACTIVATOR_VERSION);
    client->print(G(" Arduino/"));
    client->println(ARDUINO);
    client->println(G("Content-Type: application/x-www-form-urlencoded; charset=utf-8"));
    client->print(G("Content-Length: "));
    client->println(strlen(writeString)); //calculate length
    client->println();
    client->println(writeString);
    // Read from the nic or the IC buffer overflows with no warning and goes out to lunch
    timeout_time = millis()+ timeout;

    #ifdef EXOSITEDEBUG
      Serial.print(F("Sent: "));
      Serial.println(writeString);
    #endif
    
    while ((timeout_time > time_now) && RxLoop && stringPos < 200) {
      if (client->available()) {
        if (!DataRx)
          DataRx= true;
        
        c = client->read();
        rxdata[stringPos] = c;
        
        stringPos += 1;
      } else {
        rxdata[stringPos] = 0;

        if (DataRx) {
          #ifdef EXOSITEDEBUG
            Serial.println(F("HTTP Response:"));
            Serial.println(rxdata);
          #endif

          if (strstr(rxdata, "HTTP/1.1 200 OK")) {
            Serial.println(F("Provisioned Successfully"));
            varPtr = strstr(rxdata, "\r\n\r\n") + 4;

            if(strlen(varPtr) == 40){
              strncpy(cik, varPtr, 41);
              saveNVCIK();
              ret = true;
            }else{
              Serial.print(F("CRITICAL: Got 200 Response, Wasn't a Valid CIK"));
              #if EXOSITEDEBUG > 9
              Serial.print(F("CRITICAL: Failing Hard to Preserve State"));
              while(1);
              #endif
              ret = false;
            }

          }else if(strstr(rxdata, "HTTP/1.1 404 Not Found")){
            Serial.println(F("Error: A client matching the given parameters was not found in the provisioning system. (404)"));
            ret = false;
          }else if(strstr(rxdata, "HTTP/1.1 409 Conflict")){
            Serial.println(F("Warning: The client specified is already in use. (409)"));
            ret = false;
          } else {
            Serial.println(F("Warning: Unknown Response:"));
            varPtr = strstr(rxdata, "\n");
            *varPtr = '\0';
            Serial.println(rxdata);

            ret = false;
          }

          break;
        }
      }
      time_now = millis();
    }


      if(timeout_time <= time_now){
        #ifdef EXOSITEDEBUG
          Serial.println(F("HTTP Response Timeout"));
        #endif
        client->stop();
      }


    if(stringPos >= 199){
      Serial.println(F("Received too Much Content, Failing"));

        #if EXOSITEDEBUG > 2
          Serial.println("Received So Far");
          Serial.println(rxdata);
          Serial.println("Also Received:");
        #endif

      while (client->available()) {
        c = client->read();

        #if EXOSITEDEBUG > 2
          Serial.write(c);
        #endif
      }

      return false;
    }
  }else{
    Serial.println(F("Error: Can't Open Connection to Exosite."));
  }

  free(writeString);

  #ifdef EXOSITEDEBUG
    Serial.println(F("End of Provision"));
  #endif

  return ret;
}

/*==============================================================================
* List Available Content
*
* lists the content id's of an activated product
*=============================================================================*/
boolean Exosite::listAvailableContent(const char* vendorString, const char* modelString, char** returnString){
  ret = false;
  stringPos = 0;
  DataRx= false;
  RxLoop = true;
  timeout_time = 0;
  time_now = 0;
  timeout = 3000; // 3 seconds

  Serial.print(F("Connecting to Exosite..."));

  if (!client->connected()) {
    Serial.print("No Existing Connection, Opening One...");
    client->stop();
  #ifdef SL_DRIVER_VERSION
    client->sslConnect(serverName,serverPort);
  #else /*CC3200*/
    client->connect(serverName,serverPort);
  #endif /*CC3200*/
  }

  if (client->connected()) {
    client->flush();
    Serial.println(F("Connected"));

    // Send request using Exosite basic HTTP API
    client->print(G("GET /provision/download?vendor="));
    client->print(vendorString);
    client->print(G("&model="));
    client->print(modelString);
    client->println(G(" HTTP/1.1"));
    client->print(G("Host: "));
    client->println(serverName);
    client->print(G("User-Agent: Exosite-Activator/"));
    client->print(ACTIVATOR_VERSION);
    client->print(G(" Arduino/"));
    client->println(ARDUINO);
    client->print(G("X-Exosite-CIK: "));
    client->println(cik);
    client->println();
    // Read from the nic or the IC buffer overflows with no warning and goes out to lunch
    timeout_time = millis()+ timeout;

    #if EXOSITEDEBUG > 1
      Serial.println(F("Sent"));
    #endif

    while ((timeout_time > time_now) && RxLoop && stringPos < 200) {
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
        #if EXOSITEDEBUG > 4
          Serial.println(F("No More Data"));
        #endif
        rxdata[stringPos] = 0;

        if (DataRx) {
          DataRx = false;
          RxLoop = false;

            #if EXOSITEDEBUG > 1
              Serial.println("HTTP Response:");
              Serial.println(rxdata);
            #endif

          if(strstr(rxdata, "HTTP/1.1 200 OK")){
            #ifdef EXOSITEDEBUG
              Serial.println(F("HTTP Status: 200"));
            #endif

            ret = true;
            varPtr = strstr(rxdata, "\r\n\r\n") + 4;

            *returnString = (char*) realloc(*returnString, (rxdata + stringPos + 1) - varPtr);

            if(*returnString == 0)
              break;

            strncpy(*returnString, varPtr, (rxdata + stringPos + 1) - varPtr);
          } else if(strstr(rxdata, "HTTP/1.1 204 No Content")) {
            #ifdef EXOSITEDEBUG
              Serial.println(F("HTTP Status: 204"));
            #endif

            ret = true;
          } else if(strstr(rxdata, "HTTP/1.1 304 Forbidden")) {
            #ifdef EXOSITEDEBUG
              Serial.println(F("HTTP Status: 304"));
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
      client->stop();
    }

  }else{
    Serial.println(F("Error: Can't Open Connection to Exosite."));
  }

  #ifdef EXOSITEDEBUG
    Serial.println(F("End of List Available Content"));
  #endif

  return ret;
}
/*==============================================================================
* List Available Content
*
* lists the content id's of an activated product (String parser)
*=============================================================================*/
boolean Exosite::listAvailableContent(const String &vendorString, const String &modelString, String &returnString){
  #ifdef EXOSITEDEBUGMEM
    Serial.print(getFreeMemory());
    Serial.println(F(" = Free Memory String Start"));
    Serial.println(vendorString);
    Serial.println(modelString);
  #endif

  char *vendorCharString, *modelCharString, *returnCharString;
  vendorCharString = (char*)malloc(sizeof(char) * vendorString.length()+1);
  modelCharString = (char*)malloc(sizeof(char) * modelString.length()+1);
  returnCharString = (char*)malloc(sizeof(char) * 32);

  #ifdef EXOSITEDEBUGMEM
    Serial.print(getFreeMemory());
    Serial.println(F(" = Free Memory String Chars Allocated"));
    Serial.print(F("Return Address Before: "));
    Serial.println((intptr_t)returnCharString);
  #endif

  if(vendorCharString == 0 || modelCharString == 0 || returnCharString == 0){
    Serial.println(F("Not Enough Ram! Failing!"));
    while(1);
  }

  vendorString.toCharArray(vendorCharString, vendorString.length()+1);
  modelString.toCharArray(modelCharString, modelString.length()+1);


  #ifdef EXOSITEDEBUGMEM
    Serial.print(getFreeMemory());
    Serial.println(F(" = Free Memory String Start Char Write"));
  #endif

  if(this->listAvailableContent(vendorCharString, modelCharString, &returnCharString)){

    #ifdef EXOSITEDEBUGMEM
      Serial.print(getFreeMemory());
      Serial.println(F(" = Free Memory String Char Write Finished"));
      Serial.print(F("Return Address After: "));
      Serial.println((intptr_t)returnCharString);
    #endif

    returnString = String(returnCharString);


    #ifdef EXOSITEDEBUGMEM
      Serial.print(getFreeMemory());
      Serial.println(F(" = Free Memory String Chars Copied"));
    #endif

    ret = true;
  }else{
    Serial.println(F("Error Communicating with Exosite"));
    ret = false;
  }
  free(vendorCharString);
  free(modelCharString);
  free(returnCharString);


  #ifdef EXOSITEDEBUGMEM
    Serial.print(getFreeMemory());
    Serial.println(F(" = Free Memory String Chars Freed"));
  #endif

  return ret;
}

/*==============================================================================
* Get Content Info
*
* Lists content information of specific content
*=============================================================================*/
boolean Exosite::getContentInfo(const char* vendorString, const char* modelString, const char* idString, char** returnString){
  ret = false;
  stringPos = 0;
  DataRx= false;
  RxLoop = true;
  timeout_time = 0;
  time_now = 0;
  timeout = 3000; // 3 seconds

  Serial.print(F("Connecting to Exosite..."));

  if (!client->connected()) {
    Serial.print("No Existing Connection, Opening One...");
    client->stop();
  #ifdef SL_DRIVER_VERSION
    client->sslConnect(serverName,serverPort);
  #else /*CC3200*/
    client->connect(serverName,serverPort);
  #endif /*CC3200*/
  }

  if (client->connected()) {
    client->flush();
    Serial.println(F("Connected"));

    // Send request using Exosite basic HTTP API
    client->print(G("GET /provision/download?vendor="));
    client->print(vendorString);
    client->print(G("&model="));
    client->print(modelString);
    client->print("&id=");
    client->print(idString);
    client->println(G("&info=true HTTP/1.1"));
    client->print(G("Host: "));
    client->println(serverName);
    client->print(G("User-Agent: Exosite-Activator/"));
    client->print(ACTIVATOR_VERSION);
    client->print(G(" Arduino/"));
    client->println(ARDUINO);
    client->print(G("X-Exosite-CIK: "));
    client->println(cik);
    client->println();
    // Read from the nic or the IC buffer overflows with no warning and goes out to lunch
    timeout_time = millis()+ timeout;

    #if EXOSITEDEBUG > 1
      Serial.println(F("Sent"));
    #endif

    while ((timeout_time > time_now) && RxLoop && stringPos < 200) {
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
        #if EXOSITEDEBUG > 4
          Serial.println(F("No More Data"));
        #endif
        rxdata[stringPos] = 0;

        if (DataRx) {
          DataRx = false;
          RxLoop = false;

            #if EXOSITEDEBUG > 1
              Serial.println("HTTP Response:");
              Serial.println(rxdata);
            #endif

          if(strstr(rxdata, "HTTP/1.1 200 OK")){
            #ifdef EXOSITEDEBUG
              Serial.println(F("HTTP Status: 200"));
            #endif

            ret = true;
            varPtr = strstr(rxdata, "\r\n\r\n") + 4;

            *returnString = (char*) realloc(*returnString, (rxdata + stringPos + 1) - varPtr);

            if(*returnString == 0)
              break;

            strncpy(*returnString, varPtr, (rxdata + stringPos + 1) - varPtr);
          } else if(strstr(rxdata, "HTTP/1.1 204 No Content")) {
            #ifdef EXOSITEDEBUG
              Serial.println(F("HTTP Status: 204"));
            #endif

            ret = true;
          } else if(strstr(rxdata, "HTTP/1.1 400 Bad Request")) {
            #ifdef EXOSITEDEBUG
              Serial.println(F("HTTP Status: 400"));
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
      client->stop();
    }

  }else{
    Serial.println(F("Error: Can't Open Connection to Exosite."));
  }

  #ifdef EXOSITEDEBUG
    Serial.println(F("End of List Available Content"));
  #endif

  return ret;
}
/*==============================================================================
* Get Content Info
*
* Lists content information of specific content (String parser)
*=============================================================================*/
boolean Exosite::getContentInfo(const String &vendorString, const String &modelString, const String &idString, String &returnString){
  #ifdef EXOSITEDEBUGMEM
    Serial.print(getFreeMemory());
    Serial.println(F(" = Free Memory String Start"));
    Serial.println(vendorString);
    Serial.println(modelString);
    Serial.println(idString);
  #endif

  char *vendorCharString, *modelCharString, *idCharString, *returnCharString;
  vendorCharString = (char*)malloc(sizeof(char) * vendorString.length()+1);
  modelCharString = (char*)malloc(sizeof(char) * modelString.length()+1);
  idCharString = (char*)malloc(sizeof(char) * idString.length()+1);
  returnCharString = (char*)malloc(sizeof(char) * 32);

  #ifdef EXOSITEDEBUGMEM
    Serial.print(getFreeMemory());
    Serial.println(F(" = Free Memory String Chars Allocated"));
    Serial.print(F("Return Address Before: "));
    Serial.println((intptr_t)returnCharString);
  #endif

  if(vendorCharString == 0 || modelCharString == 0 || idCharString == 0 || returnCharString == 0){
    Serial.println(F("Not Enough Ram! Failing!"));
    while(1);
  }

  vendorString.toCharArray(vendorCharString, vendorString.length()+1);
  modelString.toCharArray(modelCharString, modelString.length()+1);
  idString.toCharArray(idCharString, idString.length()+1);


  #ifdef EXOSITEDEBUGMEM
    Serial.print(getFreeMemory());
    Serial.println(F(" = Free Memory String Start Char Write"));
  #endif

  if(this->getContentInfo(vendorCharString, modelCharString, idCharString, &returnCharString)){

    #ifdef EXOSITEDEBUGMEM
      Serial.print(getFreeMemory());
      Serial.println(F(" = Free Memory String Char Write Finished"));
      Serial.print(F("Return Address After: "));
      Serial.println((intptr_t)returnCharString);
    #endif

    returnString = String(returnCharString);


    #ifdef EXOSITEDEBUGMEM
      Serial.print(getFreeMemory());
      Serial.println(F(" = Free Memory String Chars Copied"));
    #endif

    ret = true;
  }else{
    Serial.println(F("Error Communicating with Exosite"));
    ret = false;
  }
  free(vendorCharString);
  free(modelCharString);
  free(idCharString);
  free(returnCharString);


  #ifdef EXOSITEDEBUGMEM
    Serial.print(getFreeMemory());
    Serial.println(F(" = Free Memory String Chars Freed"));
  #endif

  return ret;
}

/*==============================================================================
* Download Content
*
* 
*=============================================================================*/
boolean Exosite::downloadContent(const char* vendorString, const char* modelString, const char* idString, char** returnString, const char* byteRange){
  ret = false;
  stringPos = 0;
  DataRx= false;
  RxLoop = true;
  timeout_time = 0;
  time_now = 0;
  timeout = 3000; // 3 seconds

  Serial.print(F("Connecting to Exosite..."));

  if (!client->connected()) {
    Serial.print("No Existing Connection, Opening One...");
    client->stop();
  #ifdef SL_DRIVER_VERSION
    client->sslConnect(serverName,serverPort);
  #else /*CC3200*/
    client->connect(serverName,serverPort);
  #endif /*CC3200*/
  }

  if (client->connected()) {
    client->flush();
    Serial.println(F("Connected"));

    // Send request using Exosite basic HTTP API
    client->print(G("GET /provision/download?vendor="));
    client->print(vendorString);
    client->print(G("&model="));
    client->print(modelString);
    client->print("&id=");
    client->print(idString);
    client->println(G(" HTTP/1.1"));
    client->print(G("Host: "));
    client->println(serverName);
    client->print(G("User-Agent: Exosite-Activator/"));
    client->print(ACTIVATOR_VERSION);
    client->print(G(" Arduino/"));
    client->println(ARDUINO);
    client->print(G("X-Exosite-CIK: "));
    client->println(cik);

    //byte range is optional, if 'byteRange' is not empty it will use the function
    if(strlen(byteRange)>0){
      client->print("Range: bytes=");
      client->println(byteRange);

    }

    client->println();
    // Read from the nic or the IC buffer overflows with no warning and goes out to lunch
    timeout_time = millis()+ timeout;

    #if EXOSITEDEBUG > 1
      Serial.println(F("Sent"));
    #endif

    while ((timeout_time > time_now) && RxLoop && stringPos < 200) {
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
        #if EXOSITEDEBUG > 4
          Serial.println(F("No More Data"));
        #endif
        rxdata[stringPos] = 0;

        if (DataRx) {
          DataRx = false;
          RxLoop = false;

            #if EXOSITEDEBUG > 1
              Serial.println("HTTP Response:");
              Serial.println(rxdata);
            #endif

          if(strstr(rxdata, "HTTP/1.1 200 OK")){
            #ifdef EXOSITEDEBUG
              Serial.println(F("HTTP Status: 200"));
            #endif
            ret = true;
            varPtr = strstr(rxdata, "\r\n\r\n") + 4;

            *returnString = (char*) realloc(*returnString, (rxdata + stringPos + 1) - varPtr);

            if(*returnString == 0)
              break;

            strncpy(*returnString, varPtr, (rxdata + stringPos + 1) - varPtr);
          } else if(strstr(rxdata, "HTTP/1.1 204 No Content")) {
            #ifdef EXOSITEDEBUG
              Serial.println(F("HTTP Status: 204"));
            #endif

            ret = true;
          } else if(strstr(rxdata, "HTTP/1.1 206 Partial Content")) {
            #ifdef EXOSITEDEBUG
              Serial.println(F("HTTP Status: 206"));
            #endif

            ret = true;
            varPtr = strstr(rxdata, "\r\n\r\n") + 4;

            *returnString = (char*) realloc(*returnString, (rxdata + stringPos + 1) - varPtr);

            if(*returnString == 0)
              break;

            strncpy(*returnString, varPtr, (rxdata + stringPos + 1) - varPtr);
          } else if(strstr(rxdata, "HTTP/1.1 400 Bad Request")) {
            #ifdef EXOSITEDEBUG
              Serial.println(F("HTTP Status: 400"));
            #endif

            ret = true;
          } else if(strstr(rxdata, "HTTP/1.1 403 Forbidden")) {
            #ifdef EXOSITEDEBUG
              Serial.println(F("HTTP Status: 403"));
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
      client->stop();
    }

  }else{
    Serial.println(F("Error: Can't Open Connection to Exosite."));
  }

  #ifdef EXOSITEDEBUG
    Serial.println(F("End of List Available Content"));
  #endif

  return ret;
}
/*==============================================================================
* Download Content
*
* (String parser)
*=============================================================================*/
boolean Exosite::downloadContent(const String &vendorString, const String &modelString, const String &idString, String &returnString, const String byteRange){
  #ifdef EXOSITEDEBUGMEM
    Serial.print(getFreeMemory());
    Serial.println(F(" = Free Memory String Start"));
    Serial.println(vendorString);
    Serial.println(modelString);
    Serial.println(idString);
    Serial.println(byteRange);
  #endif

  char *vendorCharString, *modelCharString, *idCharString, *byteRangeChar, *returnCharString;
  vendorCharString = (char*)malloc(sizeof(char) * vendorString.length()+1);
  modelCharString = (char*)malloc(sizeof(char) * modelString.length()+1);
  idCharString = (char*)malloc(sizeof(char) * idString.length()+1);
  byteRangeChar = (char*)malloc(sizeof(char) * byteRange.length()+1);
  returnCharString = (char*)malloc(sizeof(char) * 32);

  #ifdef EXOSITEDEBUGMEM
    Serial.print(getFreeMemory());
    Serial.println(F(" = Free Memory String Chars Allocated"));
    Serial.print(F("Return Address Before: "));
    Serial.println((intptr_t)returnCharString);
  #endif

  if(vendorCharString == 0 || modelCharString == 0 || idCharString == 0 || byteRangeChar == 0 || returnCharString == 0){
    Serial.println(F("Not Enough Ram! Failing!"));
    while(1);
  }

  vendorString.toCharArray(vendorCharString, vendorString.length()+1);
  modelString.toCharArray(modelCharString, modelString.length()+1);
  idString.toCharArray(idCharString, idString.length()+1);
  byteRange.toCharArray(byteRangeChar, byteRange.length()+1);


  #ifdef EXOSITEDEBUGMEM
    Serial.print(getFreeMemory());
    Serial.println(F(" = Free Memory String Start Char Write"));
  #endif

  if(this->downloadContent(vendorCharString, modelCharString, idCharString, &returnCharString, byteRangeChar)){

    #ifdef EXOSITEDEBUGMEM
      Serial.print(getFreeMemory());
      Serial.println(F(" = Free Memory String Char Write Finished"));
      Serial.print(F("Return Address After: "));
      Serial.println((intptr_t)returnCharString);
    #endif

    returnString = String(returnCharString);


    #ifdef EXOSITEDEBUGMEM
      Serial.print(getFreeMemory());
      Serial.println(F(" = Free Memory String Chars Copied"));
    #endif

    ret = true;
  }else{
    Serial.println(F("Error Communicating with Exosite"));
    ret = false;
  }
  free(vendorCharString);
  free(modelCharString);
  free(idCharString);
  free(byteRangeChar);
  free(returnCharString);


  #ifdef EXOSITEDEBUGMEM
    Serial.print(getFreeMemory());
    Serial.println(F(" = Free Memory String Chars Freed"));
  #endif

  return ret;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//Utility Procedures
/*==============================================================================
* time
*
* Gets the server time as a unix timestamp.
*=============================================================================*/
unsigned long Exosite::timestamp(){
  unsigned long timestamp = 0;
  stringPos = 0;
  DataRx= false;
  RxLoop = true;
  timeout_time = 0;
  time_now = 0;
  timeout = 3000; // 3 seconds

  #ifdef EXOSITEDEBUG
    Serial.print(F("Connecting to Exosite (Time)..."));
  #endif

  if (!client->connected()) {
    Serial.print("No Existing Connection, Opening One...");
    client->stop();
  #ifdef SL_DRIVER_VERSION
    client->sslConnect(serverName,serverPort);
  #else /*CC3200*/
    client->connect(serverName,serverPort);
  #endif /*CC3200*/
  }

  if (client->connected()) {
    client->flush();

  #ifdef EXOSITEDEBUG
    Serial.println(F("Connected"));
  #endif

    // Send request using Exosite basic HTTP API
    client->println(G("GET /timestamp HTTP/1.1"));
    client->print(G("Host: "));
    client->println(serverName);
    client->print(G("User-Agent: Exosite-Activator/"));
    client->print(ACTIVATOR_VERSION);
    client->print(G(" Arduino/"));
    client->println(ARDUINO);
    client->println();

    // Read from the nic or the IC buffer overflows with no warning and goes out to lunch
    timeout_time = millis()+ timeout;

    #ifdef EXOSITEDEBUG
      Serial.print(F("Sent"));
    #endif
    
    while ((timeout_time > time_now) && RxLoop && stringPos < 200) {
      if (client->available()) {
        if (!DataRx)
          DataRx= true;
        
        c = client->read();
        rxdata[stringPos] = c;
        
        stringPos += 1;

        #if EXOSITEDEBUG > 2
          Serial.write(c);
        #endif
      } else {
        rxdata[stringPos] = 0;

        if (DataRx) {
          #ifdef EXOSITEDEBUG
            Serial.println(F("HTTP Response:"));
            Serial.println(rxdata);
          #endif

          if (strstr(rxdata, "HTTP/1.1 200 OK")) {
            varPtr = strstr(rxdata, "\r\n\r\n") + 4;

            timestamp = strtoul(varPtr, NULL, 10);
          }else {
            #ifdef EXOSITEDEBUG
              Serial.println(F("Warning Unknown Response: "));

              varPtr = strstr(rxdata, "\n");
              *varPtr = '\0';

              Serial.println(rxdata);
            #endif
          }

          break;
        }
      }
      time_now = millis();
    }

    
      if(timeout_time <= time_now){
        #ifdef EXOSITEDEBUG
          Serial.println(F("HTTP Response Timeout"));
        #endif
        client->stop();
      }
   

    if(stringPos >= 199){
      Serial.println(F("Received too Much Content, Failing"));

        #if EXOSITEDEBUG > 2
          Serial.println("Received So Far");
          Serial.println(rxdata);
          Serial.println("Also Received:");
        #endif

      while (client->available()) {
        c = client->read();

        #if EXOSITEDEBUG > 2
          Serial.write(c);
        #endif
      }

      return false;
    }
  }else{
    Serial.println(F("Error: Can't Open Connection to Exosite."));
  }

  #ifdef EXOSITEDEBUG
    Serial.println(F("End of Time"));
  #endif

  return timestamp;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/*==============================================================================
* saveNVCIK
*
* Write the CIK to EEPROM
*=============================================================================*/
boolean Exosite::saveNVCIK(){
  for(int i = 0; i < 40; i++){
    EEPROM.write(CIK_EEPROM_ADDRESS + i, cik[i]);
  }

  return true;
}



/*==============================================================================
* fetchNVCIK
*
* Fetch the CIK from EEPROM
*=============================================================================*/
boolean Exosite::fetchNVCIK(){
  char tempBuf[41];

  for(int i = 0; i < 40; i++){
    tempBuf[i] = EEPROM.read(CIK_EEPROM_ADDRESS + i);
  }
  tempBuf[40] = 0;

  if(strlen(tempBuf) == 40){
    strcpy(cik, tempBuf);
    return true;
  }else{
    return false;
  }
}


//This was previously used to check CIK values, however in recent updates, CIKs are
//no longer restricted to hex characters. This function is being left in because it 
//might have to be used in the future
boolean Exosite::isHex(char *str, int len){
  for(int i = 0; i < len; i++){
    if(!((str[i] >= '0' && str[i] <= '9') ||
         (str[i] >= 'A' && str[i] <= 'F') ||
         (str[i] >= 'a' && str[i] <= 'f'))){
      return false;
    }
  }

  return true;
}

