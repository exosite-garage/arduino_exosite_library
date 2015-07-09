//*****************************************************************************
//
// ESP8266OccupancySensor - An example of how to use the Exosite library
//                          on the Sparkfun ESP8266 Thing with a PIR motion
//                          sensor to detect the occupancy status of a room
//
// Requires the ESP8266 Arduino Library
//
// You MUST comment out line 503 of Exosite.cpp in the Exosite library
// because the strtoul method is not compatible with the ESP8266 Library
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
#include <ESP8266WiFi.h>
#include <Exosite.h>
#define MOTION_PIN 2
#define REPORT_TIMEOUT 15000 //milliseconds period for reporting to Exosite.com

/*==============================================================================
* Configuration Variables
*
* Change these variables to your own settings.
*=============================================================================*/
String cikData = "0000000000000000000000000000000000000000";  // <-- Fill in your CIK here! (https://portals.exosite.com -> Add Device)
const char* ssid     = "your-ssid";
const char* password = "your-password";
WiFiClient client;

// Use these variables to customize what datasources are read and written to.
String readString = "occupancyint";
String writeString = "occupancyint=";
String returnString;

/*==============================================================================
* End of Configuration Variables
*=============================================================================*/
Exosite exosite(cikData, &client);
int motionState = 0;
int movementCounter = 0; // Use to count number of times movement is detected in the intervals

/*==============================================================================
* setup
*
* Arduino setup function.
*=============================================================================*/
void setup(){  
  Serial.begin(115200);
  delay(10);
  pinMode(MOTION_PIN, INPUT);
  
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

/*==============================================================================
* loop 
*
* Arduino loop function.
*=============================================================================*/
void loop(){
  static unsigned long sendPrevTime = 0;
  // read the state of the motion sensor value:
  motionState = digitalRead(MOTION_PIN);
  // check if the motion sensor is activated
  // if it is, the motionState is LOW (with 10K resistor):
  if (motionState == LOW) {
    movementCounter++;
    Serial.println("Motion Detected.");
    delay(2000);
  }
  //Send value to Exosite every REPORT_TIMEOUT milliseconds
  if (millis() - sendPrevTime > REPORT_TIMEOUT) {
    //Only send "true" if motion detected certain number of times over last interval
    exosite.writeRead(writeString+String(movementCounter), readString, returnString);
    Serial.println(returnString);
    movementCounter = 0;
    sendPrevTime = millis();
  }
  // Use some datarule Lua Script of your choice to set the threshold for 
  // the number of times motion detected results in an occupied room
}
