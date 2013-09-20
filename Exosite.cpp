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
#include <Ethernet.h>
#include <EthernetClient.h>

#define serverName "m2.exosite.com"

/*==============================================================================
* Exosite
*
* constructor for Exosite class
*=============================================================================*/
Exosite::Exosite(String _cik, Client *_client)
{
  cik = _cik;
  client = _client;
}

/*==============================================================================
* sendToCloud
*
* send data to cloud
*=============================================================================*/
int Exosite::sendToCloud(String res, int value){
  int readCount = 0;
  int writeCount = 1;
  char* readAliases[1];
  char* readValues[1];
  char* writeAliases[1];
  char* writeValues[1];
  writeAliases[0] = (char*)malloc(sizeof(char) * 32);
  writeValues[0] = (char*)malloc(sizeof(char) * 32);
  if(writeAliases[0] == 0 || writeValues[0] == 0){
    if(writeAliases[0] != 0){
      free(writeAliases[0]);
    }
    Serial.println(F("Not enough memory!"));
    return 0;
  }

  res.toCharArray(writeAliases[0], 32);
  itoa(value, writeValues[0], 10);

  if(this->readWrite(writeAliases, writeValues, writeCount, readAliases, readValues, readCount)){
    free(writeAliases[0]);
    free(writeValues[0]);
    return 1;
  }else{
    Serial.println(F("Error Communicating with Exosite (Write)"));
  }
  free(writeAliases[0]);
  free(writeValues[0]);
  return 0;
}

/*==============================================================================
* readFromCloud
*
* read data from cloud
*=============================================================================*/
int Exosite::readFromCloud(String res ,String* pResult){
  int readCount = 1;
  int writeCount = 0;
  char* readAliases[1];
  char* readValues[1];
  char* writeAliases[1];
  char* writeValues[1];
  readAliases[0] = (char*)malloc(sizeof(char) * 32);
  readValues[0] = (char*)malloc(sizeof(char) * 32);
  if(readAliases[0] == 0 || readValues[0] == 0){
    if(readAliases[0] != 0){
      free(readAliases);
    }
    Serial.println(F("Not enough memory!"));
    return 0;
  }

  res.toCharArray(readAliases[0], 32);

  if(this->readWrite(writeAliases, writeValues, writeCount, readAliases, readValues, readCount)){
    *pResult = String(readValues[0]);
    free(readAliases[0]);
    free(readValues[0]);
    return 1;
  }else{
    Serial.println(F("Error Communicating with Exosite (Read)"));
  }
  free(readAliases[0]);
  free(readValues[0]);
  return 0;
}

/*==============================================================================
* readWrite 
*
* One step Read and Write to Cloud
*=============================================================================*/
boolean Exosite::readWrite(char* writeAliases[], char* writeValues[], int& writeCount, char* readAliases[], char* readValues[], int& readCount){
  ret = false;
  stringPos = 0;
  DataRx= false;
  RxLoop = true;
  timeout_time = 0;
  time_now = 0;
  timeout = 3000; // 3 seconds
  aliasList[0] = '\0';
  dataList[0] = '\0';
  varPtr = aliasList;
  
  for(int i = 0; i < readCount; i++){
    strcat(aliasList, readAliases[i]);
    if(i != readCount-1){
      strcat(aliasList, "&");
    }
  }

  readCount = 0;
  
  for(int i = 0; i < writeCount; i++){
    strcat(dataList, writeAliases[i]);
    strcat(dataList, "=");
    strcat(dataList, writeValues[i]);
    if(i != writeCount-1){
      strcat(dataList, "&");
    }
  }

  if (client->connect(serverName,80)) {
      if (client->connected()) {
      // Send request using Exosite basic HTTP API
      client->print(F("POST /onep:v1/stack/alias?"));
      client->print(aliasList);
      client->println(F(" HTTP/1.1"));
      client->println(F("Host: m2.exosite.com"));
      client->print(F("X-Exosite-CIK: ")); 
      client->println(cik);
      client->println(F("Accept: application/x-www-form-urlencoded; charset=utf-8"));
      client->println(F("Content-Type: application/x-www-form-urlencoded; charset=utf-8"));
      client->print(F("Content-Length: "));
      client->println(strlen(dataList)); //calculate length
      client->println();
      client->println((char*)dataList);
      // Read from the nic or the IC buffer overflows with no warning and goes out to lunch
      timeout_time = millis()+ timeout;
      
      while ((timeout_time > time_now) && RxLoop) {
        if (client->available()) {
          if (!DataRx)
            DataRx= true;
          
          c = client->read();
          rxdata[stringPos] = c;
          
          stringPos += 1;
        } else {
          rxdata[stringPos] = 0;

          if (DataRx) {
            DataRx = false;
            RxLoop = false;
            size_t length = 0;
            size_t subStringLength = 0;
            int i = 0;
            if (strstr(rxdata, "HTTP/1.1 200 OK")) {
              ret = true;
              varPtr = strstr(rxdata, "\r\n\r\n") + 4;
              varPtr2 = strstr( varPtr, "=" );

              while( varPtr > 0 && varPtr2 > 0){
                strncpy(readAliases[i], varPtr, varPtr2 - varPtr);
                readAliases[i][varPtr2 - varPtr] = '\0';

                varPtr = varPtr2 + 1;
                varPtr2 = strstr( varPtr, "&" );

                if(varPtr2 == 0){ // Last Item It Seems, Go to End of String
                  varPtr2 =  varPtr + strlen(varPtr); // For some reason
                }
                
                strncpy(readValues[i], varPtr, varPtr2 - varPtr);
                readValues[i][varPtr2 - varPtr] = '\0';

                varPtr = varPtr2 + 1;
                varPtr2 = strstr( varPtr, "=" );
                i++;
              }
              readCount = i;
            }else if(strstr(rxdata, "HTTP/1.1 204 No Content")){
              ret = true;
            } else {
              varPtr = strstr(rxdata, "\n");
              *varPtr = '\0';
              Serial.println(rxdata);
            }  
          }
        }
        time_now = millis();
      }
      client->stop();
    }
  }

  return ret;
}
