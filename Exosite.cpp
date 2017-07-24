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
Exosite::Exosite(Client *_client)
{
  client = _client;
#if !defined(ESP8266) && !defined(SL_DRIVER_VERSION)
  fetchNVCIK();
#endif
}

Exosite::Exosite(const char *_cik, Client *_client)
{
  strncpy(cik, _cik, 41);
  client = _client;
}

Exosite::Exosite(const String _cik, Client *_client)
{
  _cik.toCharArray(cik, 41);
  client = _client;
}

/*==============================================================================
* begin
*
* cik must be fetched after initialization on ESP8266 or CC3200
*=============================================================================*/
#if defined(ESP8266) || defined(SL_DRIVER_VERSION)
void Exosite::begin(){
  //fetchNVCIK();
}
#endif

/*==============================================================================
* setDomain
*
* set a custom domain to contact
*=============================================================================*/
void Exosite::setDomain(const char *domain)
{
  serverName = domain;
}

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
  timeout = 20000; // 20 seconds
  varPtr = aliasList;

  Serial.print(F("Connecting to Exosite..."));

  if (!client->connected()) {
    Serial.print("No Existing Connection, Opening One...");
    client->stop();

    client->connect(serverName,80);

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
* provision
*
* Provision on Exosite Platform, activate device and get cik.
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

    client->connect(serverName,80);

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
            Serial.println(F("Activated Successfully"));
            varPtr = strstr(rxdata, "\r\n\r\n") + 4;

            if(strlen(varPtr) == 40){
              strncpy(cik, varPtr, 41);
              //saveNVCIK();
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
Serial.println("BRUH");
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
* saveNVCIK
*
* Write the CIK to EEPROM
*=============================================================================*/
                                                                                                /*
#ifdef SL_DRIVER_VERSION
#define CIK_LENGTH 40
#define CIK_FILENAME "exosite_cik.txt"

boolean Exosite::saveNVCIK()
{
    int iRetVal;
    long lFileHandle;
    unsigned long ulToken;

    //
    // open the cik file for writing
    //
    iRetVal = sl_FsOpen((unsigned char *) CIK_FILENAME,
                        FS_MODE_OPEN_CREATE(CIK_LENGTH, _FS_FILE_OPEN_FLAG_COMMIT|_FS_FILE_PUBLIC_WRITE|_FS_FILE_PUBLIC_READ),
                        &ulToken,
                        &lFileHandle);
    if(iRetVal < 0)
    {
        iRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
        return false;
    }

    //
    // write the cik to file
    //
    iRetVal = sl_FsWrite(lFileHandle,
                         (unsigned int)0,
                         (unsigned char *)cik,
                         CIK_LENGTH);
    if (iRetVal < 0)
    {
        iRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
        return false;
    }

    //
    // close the cik file
    //
    iRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
    return true;
}

#else
boolean Exosite::saveNVCIK(){
  for(int i = 0; i < 40; i++){
    EEPROM.write(CIK_EEPROM_ADDRESS + i, cik[i]);
  }

  return true;
}
#endif /*SL_DRIVER_VERSION*/


/*==============================================================================
* fetchNVCIK
*
* Fetch the CIK from EEPROM
*=============================================================================*/
                                                                                                /*

#ifdef SL_DRIVER_VERSION
boolean Exosite::fetchNVCIK()
{
    unsigned long ulToken;
    long lFileHandle;
    long lRetVal = -1;
    char read_buffer[41];

    //
    // open a the cik file for reading
    //
    lRetVal = sl_FsOpen((unsigned char *) CIK_FILENAME,
                        FS_MODE_OPEN_READ,
                        &ulToken,
                        &lFileHandle);
    if(lRetVal < 0)
    {
        lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
        return false;
    }

    //
    // read the cik from file
    //
    lRetVal = sl_FsRead(lFileHandle,
                (unsigned int)0,
                (unsigned char *) read_buffer,
                 CIK_LENGTH);
    if ((lRetVal < 0) || (lRetVal != CIK_LENGTH))
    {
        lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
        return false;
    }

    //
    // close the cik file
    //
    lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
    if (SL_RET_CODE_OK != lRetVal)
    {
        return false;
    }

    read_buffer[40] = 0;
    strcpy(cik, read_buffer);
    return true;
}
#else
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
#endif /*SL_DRIVER_VERSION*/

/*==============================================================================
* time
*
* Gets the server time as a unix timestamp.
*=============================================================================*/
unsigned long Exosite::time(){
  unsigned long timestamp = 0;
  stringPos = 0;
  DataRx= false;
  RxLoop = true;
  timeout_time = 0;
  time_now = 0;
  timeout = 3000; // 3 seconds

  Serial.print(F("Connecting to Exosite (Time)..."));

  if (!client->connected()) {
    Serial.print("No Existing Connection, Opening One...");
    client->stop();
#ifdef SL_DRIVER_VERSION
    client->sslConnect(serverName,443);
#else /*CC3200*/
    client->connect(serverName,80);
#endif /*CC3200*/
  }

  if (client->connected()) {
    client->flush();
    Serial.println(F("Connected"));

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
            Serial.print(F("Current Time is: "));
            Serial.println(varPtr);

            timestamp = strtoul(varPtr, NULL, 10);
          }else {
            Serial.println(F("Warning: Unknown Response:"));
            varPtr = strstr(rxdata, "\n");
            *varPtr = '\0';
            Serial.println(rxdata);
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

/*
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
/*

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
