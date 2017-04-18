//*****************************************************************************
//
// EXOSITE MURANO EXAMPLE: WIFI SMART LIGHT BULB
//
// Requires the ESP8266 Arduino Library
//
// Copyright (c) 2016 Exosite LLC.  All rights reserved.
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
#include <Exosite-dev.h>


#define DHTPIN 2     // what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
#define LED_PIN 5 //LED PIN


/*==============================================================================
* WIFI Configuration Variables - Change these variables to your own settings.
*=============================================================================*/
const char ssid[]     = "salamander77";
const char password[] = "bem44ELWpj";

/*==============================================================================
* EXOSITE MURANO Configuration Variables - Change these variables to your own settings.
*=============================================================================*/
#define productId "a2mrh23zgo3qsemi"
//#define productId "re28thgcnogh9f6r"

const int REPORT_INTERVAL = 1000; //milliseconds period for reporting to Exosite.com
const int SENSOR_INTERVAL = 250;

// Use these variables to customize what dataports are read and written to.
String readString = "state";
String returnString;

int randomNumber = 0;
/*==============================================================================
* End of Configuration Variables
*=============================================================================*/

char macString[18];  // Used to store a formatted version of the MAC Address
byte macData[WL_MAC_ADDR_LENGTH];
long unsigned int prevSendTime = 0;
long unsigned int prevSensorTime = 0;
int exosite_comm_errors = 0;
int comm_errors = 0;

WiFiClient client;
Exosite exosite(&client);

/*==============================================================================
* setup
*
* Arduino setup function.
*=============================================================================*/
void setup() {
  Serial.begin(112500);
  EEPROM.begin(40);
  exosite.begin();
  exosite.setDomain(productId".m2.exosite.com");

  //SET UP IO PINS
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.println("");
  Serial.println("setup: Exosite Murano Platform Example App - Smart Lighbulb");

  connectWifi();

  //Try to activate in case device identifier has CIK in platform or a new Product ID
  Serial.println("setup: Check Exosite Platform Provisioning");
  Serial.print("setup: Murano Product ID: ");
  Serial.print(productId);
  Serial.print(F(", Device Identifier (MAC Address): "));
  Serial.println(macString);

  for (int c=0; c<8; c++) {
     digitalWrite(LED_PIN, LOW);
     delay(100);
     digitalWrite(LED_PIN, HIGH);
     delay(100);
  }
  if (exosite.provision(productId, productId, macString)) {
        Serial.println("setup: Provision Succeeded");
        EEPROM.commit();
  } else {
        Serial.println("setup: Provision Check Done");
  }

  // Setup Sensor Interface

}
/*==============================================================================
* loop
*
* Arduino loop function.
*=============================================================================*/
void loop()
{
  
  int8_t index = 0;
  int8_t lastIndex = -1;
  float h,t;

  if (prevSendTime + REPORT_INTERVAL < millis() || millis() < prevSendTime || isnan(h) || isnan(t) ) {
    int uptime = millis()/1000;
    
    Serial.println("==========================");
    
    if (exosite.longPoll(readString, returnString)) {
      //Serial.println("Succeeded");
      exosite_comm_errors = 0;
      comm_errors = 0;
      prevSendTime = millis();
      //Handle Response Message
      for(;;) {
        index = returnString.indexOf("=", lastIndex+1);
        if(index != 0) {
          String alias = "";
          String tempString = returnString.substring(lastIndex+1, index);
          Serial.print(F("exo: Dataport - "));
          Serial.print(tempString);
          lastIndex = returnString.indexOf("&", index+1);
          alias = tempString;
          if(lastIndex != -1){
            tempString = returnString.substring(index+1, lastIndex);
          }else{
            tempString = returnString.substring(index+1);
          }


          //Handle response for pin D13 to control LED
          if (alias == "state"){
            if(tempString == "1"){
              digitalWrite(LED_PIN, LOW);
              Serial.println("turn on Light");
            }else if(tempString == "0"){
              digitalWrite(LED_PIN, HIGH);
              Serial.println("turn off Light");
            }else{
              Serial.print(F("Unknown Setting: "));
              Serial.println(tempString);
            }
          } else if (alias == "msg"){
            Serial.print("Message: ");
            Serial.println(tempString);
          } else {
            tempString = "0"; 
            Serial.println("Unknown Alias Dataport");
          }

          if(lastIndex == -1)
            break;

        } else{
          Serial.println(F("No Index"));
          break;
        }
      }

    } else {
      //Serial.println("HANDLER: network or platform api request issue");
      exosite_comm_errors++;
      comm_errors++;

    }


    if (exosite_comm_errors == 5) {

      Serial.print("exo: Check Provisioning - ");
      Serial.print("Murano Product ID: ");
      Serial.print(productId);
      Serial.print(F(", Device Identifier (MAC Address): "));
      Serial.println(macString);

      if (exosite.provision(productId, productId, macString)) {
        Serial.println("exo: Provision: New Activation");
        EEPROM.commit();

      } else {
        Serial.println("exo: Provision: No New Activation");

      }
      exosite_comm_errors = 0;
    }
    if (comm_errors == 20) {
      delay(2000);
      // If not able to communicate, test restarting WiFi Router Connection
      Serial.println("wifi: Setup WiFi Network Connection Again - ");
      connectWifi();
      comm_errors = 0;
    }
  }
}

/*==============================================================================
* connectWifi
*
* Use in setup to connect to local Wifi network
*=============================================================================*/
void connectWifi() {
  WiFi.mode(WIFI_STA); // Need for deepSleep
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("wifi: Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  byte ledStatus = LOW;
  int delays = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delays++;
    if (delays > 30)
      Serial.println("");
    // Blink the LED
    digitalWrite(LED_PIN, ledStatus); // Write LED high/low
    ledStatus = (ledStatus == HIGH) ? LOW : HIGH;
    delay(100);
  }
  digitalWrite(LED_PIN, HIGH);
  Serial.println("");
  Serial.println("wifi: connected");
  Serial.print("wifi: IP Address: ");
  Serial.println(WiFi.localIP());
  delay(100);
  // Create MAC Address String in the format FF:FF:FF:FF:FF:FF
  WiFi.macAddress(macData);
  snprintf(macString, 18, "%02X%02X%02X%02X%02X%02X",
           macData[5], macData[4], macData[3], macData[2], macData[1], macData[0]);
  // Print Some Useful Info
  Serial.print(F("wifi: MAC Address: "));
  Serial.println(macString);
}
