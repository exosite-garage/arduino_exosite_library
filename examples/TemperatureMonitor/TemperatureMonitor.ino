/*
 Exosite Arduino Basic Temp Monitor 2 (updated to use Exosite library)
  
 This sketch shows an example of sending data from a connected
 sensor to Exosite. (http://exosite.com) Code was used from various
 public examples including the Arduino Ethernet examples and the OneWire
 Library examples found on the Arduino playground. 
 (OneWire Lib credits to Jim Studt, Tom Pollard, Robin James, and Paul Stoffregen)
  
 This code keeps track of how many milliseconds have passed
 and after the user defined REPORT_TIMEOUT (default 60 seconds)
 reports the temperature from a Dallas Semi DS18B20 1-wire temp sensor.
 The code sets up the Ethernet client connection and connects / disconnects 
 to the Exosite server when sending data.
  
 Assumptions:
 - Tested with Aruduino 1.0.5
 - Arduino included Ethernet Library
 - Arduino included SPI Library
 - Using Exosite library (2013-10-20) https://github.com/exosite-garage/arduino_exosite_library
 - Using OneWire Library Version 2.0 - http://www.arduino.cc/playground/Learning/OneWire
 - Using Dallas Temperature Control Library - http://download.milesburton.com/Arduino/MaximTemperature/DallasTemperature_372Beta.zip
 - Uses Exosite's basic HTTP API, revision 1.0 https://github.com/exosite/api/tree/master/data
 --- USER MUST DO THE FOLLOWING ---
 - User has an Exosite account and created a device (CIK needed / https://portals.exosite.com -> Add Device)
 - User has added a device to Exosite account and added a data source with alias 'temp', type 'float'
 
  
 Hardware:
 - Arduino Duemilanove or similiar
 - Arduino Ethernet Shield
 - Dallas Semiconductor DS18B20 1-Wire Temp sensor used in parasite power mode (on data pin 7, with 4.7k pull-up)
 
Version History:
- 1.0 - November 8, 2010 - by M. Aanenson
- 2.0 - July 6, 2012 - by M. Aanenson
- 3.0 - October 25, 2013 - by M. Aanenson - Note: Updated to use latest Exosite Arduino Library

*/
  
  
#include <SPI.h>
#include <Ethernet.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Exosite.h>
 
// Pin use
#define ONEWIRE 7 //pin to use for One Wire interface

// Set up which Arduino pin will be used for the 1-wire interface to the sensor
OneWire oneWire(ONEWIRE);
DallasTemperature sensors(&oneWire);
 
/*==============================================================================
* Configuration Variables
*
* Change these variables to your own settings.
*=============================================================================*/
String cikData = "0000000000000000000000000000000000000000";  // <-- FILL IN YOUR CIK HERE! (https://portals.exosite.com -> Add Device)
byte macData[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};        // <-- FILL IN YOUR Ethernet shield's MAC address here.

// User defined variables for Exosite reporting period and averaging samples
#define REPORT_TIMEOUT 30000 //milliseconds period for reporting to Exosite.com
#define SENSOR_READ_TIMEOUT 5000 //milliseconds period for reading sensors in loop


/*==============================================================================
* End of Configuration Variables
*=============================================================================*/

class EthernetClient client;
Exosite exosite(cikData, &client);

//
// The 'setup()' function is the first function that runs on the Arduino.
// It runs completely and when complete jumps to 'loop()' 
//
void setup() {
  Serial.begin(9600);
  Serial.println("Boot");
  // Start up the OneWire Sensors library
  sensors.begin();
  delay(1000);
  Serial.println("Starting Exosite Temp Monitor");
  Serial.print("OneWire Digital Pin Specified: ");
  Serial.println(ONEWIRE);
  Ethernet.begin(macData);
  // wait 3 seconds for connection
  delay(3000); 
 
}
 
//
// The 'loop()' function is the 'main' function for Arduino 
// and is essentially a constant while loop. 
//
void loop() {
  static unsigned long sendPrevTime = 0;
  static unsigned long sensorPrevTime = 0; 
  static float tempF;
  char buffer[7];
  String readParam = "";
  String writeParam = "";
  String returnString = "";  
   
  Serial.print("."); // print to show running
 
 // Read sensor every defined timeout period
  if (millis() - sensorPrevTime > SENSOR_READ_TIMEOUT) {
    Serial.println();
    Serial.println("Requesting temperature...");
    sensors.requestTemperatures(); // Send the command to get temperatures
    float tempC = sensors.getTempCByIndex(0);
    Serial.print("Celsius:    ");
    Serial.print(tempC);
    Serial.println(" C ..........DONE");
    tempF = DallasTemperature::toFahrenheit(tempC);
    Serial.print("Fahrenheit: ");
    Serial.print(tempF);
    Serial.println(" F ..........DONE");
     
    sensorPrevTime = millis();
  }
 
  // Send to Exosite every defined timeout period
  if (millis() - sendPrevTime > REPORT_TIMEOUT) {
    Serial.println(); //start fresh debug line
    Serial.println("Sending data to Exosite...");
    
    readParam = "";        //nothing to read back at this time e.g. 'control&status' if you wanted to read those data sources
    writeParam = "temp="; //parameters to write e.g. 'temp=65.54' or 'temp=65.54&status=on'
    
    String tempValue = dtostrf(tempF, 1, 2, buffer); // convert float to String, minimum size = 1, decimal places = 2
    
    writeParam += tempValue;    //add converted temperature String value
    
    //writeParam += "&message=hello"; //add another piece of data to send

    if ( exosite.writeRead(writeParam, readParam, returnString)) {
      Serial.println("Exosite OK");
      if (returnString != "") {
        Serial.println("Response:");
        Serial.println(returnString);
      }
    }
    else {
      Serial.println("Exosite Error");
    }
 
    sendPrevTime = millis(); //reset report period timer
    Serial.println("done sending.");
  }
  delay(1000); //slow down loop
}
