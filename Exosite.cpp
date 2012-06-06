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
Exosite::Exosite(EthernetClass *eth, byte* _mac, String _cik)
{  
  ethernet = eth;
  mac = _mac;
  cik = _cik;
}

/*==============================================================================
* init 
*
* initialization function for Exosite class. 
*=============================================================================*/
void Exosite::init(void)
{
  ethernet->begin(mac);
  delay(500);
  this->client = new EthernetClient();
}

/*==============================================================================
* sendToCloud 
*
* send data to cloud 
*=============================================================================*/
int Exosite::sendToCloud(String res, int value) 
{
 
  ret = 0;
  stringPos = 0;
  DataRx = false;
  RxLoop = true;
  timeout_time = 0;
  time_now = 0;
  timeout = 3000; // 3 seconds
  myDataString = ""; //allocate for actual data sent

  if (client->connect(serverName,80)) {
    if (client->connected()) {
      myDataString += res;
      myDataString += "="; //put into resource
      myDataString += value; //just send the value
      // Send request using Exosite basic HTTP API
      client->println("POST /api:v1/stack/alias HTTP/1.1");
      client->println("Host: m2.exosite.com");
      client->print("X-Exosite-CIK: "); 
      client->println(cik);
      client->println("Content-Type: application/x-www-form-urlencoded; charset=utf-8");
      client->println("Accept: application/xhtml+xml");
      client->print("Content-Length: ");
      client->println(myDataString.length()); //calculate length
      client->println();
      client->println(myDataString);

      // Read from the nic
      //
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
            DataRx= false;
            RxLoop = false;

            ret=1;
          }
        }//else
        time_now = millis();
      }// while ((timeout_time > time_now) && RxLoop) {

      client->stop();
    }
  }// if (client->connect(serverName,80)) {
  return ret;
}

/*==============================================================================
* readFromCloud 
*
* read data from cloud 
*=============================================================================*/
int Exosite::readFromCloud(String res ,String* pResult) 
{
 
  ret = 0;
  stringPos = 0;
  DataRx= false;
  RxLoop = true;
  timeout_time = 0;
  time_now = 0;
  timeout = 3000; // 3 seconds
  myDataString = ""; //allocate for actual data sent

  if (client->connect(serverName,80)) {
    if (client->connected()) {
      // Send request using Exosite basic HTTP API
      client->print("GET /api:v1/stack/alias?");
      client->print(res);
      client->println(" HTTP/1.1");
      client->println("Host: m2.exosite.com");
      client->print("X-Exosite-CIK: "); 
      client->println(cik);
      client->println("Accept: application/x-www-form-urlencoded; charset=utf-8");
      client->println();
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
            String rxstrg = String(rxdata);
            int length = 0;
            int rxresultpos = 0;
            int subStringLength = 0;
            if (rxstrg.startsWith("HTTP/1.1 200 OK")) {
              length = rxstrg.length();
              rxresultpos=rxstrg.indexOf('=');
              subStringLength = length - rxresultpos;
              *pResult= String(rxstrg.substring(rxresultpos+1));
            } else {
              rxresultpos=rxstrg.indexOf('\n');
              subStringLength = rxresultpos;
              *pResult= String(rxstrg.substring(0,rxresultpos));
            }  
            ret=1;
          }
        }
        time_now = millis();
      }
      client->stop();
    }
  }
  return ret;
}
