//*****************************************************************************
//
// ESP8266OccupancySensorNoSleepProvision - An example of how to use the Exosite 
//                                          library on the Sparkfun ESP8266 Thing 
//                                          with a PIR motion sensor to detect the 
//                                          occupancy status of a room.
//
// Requires the ESP8266 Arduino Library
//
// You MUST use the staging version of the ESP8266 library (or the stable version
// if newer than Aug 10, 2015) so that method strtoul is compatible
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
#include <OneWire.h>
#include <DallasTemperature.h>
const int MOTION_PIN = 13;
const int MOTION_POWER_PIN = 4;
const int ONEWIRE = 12;
const int LED_PIN = 5;

/*==============================================================================
* Configuration Variables
*
* Change these variables to your own settings.
*=============================================================================*/
const char ssid[]     = "SSID HERE";
const char password[] = "PASSWORD HERE";
char macString[18];  // Used to store a formatted version of the MAC Address
byte macData[WL_MAC_ADDR_LENGTH];

const int REPORT_TIMEOUT = 30000; //milliseconds period for reporting to Exosite.com

// Use these variables to customize what datasources are read and written to.
String readString = "occupancyint";
String writeString = "occupancyint=";
String returnString;
String writeString5 = "temp=";
String readString5 = "temp";

/*==============================================================================
* End of Configuration Variables
*=============================================================================*/
WiFiClient client;
Exosite exosite(&client);
// Set up which Arduino pin will be used for the 1-wire interface to the sensor
// Be sure to use OneWire library from Arduino Library Manager, not from the included
// library in the ESP8266 Board package
OneWire oneWire(ONEWIRE);
DallasTemperature DS18B20(&oneWire);
int motionState = 0;
int movementCounter; // Use to count number of times movement is detected in the intervals
float tempF, tempC;
unsigned long checkPrevTime = 0;

/*==============================================================================
* setup
*
* Arduino setup function.
*=============================================================================*/
void setup(){  
  initHardware();
  delay(100);
  connectWifi();
  movementCounter = 0;

  exosite.begin();

  ESPprovision();

  Serial.println("Calibrating...");
  delay(30000); // Calibration time
  Serial.println("Ready to detect motion.");
  delay(50);
}

/*==============================================================================
* loop 
*
* Arduino loop function.
*=============================================================================*/
void loop()
{
  checkPrevTime = millis();
  while(millis() - checkPrevTime < REPORT_TIMEOUT){
    // Read the state of the motion sensor value
    motionState = digitalRead(MOTION_PIN);
    // Check if the motion sensor is activated
    // If it is, the motionState is LOW (with 10K resistor)
    if (motionState == LOW) {
      movementCounter++;
      Serial.println("Motion Detected.");
      delay(2000);
    }
    else delay(100);
  }

  // Read temperature sensor
  DS18B20.requestTemperatures(); // Send the command to get temperatures
  tempC = DS18B20.getTempCByIndex(0);
  Serial.println("Temperature in degrees C is " + String(tempC));
  tempF = DallasTemperature::toFahrenheit(tempC);
  Serial.println("Temperature in degrees F is " + String(tempF));
  
  delay(100);
  reportToExosite();
}

/*==============================================================================
* initHardware 
*
* Use in setup to initialize pins and Serial port 
*=============================================================================*/
void initHardware(){
  Serial.begin(112500);
  delay(10);
  pinMode(MOTION_PIN, INPUT);
  pinMode(MOTION_POWER_PIN, OUTPUT);
  digitalWrite(MOTION_POWER_PIN, HIGH);
  DS18B20.begin();
  EEPROM.begin(40);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  delay(1000);
}

/*==============================================================================
* connectWifi 
*
* Use in setup to connect to local Wifi network
*=============================================================================*/
void connectWifi(){
  WiFi.mode(WIFI_STA); // Need for deepSleep
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);

  byte ledStatus = LOW;
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    // Blink the LED
    digitalWrite(LED_PIN, ledStatus); // Write LED high/low
    ledStatus = (ledStatus == HIGH) ? LOW : HIGH;
    delay(100);
  }
  digitalWrite(LED_PIN, HIGH);

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  delay(100);
  
  // Create MAC Address String in the format FF:FF:FF:FF:FF:FF
  WiFi.macAddress(macData);
  snprintf(macString, 18, "%02X:%02X:%02X:%02X:%02X:%02X",
           macData[5], macData[4], macData[3], macData[2], macData[1], macData[0]);
  // Print Some Useful Info
  Serial.print(F("MAC Address: "));
  Serial.println(macString);
}

/*==============================================================================
* ESPprovision
*
* Activate the ESP8266 to specified vendor and model and get a CIK
*=============================================================================*/
void ESPprovision(){
  byte ledStatus = HIGH;
  while(client.connected()==0){
    if(exosite.provision("exosite", "arduinoyun-generic", macString)) EEPROM.commit();
    digitalWrite(LED_PIN, ledStatus); // Write LED high/low
    ledStatus = (ledStatus == HIGH) ? LOW : HIGH;
    delay(500);
  }
  digitalWrite(LED_PIN, HIGH);
  delay(100);
}

/*==============================================================================
* reportToExosite 
*
* Write number of motions detected in period as well as awake status to dataports
*=============================================================================*/
void reportToExosite(){
  while(!exosite.writeRead(writeString+String(movementCounter)+"&"+writeString5+String(tempF), readString + "&" + readString5, returnString)) {
    Serial.println("Retrying connection");
    delay(1000);
  }

  movementCounter = 0;
  delay(1000);
}

