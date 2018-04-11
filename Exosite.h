//*****************************************************************************
//
// exosite.h - Prototypes for the Exosite Cloud API
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

#ifndef Exosite_h
#define Exosite_h

#define ACTIVATOR_VERSION   "2.5.3"

// Select a Debug Level: 
//#define EXOSITEDEBUG 1
//#define EXOSITEDEBUG 2
//#define EXOSITEDEBUG 3

// Enable or Disable Direct from ROM Sending
// Disabled on ESP because of performance issue and lack of need.
#if defined(ESP8266) or defined(NO_FLASH_NET_STRINGS)
    #define G(x) x
#else
    #define G(x) F(x)
#endif

// Enable Memory Debugging: (requires MemoryFree library)
//#define EXOSITEDEBUGMEM

#ifndef CIK_EEPROM_ADDRESS
  #define CIK_EEPROM_ADDRESS 0 //Takes Addresses 0 - 39 (dec)
#endif

// Do not Edit Past This Point

#include <SPI.h>
#include <Client.h>
#include <EEPROM.h>


#ifdef EXOSITEDEBUGMEM
#include <MemoryFree.h>
#endif

class Exosite
{
  private:
    class Client* client;
    char cik[41];
    const char *serverName = "m2.exosite.io";
    int serverPort = 80;
    char rxdata[200];
    char aliasList[50];
    char* varPtr;
    char* varPtr2;
    boolean ret;
    int stringPos;
    boolean DataRx;
    boolean RxLoop;
    char c;
    unsigned long longPollTimeoutRequest;
    unsigned long timeout_time;
    unsigned long time_now;
    unsigned long timeout;

    // Helpers
    boolean isHex(char *str, int len);


  public:
    // Constructor
    Exosite(Client *_client);
    Exosite(const char *_cik, Client *_client);
    Exosite(const String _cik, Client *_client);

    #if defined(ESP8266)
    void begin();
    #endif

    // Current Methods
  #ifdef SL_DRIVER_VERSION
    void setDomain(const char *domain, int port=443);
  #else /*CC3200*/
    void setDomain(const char *domain, int port=80);
  #endif /*CC3200*/
    

    boolean writeRead(const char* writeString,const char* readString, char** returnString);
    boolean writeRead(const String &writeString, const String &readString, String &returnString);

    boolean read(const char* readString, char** returnString);
    boolean read(const String &readString, String &returnString);

    boolean longPoll(const int timeoutRequest, const char* readString, char** returnString, const char* currentTimestamp);
    boolean longPoll(const int timeoutRequest, const String &readString, String &returnString, const String = "");

    boolean write(const char* writeString);
    boolean write(const String &writeString);

    boolean provision(const char* vendorString, const char* modelString, const char* snString);

    boolean listAvailableContent(const char* vendorString, const char* modelString, char** returnString);
    boolean listAvailableContent(const String &vendorString, const String &modelString, String &returnString);

    boolean getContentInfo(const char* vendorString, const char* modelString, const char* idString, char** returnString);
    boolean getContentInfo(const String &vendorString, const String &modelString, const String &idString, String &returnString);

    boolean downloadContent(const char* vendorString, const char* modelString, const char* idString, char** returnString, const char* byteRange);
    boolean downloadContent(const String &vendorString, const String &modelString, const String &idString, String &returnString,  const String = "");

    boolean saveNVCIK();
    boolean fetchNVCIK();

    unsigned long timestamp();
};

#endif